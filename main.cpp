#ifdef __ANDROID__
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#endif

#include <string>
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


void draw_text(SDL_Renderer* renderer, const char* string, TTF_Font* font, int x, int y, Uint32 colour);
void draw_text_centered(SDL_Renderer* renderer, const char* string, TTF_Font* font, int x, int y, Uint32 colour);

//returns true if the mouse is on this box
bool handle_click_box(SDL_Renderer* renderer,const char* text, TTF_Font* font, float mouse_x,float mouse_y,float x1,float y1,float x2,float y2){

	bool ret= false;
	Transform trans;
	trans.init();
	if(mouse_x > x1 && mouse_y > y1 && mouse_x < x2 && mouse_y < y2){
		ret=true;
		trans.drawrectangle(renderer, x1,y1,x2,y2,0xFFFF0000);
	}else{
		trans.drawrectangle(renderer, x1,y1,x2,y2,0xFFD00000);
	}
	draw_text_centered(renderer, text, font, (x2+x1)/2, (y2+y1)/2, 0xFF000000);
	return ret;
}



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



LINE lines[am_lines];
vector<TRAIN> trains;

void draw_text(SDL_Renderer* renderer, const char* string, TTF_Font* font, int x, int y, Uint32 colour){
	//TTF_Font* font = TTF_OpenFont(font_file, size);
	if(!font) {
		printf("TTF_OpenFont: %s\n", TTF_GetError());
	}
	SDL_Colour colour_sdl = {(Uint8)((colour & 0x00FF0000) >> 16),
				 (Uint8)((colour & 0x0000FF00) >> 8),
				 (Uint8)((colour & 0x000000FF))};
	SDL_Surface* text_surface
		= TTF_RenderText_Blended
		(font, string,colour_sdl);

	SDL_Texture * texture = SDL_CreateTextureFromSurface(renderer, text_surface);
	int texture_width = 0;
	int texture_height= 0;
	SDL_QueryTexture(texture, NULL, NULL, &texture_width, &texture_height);
	SDL_Rect texture_rect = { x, y, texture_width, texture_height};

	SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
	SDL_DestroyTexture(texture);
	SDL_FreeSurface(text_surface);

}

void draw_text_centered(SDL_Renderer* renderer, const char* string, TTF_Font* font, int x, int y, Uint32 colour){
	int w,h;
	TTF_SizeText(font,string,&w,&h);
	draw_text(renderer, string, font, x-w/2, y-h/2, 0xFF000000);

}

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


			string message;
			TTF_Init();



			bool show_message=false;
			unsigned int message_end_frame;// at wich frame the
						       // message shouldn't be
						       // displayed anymore

			//initialize lines
			for(int i=0; i<am_lines; i++){
				lines[i].create(line_colours[i],i);
			}

			stations.init();

			//initialize random generator
			time_t t;
			srand((unsigned) time(&t));


			Uint32 start_tick;
			SDL_Event event;

			float scale = calc_scale(screen_width, screen_height);
			TTF_Font* font_15 = TTF_OpenFont(font_file, 15*scale);
			TTF_Font* font_30 = TTF_OpenFont(font_file, 30*scale);
			TTF_Font* font_20 = TTF_OpenFont(font_file, 20*scale);


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

			unsigned int frame = 0;

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
				for(int i =0; (unsigned int)i<am_lines; i++){
					if(!lines[i].used)
						continue;
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
				
				if(status.play_status == PLAYING){
				if(hovering.type == STATION_OBJECT){
					if(mouse.click && selected.line_i == INT_MAX){
						
						//find not used lines
						int free_i=INT_MAX;
						for(unsigned int i =0; i<am_lines; i++){
							if(!lines[i].used && line_unlocked[i]<=status.points){
								free_i=i;
								break;
							}
						}
						if(free_i != INT_MAX){

							selected.line_i = free_i;
							lines[free_i].use();
							lines[selected.line_i].click_add(stations.hovering_id);
						}else{
							message = "You've ran out of lines";
							show_message=true;
							message_end_frame = frame + framerate*1;
						}
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

				draw_text(renderer, info_text, font_15, 10,10, 0xFF000000);
				if(status.play_status==GAME_OVER){
					char  text[] = {'G','A','M','E',' ','O','V','E','R','\0'};
					draw_text_centered(renderer, text, font_30, screen_width/2,screen_height/2, 0xFF000000);
					char restart_text[] = {'R','e','s','t','a','r','t','\0'};
					if(
							handle_click_box(renderer,
								restart_text,
								font_30,
								mouse.x,mouse.y,
								screen_width/4,
								screen_height*5/8,
								screen_width*3/4,
								screen_height*7/8)&&
							mouse.click){
						for(int i =0; i<am_lines; i++){
							lines[i].stop_using();
							stations.clear();
						}
						status.play_status = PLAYING;
						stations.random_add();
						stations.random_add();
						stations.random_add();
						stations.random_add();

					}
				}

				//drw message
				if(show_message){
					draw_text(renderer, message.c_str(), font_20, 30,10, 0xFFFF0000);
					if(frame> message_end_frame){
						show_message=false;
					}
				}
					

				//draw line information
				for(int i =0; i<am_lines; i++){
					int y_pos = screen_height*(0.25+0.25/am_lines*i);
					Uint32 colour;
					if(status.points>=line_unlocked[i]){
						colour =lines[i].colour;
					}else{
						colour =0xFF505050;
					}

					if(lines[i].used){
						Uint32 black = 0xFF000000;
						filledCircleColor(renderer, scale*10, y_pos, scale*7, black);
						boxColor(renderer, 10, y_pos-7*scale, scale*20, y_pos+scale*7, black);
						filledCircleColor(renderer, scale*20, y_pos, scale*7, black);
					}

					filledCircleColor(renderer, scale*10, y_pos, scale*5, colour);
					boxColor(renderer, scale*10, y_pos-scale*5, scale*20, y_pos+scale*5, colour);
					filledCircleColor(renderer, scale*20, y_pos, scale*5, colour);
				}

				//for one reason or another the program
				//crashes at some points when these two lines
				//aren't added in. This is why I added them
				SDL_RendererInfo info;
				SDL_GetRendererInfo(renderer, &info);
				SDL_RenderPresent(renderer);
				frame++;


			}
			TTF_CloseFont(font_15);
			TTF_CloseFont(font_30);
			TTF_CloseFont(font_20);
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
