#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <climits>

#include "transform.h"

using namespace std;

#define framerate 40.0

#define screen_width 640
#define screen_height 480


typedef struct {
	Uint8 r,g,b;
}COLOUR;

const COLOUR line_colours[]={
{255,0,255},
{255,0,0},
{0,255,0},
{255,255,0},
{0,0,255},
{0,0,0},
{0,255,255}};

class MOUSE{
	public:
	bool prev_mouse_pressed = false;//true if mouse was pressed in previous frame
	bool click = false;
	Uint32 buttons;
	float x,y;
	void update(){
		int tmp_x, tmp_y;
		buttons = SDL_GetMouseState(&tmp_x, &tmp_y);
		x=  (float)tmp_x; y=(float)tmp_y;
		bool mouse_pressed = buttons & SDL_BUTTON_LMASK;
		click = mouse_pressed && !prev_mouse_pressed;
		prev_mouse_pressed = mouse_pressed;
	}
};

typedef struct{
	float x;
	float y;
	int id;
}Point2d;

const float STATION_WAIT_TIME = 0.1;

float distance2d(Point2d a, Point2d b){
	return(sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2)));
}

const int n_lines =7;

typedef struct node node_t;

typedef enum {NEXT=1, PREV=0} LINK_DIRECTION;

struct node{
	node_t* links[2];
	int value;
};

node_t* add_node_before(node_t* node){
	node_t *new_node_p  = (node_t*)malloc(sizeof(node_t));
	new_node_p->links[NEXT] = node;
	if(node){
		new_node_p->links[PREV] = node->links[PREV];
		if(node->links[PREV])
			node->links[PREV]->links[NEXT] = new_node_p;
		node->links[PREV] = new_node_p;
	}else{
		new_node_p->links[PREV] = NULL;
	}

	return(new_node_p);
}

const int connections[][2] = {{0,1},{0,2}};

vector<Point2d> stations;

void must_init(bool succeeded, string message){
	if(!succeeded){
		//cout << "Failed to " << message << endl;
	}
	//exit(1);
}

//The object at the ending of a railline
class BUFFER{
	private:

	float left_x;
	float left_y;
	float right_x;
	float right_y;
	float middle_x;
	float middle_y;
	float bottom_x;
	float bottom_y;
	
	public:	
	void create(Point2d station1, Point2d station2){
		float dy = station1.y - station2.y;
		float dx = station1.x - station2.x;

		float length = sqrt( dy*dy + dx*dx );

		float norm_dy = dy/length;
		float norm_dx = dx/length;
		
		middle_x = station1.x + norm_dx*20;
		middle_y = station1.y + norm_dy*20;
		
		left_x = middle_x - norm_dy*20;
		left_y = middle_y + norm_dx*20;
		
		right_x = middle_x + norm_dy*20;
		right_y = middle_y - norm_dx*20;
	}

	void draw(SDL_Renderer* renderer,transform& trans, COLOUR colour){
		trans.drawline(renderer, bottom_x, bottom_y, middle_x,
				middle_y, 8,colour.r, colour.g, colour.b, 255);
		trans.drawline(renderer, left_x, left_y, right_x,
				right_y, 8,colour.r, colour.g, colour.b, 255);


	}
};

class TRAIN{
	public:

	bool initialised =false;

	private:
	enum {AT_STATION, ON_LINE} location_type = AT_STATION;
	LINK_DIRECTION direction;
	
	int station_id;
	float waiting_time_seconds = STATION_WAIT_TIME;

	node_t* start_line;
	float velocity = 1;

	float width = 10;
	float height = 20;

	float x0;
	float y0;
	float x1;
	float y1;
	float slope;

	float x,y;
	public:
	void draw(SDL_Renderer* renderer,transform& trans){
		transform train_trans;
	        train_trans.m = identity;
		train_trans.rotate(atan(slope)+M_PI/2);
		train_trans.translate(x, y);
		train_trans.m = matrix_mul(trans.m,train_trans.m);
		train_trans.drawrectangle(renderer, -width, -height, width, height, 0,0,0,255);
	}

	void init(node_t* start_station, LINK_DIRECTION in_direction){
		initialised = true;
		start_line = start_station;
		station_id = start_line->value;
		direction = in_direction;
	}
	void move(float seconds){
		switch(location_type){
		case(AT_STATION):
			x = stations[station_id].x;
			y = stations[station_id].y;
			waiting_time_seconds-= seconds;
			if(waiting_time_seconds<0){
				location_type = ON_LINE;
				if(!start_line->links[direction])
						direction = direction ? PREV : NEXT;
					start_line = start_line->links[direction];
				break;
			}
			x0 = stations[start_line->value].x;
			y0 = stations[start_line->value].y;
			x1 = stations[start_line->links[direction]->value].x;
			y1 = stations[start_line->links[direction]->value].y;
			slope = (y1-y0)/(x1-x0);
			break;
				
		case(ON_LINE):
			float dx = sqrt( velocity/(slope*slope+1) );
			if(x1-x0<0)
				dx = -dx;
			float dy = slope*dx;
			x+=dx;
			y+=dy;

			//check if the train is beyond the station
			if( (x-x0)/(x1-x0) >1 ||
			    (y-y0)/(y1-y0) >1){
				station_id = start_line->value;
				location_type = AT_STATION;
				waiting_time_seconds = STATION_WAIT_TIME;
				if(!start_line->links[direction])
						direction = direction ? PREV : NEXT;
			}
		}
	}
};

typedef enum{NO_OBJECT=0,STATION_OBJECT,LINE_OBJECT,BUFFER_OBJECT} OBJECT_TYPE;

typedef struct {
	OBJECT_TYPE type;
	int station_id; //only if type is set to STATION_OBJECT
	int line_i;//only set if type is et to LINE_OBJECT
}object_pointer;

typedef struct{
	int line_i=INT_MAX;
}selection_pointer;


typedef struct{
	node_t* stations;
	TRAIN train;
	int length=0;
	BUFFER bufferstop1;
	BUFFER bufferstop2;
}LINE_T;


class LINE{
	public:
	node_t* station_nodes;
	TRAIN train;
	int length=0;
	BUFFER bufferstop1;
	BUFFER bufferstop2;
	COLOUR colour;
	node_t* selected;
	node_t* hovering;
	
	
	public:
	void create(COLOUR new_colour){
		colour = new_colour;
	}
	void click_add(int station_id){
		selected = add_node_before(selected);
		if(!selected->links[PREV])
			station_nodes = selected;
		selected->value = station_id;
		length +=1;
		if(length >=2 && !train.initialised)
			train.init(station_nodes, NEXT);
	}

	void click_select(){
		if(hovering){
			selected = hovering;
		}else{
			selected = station_nodes;
		}
	}
	void unselect(){
		selected = NULL;
	}
	bool handle_mouse(float mouse_x, float mouse_y){

		hovering = NULL;
		
		bool first = true;
		int prev = 0;
		node_t* head = station_nodes;
		int closest_line_distance=INT_MAX;
		bool return_val=false;
		while(head){
			if(first){
				first = false;
			}
			else{
			float x0 = stations[prev].x;
			float y0 = stations[prev].y;
			float x1 = stations[head->value].x;
			float y1 = stations[head->value].y;

			//calculate distance from line to mouse only if it the mouse is in its box
			if( !
				((mouse_x >= 10+x0 && mouse_x >= 10+x1) ||
				(mouse_x <= -10+x0 && mouse_x <= -10+x1) ||
				(mouse_y >= 10+y0 && mouse_y >= 10+y1) ||
				(mouse_y <= -10+y0 && mouse_y <= -10+y1))
				){

			
				float slope = (y1-y0)/(x1-x0);
				float slope_perpendicular = -1/slope;
				float start_perpendicular = (x0-mouse_x)*slope_perpendicular+mouse_y;

				//The points where the distance to the line will be measured
				//from.
				float distance_x = (start_perpendicular-y0)/(slope-slope_perpendicular)+x0;
				float distance_y = slope*(distance_x-x0)+y0;
				
				float distance = distance2d({distance_x, distance_y, 0}, {mouse_x, mouse_y, 0});
				//cout << "mouse x,y:" << mouse_x << " " << mouse_y << endl;
				if(distance<closest_line_distance&&distance<10){
					return_val = true;
					hovering= head;
					closest_line_distance = distance;
				}


			}}
			prev = head->value;

			head = head->links[NEXT];

		}
		return(return_val);
	}
	void draw(SDL_Renderer* renderer,transform& trans,
			int mouse_x, int mouse_y){
		bool first = true;
		int prev = 0;

		node_t* head = station_nodes;

		if(train.initialised && length >= 2){
			train.move(1/framerate);
			train.draw(renderer, trans);
		}

		while(head){

			if(first){
				prev = head->value;
				first = false;
				if(selected== head){
					trans.drawline(renderer, stations[head->value].x, 
							stations[head->value].y, mouse_x, 
							mouse_y, 8,colour.r, colour.g, colour.b, 255);
					trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
				}
					

			}
			else{
			float x0 = stations[prev].x;
			float y0 = stations[prev].y;
			float x1 = stations[head->value].x;
			float y1 = stations[head->value].y;


			if(selected == head){
				trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 127);
				trans.drawline(renderer, x0, y0, mouse_x, mouse_y, 8,colour.r, colour.g, colour.b, 255);
				trans.drawline(renderer, mouse_x, mouse_y, x1, y1, 8,colour.r, colour.g, colour.b, 255);
				trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
			}

			else if(hovering== head){
				trans.drawline(renderer, x0, y0, x1, y1, 8,0, 0, 0, 255);
			}
			else
				trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 255);
			prev = head->value;
			}
			head = head->links[NEXT];
		}
	}
};

vector<LINE> lines;

int main(int argc, char* argv[]){
	SDL_Window *window = NULL;
	must_init( SDL_Init(SDL_INIT_EVERYTHING)<0, "initialize everything.");

	window = SDL_CreateWindow("Example", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, 640, 480, SDL_WINDOW_SHOWN);

	//SDL_TimerID timer = SDL_AddTimer(framerate, NULL, NULL);
	SDL_Surface *screen = SDL_GetWindowSurface(window);

	SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);

	must_init(screen , "get window surface");
	must_init(window," create window");

	bool running = true;

	Uint32 start_tick;
	SDL_Event event;

	float camera_x= 0;
	float camera_y= 0;


	MOUSE mouse;


	object_pointer hovering;
	selection_pointer selected;


	while(running){

		hovering.type = NO_OBJECT;
		
		transform trans;
		trans.init();
		trans.translate(camera_x,camera_y);


		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if(state[SDL_SCANCODE_LEFT])
			camera_x+=10;
		if(state[SDL_SCANCODE_UP]) 
			camera_y+=10;
		if(state[SDL_SCANCODE_RIGHT])
			camera_x-=10;
		if(state[SDL_SCANCODE_DOWN]) 
			camera_y-=10;
		if(mouse.buttons & SDL_BUTTON_RMASK && selected.line_i != INT_MAX){
			lines[selected.line_i].unselect();
			selected.line_i=INT_MAX;
		}

		mouse.update();



		//initialize line list
	

		start_tick = SDL_GetTicks();

		while ( SDL_PollEvent( &event) ){
			if(event.type == SDL_QUIT)
				running = false;
		}


		must_init(SDL_UpdateWindowSurface(window)<0,"update window");


		// clear window
		SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
		SDL_RenderClear(renderer);

		//drawing
		Uint32 black = SDL_MapRGB(screen->format,  0,0,0);
		SDL_Rect background = {10,0,4,screen_height};
		SDL_FillRect(screen, &background, black);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		//Check if the mouse is hovering above a station
		if(!hovering.type){
		for(Point2d station : stations){
			if(pow( station.x-(mouse.x-camera_x),2) + 
					pow(station.y-(mouse.y-camera_y),2)< 100 ){
				hovering.type = STATION_OBJECT;
				hovering.station_id = station.id;
			}
		}}

		//int closest_line_distance=INT_MAX;
		//loop through all lines to see which one the mouse is nearest to
		/*if(!hovering.type){
		int count = 0;
		for(LINE line : lines){
			bool first = true;
			int prev = 0;
			node_t* head = line.station_nodes;
			while(head){
				if(first){
					first = false;
				}
				else{
				float x0 = stations[prev].x;
				float y0 = stations[prev].y;
				float x1 = stations[head->value].x;
				float y1 = stations[head->value].y;


				//calculate distance from line to mouse only if it the mouse is in its box
				if( !
					((mouse.x-camera_x >= 10+x0 && mouse.x-camera_x >= 10+x1) ||
					(mouse.x-camera_x <= -10+x0 && mouse.x-camera_x <= -10+x1) ||
					(mouse.y-camera_y >= 10+y0 && mouse.y-camera_y >= 10+y1) ||
					(mouse.y-camera_y <= -10+y0 && mouse.y-camera_y <= -10+y1))
					){

					cout << "in box"<< endl;
				
					float slope = (y1-y0)/(x1-x0);
					float slope_perpendicular = -1/slope;
					float start_perpendicular = (x0-(mouse.x-camera_x))*slope_perpendicular+mouse.y-camera_y;

					//The points where the distance to the line will be measured
					//from.
					float distance_x = (start_perpendicular-y0)/(slope-slope_perpendicular)+x0;
					float distance_y = slope*(distance_x-x0)+y0;
					
					float distance = distance2d({distance_x, distance_y, 0}, {mouse.x-camera_x, mouse.y-camera_y, 0});
					//cout << "Distance:" << distance << ", distance_x,distance_y:" << distance_x << " " << distance_y <<  endl;
					//cout << "mouse x,y:" << mouse.x << " " << mouse.y << endl;
					if(distance<closest_line_distance&&distance<10){
						cout << "found" << endl;
						hovering.node = head;
						hovering.type = LINE_OBJECT;
						hovering.line_i=count;
						closest_line_distance = distance;
					}


				}}
				prev = head->value;

				head = head->links[NEXT];

			}
			count++;
		}}*/
		for(int i =0; (unsigned int)i<lines.size(); i++){
			if(!hovering.type&&lines[i].handle_mouse(mouse.x-camera_x, mouse.y-camera_y)){
				hovering.type = LINE_OBJECT;
				hovering.line_i = i;
			}
			if(mouse.click && hovering.type==LINE_OBJECT && hovering.line_i == i){
				lines[i].click_select();
				selected.line_i = i;
			}
			lines[i].draw(renderer, trans, mouse.x-camera_x, mouse.y-camera_y);
			/*bool first = true;
			int prev = 0;

			node_t* head = lines[i].station_nodes;

			if(lines[i].train.initialised && lines[i].length >= 2){
				lines[i].train.move(1/framerate);
				lines[i].train.draw(renderer, trans);
			}

			COLOUR colour = line_colours[i];
			while(head){

				if(first){
					prev = head->value;
					first = false;
					if(selected.node == head){
						trans.drawline(renderer, stations[head->value].x, 
								stations[head->value].y, mouse.x, 
								mouse.y, 8,colour.r, colour.g, colour.b, 255);
						trans.drawcircle(renderer, mouse.x, mouse.y, 8, 255, 0, 0, 255);
					}
						

				}
				else{
				float x0 = stations[prev].x;
				float y0 = stations[prev].y;
				float x1 = stations[head->value].x;
				float y1 = stations[head->value].y;


				if(selected.node == head){
					trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 127);
					trans.drawline(renderer, x0, y0, mouse.x, mouse.y, 8,colour.r, colour.g, colour.b, 255);
					trans.drawline(renderer, mouse.x, mouse.y, x1, y1, 8,colour.r, colour.g, colour.b, 255);
					trans.drawcircle(renderer, mouse.x, mouse.y, 8, 255, 0, 0, 255);
				}

				else if(hovering.type == LINE_OBJECT && hovering.node == head){
					trans.drawline(renderer, x0, y0, x1, y1, 8,0, 0, 0, 255);
					if(mouse.click){
						selected.node = hovering.node;
						selected.line_i = hovering.line_i;
					}
				}
				else
					trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 255);
				prev = head->value;
				}
				head = head->links[NEXT];
			}*/
		}
		//draw staions
		for(Point2d station : stations){
			if((hovering.station_id == station.id) && hovering.type == STATION_OBJECT){
				trans.drawcircle(renderer, station.x, station.y, 8, 0, 255, 0, 255);
					if(mouse.click){
						if(selected.line_i == INT_MAX){
							lines.resize(lines.size()+1);
							selected.line_i = lines.size()-1;
							cout << selected.line_i << endl;
							lines[lines.size()-1].create(line_colours[selected.line_i]);
						}
						lines[lines.size()-1].click_add(station.id);
						/*selected.node = add_node_before(selected.node);
						if(!selected.node->links[PREV])
							lines[selected.line_i].station_nodes = selected.node;
						selected.node->value = station.id;
						lines[selected.line_i].length +=1;
						if(lines[selected.line_i].length >=2 && !lines[selected.line_i].train.initialised)
							lines[lines.size()-1].train.init(lines[lines.size()-1].station_nodes, NEXT);*/
						//selected.node = lines[line_i].stations;
						//selected.line_i = line_i;
					}
			}else
				trans.drawcircle(renderer, station.x, station.y, 8, 255, 0, 0, 255);
		}
		if(mouse.click && selected.line_i==INT_MAX){
			stations.push_back({mouse.x-camera_x, mouse.y-camera_y, (int)stations.size()});
		}
		if(mouse.click && selected.line_i!=INT_MAX && !hovering.type){
			Point2d station = {mouse.x-camera_x, mouse.y-camera_y, (int)stations.size()};

			lines[selected.line_i].click_add(station.id);
	
			/*selected.lines = add_node_before(selected.node);
			lines[selected.line_i].length +=1;
			if(!selected.node->links[PREV])
				lines[selected.line_i].station_nodes = selected.node;
			selected.node->value = station.id;*/
			stations.push_back(station);
		}
		
		

		//draw connections

		if( ( 1000/framerate) > SDL_GetTicks() - start_tick ){
			SDL_Delay( 1000/ framerate - ( SDL_GetTicks() - start_tick) );
		}
		SDL_RenderPresent(renderer);
	}


	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
