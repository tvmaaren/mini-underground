#ifdef __ANDROID__
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
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


#define inverse_trans_x(x) (x/scale-camera_x)
#define inverse_trans_y(y) (y/scale-camera_y)


const COLOUR line_colours[]={
{255,0,255},
{255,0,0},
{0,255,0},
{255,255,0},
{0,0,255},
{0,0,0},
{0,255,255}};


float calc_scale(int width, int height){
		int min_width_heigt = width>height ? height : width;
		float scale = (float)min_width_heigt/(float)480;
		return scale;
}

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


STATUS status={0,PLAYING};



vector<LINE> lines;
vector<TRAIN> trains;

int main(int argc, char* argv[]){
	if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
		SDL_Window* window = NULL;
		SDL_Renderer* renderer = NULL;

		int screen_width = 640;
		int screen_height = 480;

		if (SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &window, &renderer) == 0) {
			//maybe the screen hasn't have the same size as asked
			SDL_SetWindowResizable(window, SDL_TRUE);
			SDL_GL_GetDrawableSize(window, &screen_width,&screen_height);

			SDL_bool done = SDL_FALSE;


			 	
			if(TTF_Init()==-1) {
				   cout << "TTF_Init:" << TTF_GetError() << endl;
				   exit(2);
			}
			TTF_Font* font = TTF_OpenFont("DejaVuSansCondensed-Bold.ttf",20);


			stations.init();

			//initialize random generator
			time_t t;
			srand((unsigned) time(&t));


			Uint32 start_tick;
			SDL_Event event;

			float scale = calc_scale(screen_width, screen_height);


			float camera_x= screen_width/2/scale;
			float camera_y= screen_height/2/scale;
			float start_camera_x;
			float start_camera_y;

			stations.random_add();
			stations.random_add();
			stations.random_add();
			stations.random_add();


			MOUSE mouse;

			bool fingerdown=false;//Touch screen


			object_pointer hovering;
			selection_pointer selected;

			int frame = 0;

			while (!done) {
				SDL_GetWindowSize(window, &screen_width,&screen_height);
				//rescale based on size window
				/*int min_width_heigt = screen_width>screen_height ? screen_height : screen_width;*/
				scale = calc_scale(screen_width, screen_height);

				hovering.type = NO_OBJECT;

				Transform trans;
				trans.init();
				//stations.add(-camera_x/scale,-camera_y/scale);
				trans.scale(scale);
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


				if(status.play_status==PLAYING&&mouse.let_go&&selected.line_i!=INT_MAX){
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
				if(status.play_status == PLAYING && stations.check_hovering(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y))){
					hovering.type = STATION_OBJECT;
				}
				for(int i =0; (unsigned int)i<lines.size(); i++){
					if(status.play_status == PLAYING){
					if(!hovering.type&&lines[i].handle_mouse(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y))){
						hovering.type = LINE_OBJECT;
						hovering.line_i = i;
					}
					if(mouse.click && hovering.type==LINE_OBJECT && hovering.line_i == i){
						lines[i].click_select();
						selected.line_i = i;
					}
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
				if(status.play_status == PLAYING){
				if(hovering.type == STATION_OBJECT){
					if(mouse.click && selected.line_i == INT_MAX){
						lines.resize(lines.size()+1);
						lines[lines.size()-1].id=lines.size()-1;
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
				}
				if(mouse.click && selected.line_i==INT_MAX){
					mouse.start_x = mouse.x;
					mouse.start_y = mouse.y;
					start_camera_x  = camera_x;
					start_camera_y= camera_y;
				}
				if(mouse.pressed&&selected.line_i==INT_MAX){
					camera_x = start_camera_x - (mouse.start_x-mouse.x)/scale;
					camera_y = start_camera_y - (mouse.start_y-mouse.y)/scale;
				}
				if(mouse.click && selected.line_i!=INT_MAX && !hovering.type){

					lines[selected.line_i].click_add(
							stations.add(inverse_trans_x(mouse.x), inverse_trans_y(mouse.y)));

				}



				//draw connections

				if( ( 1000/framerate) > SDL_GetTicks() - start_tick ){
					SDL_Delay( 1000/ framerate - ( SDL_GetTicks() - start_tick) );
				}

				//draw information text
				char info_text[100];
				sprintf(info_text, "%d",status.points);

				draw_text(renderer, info_text, 15, 10,10);
				if(status.play_status==GAME_OVER){
					char  text[] = {'G','A','M','E',' ','O','V','E','R','\0'};
					draw_text(renderer, text, 30, screen_width/2,screen_height/2);
				}
				

				//for one reason or another the program
				//crashes at some points when these two lines
				//aren't added in. This is why I added them
				SDL_RendererInfo info;
				SDL_GetRendererInfo(renderer, &info);
				SDL_RenderPresent(renderer);
				frame++;


			}
			TTF_CloseFont(font);
		}

		if (renderer) {
			SDL_DestroyRenderer(renderer);
		}
		if (window) {
			SDL_DestroyWindow(window);
		}
	}
	TTF_Quit();
	SDL_Quit();
	return 0;

}
