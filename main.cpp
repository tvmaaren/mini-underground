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

typedef enum {STATION, LINE} STATION_MODE;

typedef struct {
	Uint8 r,g,b;
}COLOUR;

const COLOUR line_colours[]={
{0,0,0},
{255,0,0},
{0,255,0},
{255,255,0},
{0,0,255},
{255,0,255},
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
	STATION_MODE mode =STATION;
};

typedef struct{
	float x;
	float y;
	int id;
}Point2d;

float distance2d(Point2d a, Point2d b){
	return(sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2)));
}

const int n_lines =7;

vector<int> lines[n_lines];
const int connections[][2] = {{0,1},{0,2}};

//const Point2d stations[] = {{100,20},{400,300},{200,400},{90,30}};
vector<Point2d> stations;
	//{{100,30},{40,65}};

void must_init(bool succeeded, string message){
	if(!succeeded){
		//cout << "Failed to " << message << endl;
	}
	//exit(1);
}


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

	int line_i = 0;


	while(running){
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if(state[SDL_SCANCODE_LEFT])
			camera_x+=10;
		if(state[SDL_SCANCODE_UP]) 
			camera_y+=10;
		if(state[SDL_SCANCODE_RIGHT])
			camera_x-=10;
		if(state[SDL_SCANCODE_DOWN]) 
			camera_y-=10;
		if(mouse.buttons & SDL_BUTTON_RMASK)
			mouse.mode = STATION;

		mouse.update();



		//initialize line list
	
		transform trans;
		trans.init();
		trans.translate(camera_x,camera_y);

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
		int station_hovering = -1;
		for(Point2d station : stations){
			if(pow( station.x-(mouse.x-camera_x),2) + 
					pow(station.y-(mouse.y-camera_y),2)< 100 ){
				station_hovering = station.id;
			}
		}

		int closest_line_index=-1;
		int closest_line_distance=INT_MAX;
		//loop through all lines to see which one the mouse is nearest to
		if(station_hovering<0){
		for(int i = 0; i<n_lines; i++ ){
			bool first = true;
			int prev = 0;
			for(int station_i: lines[i]){
				cout << "station:"<< station_i << endl;
				if(station_i == 2){
					i = i;
				}
				if(first){
					first = false;
				}
				else{
				float x0 = stations[prev].x;
				float y0 = stations[prev].y;
				float x1 = stations[station_i].x;
				float y1 = stations[station_i].y;


				//calculate distance from line to mouse only if it the mouse is in its box
				if( !
					((mouse.x >= 10+x0 && mouse.x >= 10+x1) ||
					(mouse.x <= -10+x0 && mouse.x <= -10+x1) ||
					(mouse.y >= 10+y0 && mouse.y >= 10+y1) ||
					(mouse.y <= -10+y0 && mouse.y <= -10+y1))
					){

					cout << "in box"<< endl;
				
					float slope = (y1-y0)/(x1-x0);
					float slope_perpendicular = -1/slope;
					float start_perpendicular = (x0-mouse.x)*slope_perpendicular+mouse.y;

					//The points where the distance to the line will be measured
					//from.
					float distance_x = (start_perpendicular-y0)/(slope-slope_perpendicular)+x0;
					float distance_y = slope*(distance_x-x0)+y0;
					
					float distance = distance2d({distance_x, distance_y, 0}, {mouse.x, mouse.y, 0});
					//cout << "Distance:" << distance << ", distance_x,distance_y:" << distance_x << " " << distance_y <<  endl;
					//cout << "mouse x,y:" << mouse.x << " " << mouse.y << endl;
					if(distance<closest_line_distance&&distance<10){
						cout << "found" << endl;
						closest_line_index = i;
						closest_line_distance = distance;
					}


				}}
				prev = station_i;

			}
		}}
		for(int i = 0; i<n_lines; i++ ){
			bool first = true;
			int prev = 0;

			bool hovering = i == closest_line_index;
			for(int station_i: lines[i]){

				if(first){
					prev = station_i;
					first = false;
				}
				else{
				float x0 = stations[prev].x;
				float y0 = stations[prev].y;
				float x1 = stations[station_i].x;
				float y1 = stations[station_i].y;


				//calculate distance from line to mouse only if it the mouse is in its box
				/*if( !
					((mouse.x >= 10+x0 && mouse.x >= 10+x1) ||
					(mouse.x <= -10+x0 && mouse.x <= -10+x1) ||
					(mouse.y >= 10+y0 && mouse.y >= 10+y1) ||
					(mouse.y <= -10+y0 && mouse.y <= -10+y1))
					&& !hovering_above_line//make sure it isn't already hovering above another line
					){
				
					float slope = (y1-y0)/(x1-x0);
					float slope_perpendicular = -1/slope;
					float start_perpendicular = (x0-mouse.x)*slope_perpendicular+mouse.y;

					//The points where the distance to the line will be measured
					//from.
					float distance_x = (start_perpendicular-y0)/(slope-slope_perpendicular)+x0;
					float distance_y = slope*(distance_x-x0)+y0;
					
					float distance = distance2d({distance_x, distance_y, 0}, {mouse.x, mouse.y, 0});
					cout << "Distance:" << distance << ", distance_x,distance_y:" << distance_x << " " << distance_y <<  endl;
					cout << "mouse x,y:" << mouse.x << " " << mouse.y << endl;
					mouse_hovers_above_line = distance < 100;
					hovering_above_line = mouse_hovers_above_line;


				}*/

				//thickLineRGBA(renderer, x0, y0, x1, y1, 8,255, 0, 0, 255);
				COLOUR colour = line_colours[i];
				if(hovering){
					trans.drawline(renderer, x0, y0, x1, y1, 8,0, 0, 0, 255);
					if(mouse.click){
						mouse.mode = LINE;
						line_i = i;
					}
				}
				else
					trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 255);
				prev = station_i;
				}
			}
		}
		//draw staions
		for(Point2d station : stations){
			if(station_hovering == station.id){
				trans.drawcircle(renderer, station.x, station.y, 8, 0, 255, 0, 255);
					if(mouse.click){
						if(mouse.mode != LINE)
							line_i++;
						lines[line_i].push_back(station.id);
						mouse.mode = LINE;
					}
			}else
				trans.drawcircle(renderer, station.x, station.y, 8, 255, 0, 0, 255);
		}
		if(mouse.click && mouse.mode==STATION)
			stations.push_back({mouse.x-camera_x, mouse.y-camera_y, (int)stations.size()});
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
