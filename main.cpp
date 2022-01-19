#ifdef __ANDROID__
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#endif

#include <stdlib.h>
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


#define inverse_trans_x(x) (x/Scale-camera_x)
#define inverse_trans_y(y) (y/Scale-camera_y)


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
	bool prev_pressed = false;//true if mouse was pressed in previous frame
	bool prev_let_go;
	bool pressed=false;
	bool click = false;
	bool let_go = false;
	Uint32 buttons;

	//when left mouse_button is hold it remembers the original location of the mouse
	float start_x, start_y;
	
	float x,y;
	void update(){
		int tmp_x, tmp_y;
		buttons = SDL_GetMouseState(&tmp_x, &tmp_y);
		x=  (float)tmp_x; y=(float)tmp_y;
		pressed = buttons & SDL_BUTTON_LMASK;
		click = pressed && !prev_pressed;
		let_go = !pressed && prev_pressed;
		prev_pressed = pressed;
	}
};




const int n_lines =7;

typedef struct node node_t;



const int connections[][2] = {{0,1},{0,2}};

//vector<STATION> stations;
STATION_LIST stations;

#ifndef __ANDROID__
void must_init(bool succeeded, string message){
	if(!succeeded){
		//cout << "Failed to " << message << endl;
	}
	//exit(1);
}
#endif



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
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		SDL_Window* window = NULL;
		SDL_Renderer* renderer = NULL;

		if (SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer) == 0) {
			SDL_bool done = SDL_FALSE;

			stations.init();

			//initialize random generator
			time_t t;
			srand((unsigned) time(&t));


			Uint32 start_tick;
			SDL_Event event;

			float camera_x= screen_width/2;
			float camera_y= screen_height/2;
			float start_camera_x;
			float start_camera_y;

			stations.random_add();
			stations.random_add();
			stations.random_add();


			MOUSE mouse;

			bool fingerdown=false;//Touch screen


			object_pointer hovering;
			selection_pointer selected;

			int frame = 0;

			while (!done) {
				hovering.type = NO_OBJECT;

				Transform trans;
				trans.init();
				trans.scale(Scale);
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


				mouse.update();

				//initialize line list

				start_tick = SDL_GetTicks();

				// clear window
				SDL_SetRenderDrawColor(renderer, 242, 242, 242, 255);
				SDL_RenderClear(renderer);


				if(mouse.let_go&&selected.line_i!=INT_MAX){
					lines[selected.line_i].unselect();
					selected.line_i=INT_MAX;
				}
				while (SDL_PollEvent(&event)) {
					if (event.type == SDL_QUIT) {
						done = SDL_TRUE;
					}
				}
				fingerdown = (event.type==SDL_FINGERDOWN || fingerdown) &&
					!(event.type==SDL_FINGERUP);
				//drawing
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

				//Check if the mouse is hovering above a station
				if(stations.check_hovering(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y))){
					hovering.type = STATION_OBJECT;
				}
				for(int i =0; (unsigned int)i<lines.size(); i++){
					if(!hovering.type&&lines[i].handle_mouse(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y))){
						hovering.type = LINE_OBJECT;
						hovering.line_i = i;
					}
					if(mouse.click && hovering.type==LINE_OBJECT && hovering.line_i == i){
						lines[i].click_select();
						selected.line_i = i;
					}
					lines[i].draw(renderer, trans, inverse_trans_x(mouse.x),inverse_trans_y(mouse.y));
				}
				stations.draw(renderer, trans);
				/*if(mouse.click  && hovering.type == STATION_OBJECT){
					if(selected.line_i == INT_MAX ){
						lines.resize(lines.size()+1);
						selected.line_i = lines.size()-1;
						lines[lines.size()-1].create(line_colours[selected.line_i]);
					}
					lines[selected.line_i].click_add(stations.hovering_id);
				}*/
				if(hovering.type == STATION_OBJECT){
					if(mouse.click && selected.line_i == INT_MAX){
						lines.resize(lines.size()+1);
						selected.line_i = lines.size()-1;
						lines[lines.size()-1].create(line_colours[selected.line_i]);
						lines[selected.line_i].click_add(stations.hovering_id);
					}else if(selected.line_i != INT_MAX){
						lines[selected.line_i].click_add(stations.hovering_id);
					}
				}
				if(random() < chance_of_a_new_station){
					stations.random_add();
				}
				if(mouse.click && selected.line_i==INT_MAX){
					mouse.start_x = mouse.x;
					mouse.start_y = mouse.y;
					start_camera_x  = camera_x;
					start_camera_y= camera_y;
				}
				if(mouse.pressed&&selected.line_i==INT_MAX){
					camera_x = start_camera_x - (mouse.start_x-mouse.x)/Scale;
					camera_y = start_camera_y - (mouse.start_y-mouse.y)/Scale;
				}
				if(mouse.click && selected.line_i!=INT_MAX && !hovering.type){

					lines[selected.line_i].click_add(
							stations.add(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y))
					);

				}



				//draw connections

				if( ( 1000/framerate) > SDL_GetTicks() - start_tick ){
					SDL_Delay( 1000/ framerate - ( SDL_GetTicks() - start_tick) );
				}
				SDL_RenderPresent(renderer);
				frame++;


			}
		}

		if (renderer) {
			SDL_DestroyRenderer(renderer);
		}
		if (window) {
			SDL_DestroyWindow(window);
		}
	}
	SDL_Quit();
	return 0;

}
