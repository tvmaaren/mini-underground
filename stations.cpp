#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "types.hpp"
#include "settings.hpp"
#include "transform.hpp"
#include "stations.hpp"
#include <stdlib.h>
#include <iostream>

using namespace std;

void STATION::draw(SDL_Renderer* renderer,transform& trans, COLOUR colour){
	trans.drawcircle(renderer, pos.x, pos.y, 8, colour.r, colour.g, colour.b, 255);
	
	
	for(int i =1; i<=am_passengers ; i++){
		trans.drawcircle(renderer, pos.x+20*i, pos.y, 4, 0, 0, 0, 255);
	}
}

void STATION::add_passenger(){
	
	
	//maybe add a passenger
	if(rand() < chance_of_a_new_passenger)
		am_passengers++;
	
}
void STATION::create(float x, float y, int new_id){
	pos.x = x;
	id = new_id;
	pos.y = y;


}
