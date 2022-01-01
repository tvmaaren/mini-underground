#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <climits>

using namespace std;

#include "types.hpp"
#include "transform.hpp"
#include "misc.hpp"
#include "lines.hpp"
#include "stations.hpp"
#include "settings.hpp"




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




const int n_lines =7;

typedef struct node node_t;



const int connections[][2] = {{0,1},{0,2}};

//vector<STATION> stations;
STATION_LIST stations;

void must_init(bool succeeded, string message){
	if(!succeeded){
		//cout << "Failed to " << message << endl;
	}
	//exit(1);
}



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

	//initialize random generator
	time_t t;
	srand((unsigned) time(&t));

	bool running = true;

	Uint32 start_tick;
	SDL_Event event;

	float camera_x= screen_width/2;
	float camera_y= screen_height/2;

	stations.random_add();
	stations.random_add();
	stations.random_add();


	MOUSE mouse;


	object_pointer hovering;
	selection_pointer selected;

	int frame = 0;
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
		if(stations.check_hovering(mouse.x- camera_x, mouse.y -camera_y)){
			hovering.type = STATION_OBJECT;
		}
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
		}
		stations.draw(renderer, trans);
		if(mouse.click && hovering.type == STATION_OBJECT){
			if(selected.line_i == INT_MAX){
				lines.resize(lines.size()+1);
				selected.line_i = lines.size()-1;
				lines[lines.size()-1].create(line_colours[selected.line_i]);
			}
			lines[selected.line_i].click_add(stations.hovering_id);
		}
		if(random() < chance_of_a_new_station){
			stations.random_add();
		}
		if(mouse.click && selected.line_i==INT_MAX){
			stations.add(mouse.x-camera_x, mouse.y-camera_y);
		}
		if(mouse.click && selected.line_i!=INT_MAX && !hovering.type){
			stations.add(mouse.x-camera_x, mouse.y-camera_y);

			lines[selected.line_i].click_add(
				stations.add(mouse.x-camera_x, mouse.y-camera_y)
			);
	
		}
		
		

		//draw connections

		if( ( 1000/framerate) > SDL_GetTicks() - start_tick ){
			SDL_Delay( 1000/ framerate - ( SDL_GetTicks() - start_tick) );
		}
		SDL_RenderPresent(renderer);
		frame++;
	}


	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
