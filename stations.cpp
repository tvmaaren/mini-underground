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
	switch(shape){
		case(SQUARE):
			trans.drawrectangle(renderer, pos.x+8, pos.y+8, pos.x-8, pos.y-8, colour.r, colour.g, colour.b, 255);
			break;
		case(CIRCLE):
			trans.drawcircle(renderer, pos.x, pos.y, 8, colour.r, colour.g, colour.b, 255);
			break;
	}
	
	for(int i =1; i<=am_passengers_per_type[SQUARE] ; i++){
		trans.drawrectangle(renderer, pos.x+20*i+4,
				pos.y+4,pos.x+20*i-4, pos.y-4,  0, 0, 0, 255);
	}
	for(int i =1; i<=am_passengers_per_type[CIRCLE] ; i++){
		trans.drawcircle(renderer, pos.x+20*i, pos.y, 4, 0, 0, 0, 255);
	}
}

//Tries to subtract %subtract from value
//Returns amount it has subtracted
int subtract_without_negative(int& value, int subtract){
	if(value>subtract){
		value-=subtract;
		return subtract;
	}
	//We know subtract>=value
	int ret = value;
	value =0;
	return(ret);

}

//returns the amount of passengers that leave the station
int STATION::passenger_leavestation(int room_in_train, SHAPE shape){
	int ret = subtract_without_negative(am_passengers_per_type[shape], room_in_train);
	am_passengers-=ret;
	return(ret);
	
	/*if(room_in_train>=am_passengers){
		int ret = am_passengers;
		am_passengers = 0;
		return(ret);
	}

	am_passengers-=room_in_train;
	return(room_in_train);*/
}

void STATION::add_passenger(){
	
	
	//maybe add a passenger
	if(rand() < chance_of_a_new_passenger){
		am_passengers++;
		am_passengers_per_type[rand()%2]++;
	}
	
	
}
void STATION::create(float x, float y, int new_id){
	pos.x = x;
	id = new_id;
	pos.y = y;
	
	switch(rand()%2){
		case(0):
			shape = SQUARE;
			break;
		case(1):
			shape = CIRCLE;
			break;
	}
}
