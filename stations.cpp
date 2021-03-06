#ifdef __ANDROID__
#include <SDL.h>
#include <SDL2_gfxPrimitives.h>
#include <SDL_ttf.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#endif

#include <stdlib.h>
#include <vector>

using namespace std;

#include "types.hpp"
#include "misc.hpp"
#include "settings.hpp"
#include "transform.hpp"
#include "stations.hpp"
#include "lines.hpp"
#include "passengers.hpp"

extern STATUS status;



void STATION::draw(SDL_Renderer* renderer,Transform& trans, Uint32 colour){
	trans.drawshape(renderer, shape, pos.x, pos.y, 8, colour);
	
	int passenger_i = 1;
	for(int i = 0; i<shapes; i++){
		for(int ii =1; ii<=am_passengers_per_type[i] ; ii++){
			trans.drawshape(renderer, int_to_shape(i), pos.x+10*passenger_i, pos.y, 4, 0xFF000000);
			passenger_i++;
		}
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
	
}

void STATION::add_passenger(bool* allowed_shapes){
	//maybe add a passenger
	if(rand() < chance_of_a_new_passenger){

		int passenger_shape;
		do{
			passenger_shape = rand()%shapes;
		}while(!allowed_shapes[passenger_shape] || passenger_shape==shape );
		
		am_passengers++;
		am_passengers_per_type[passenger_shape]++;
	}
	if(am_passengers>station_max_passengers){
		status.play_status = GAME_OVER;
	}
}
SHAPE STATION::create(float x, float y, int new_id){
	pos.x = x;
	id = new_id;
	pos.y = y;

	for(int i=0; i<shapes; i++){
		am_passengers_per_type[i]=0;
	}
	shape = int_to_shape(rand()%shapes);
	return shape;
}
bool STATION::is_hovering(float mouse_x, float mouse_y){
	return(
		pow( pos.x-mouse_x,2) + pow(pos.y-mouse_y,2) < 100
	);
}

//returns id of the new station
int STATION_LIST::add(float x, float y){
	stations.resize(stations.size()+1);

	SHAPE shape = stations[stations.size()-1].create(x, y, stations.size()-1);
	used_shape[shape]=true;

	if(x>max_station_x)
		max_station_x=x;
	if(y>max_station_y)
		max_station_y=y;
	if(x<min_station_x)
		min_station_x=x;
	if(y<min_station_y)
		min_station_y=y;
	return(stations.size()-1);
}
int STATION_LIST::random_add(){
	/*int rand_out = random();
	int delta = (int)(max_station_x-min_station_x);
	float distance */
	float x,y;
	while(true){
		x =  min_station_x -30 + (float) (random()%(int)(60+max_station_x-min_station_x));
		y =  min_station_y -30 + (float) (random()%(int)(60+max_station_y-min_station_y));
		
		bool to_close = false;
		for(STATION station : stations){
			to_close = distance2d(station.pos, {x,y})<
				min_seperation_stations;
			if(to_close)
				break;

		}
		if(!to_close)
			break;
	}
	return(add(x,y));
}
bool STATION_LIST::check_hovering(float mouse_x, float mouse_y){
	for(STATION station : stations){
		if(station.is_hovering(mouse_x,mouse_y)){
			hovering = true;
			hovering_id = station.id;
			return true;
		}
	}
	hovering = false;
	return false;
}

void STATION_LIST::draw(SDL_Renderer* renderer, Transform& trans){
if(stations.size()>=1){

	//draw staions
	for(unsigned int i =0; i<stations.size(); i++){
		if(status.play_status == PLAYING)
			stations[i].add_passenger(used_shape);
		if((hovering_id == stations[i].id) && hovering){
			stations[i].draw(renderer, trans,0xFF00FF00);
		}else{
			stations[i].draw(renderer, trans,0xFF0000FF);
			
		}
	}}
}

void STATION_LIST::init(){
	for(int i=0; i<shapes; i++){
		used_shape[i]=false;
	}
}

void STATION_LIST::clear(){
	max_station_x=0;
	max_station_y=0;
	min_station_x=0;
	min_station_y=0;
	stations.clear();
	init();
}
