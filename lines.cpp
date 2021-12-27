#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <climits>

#include "settings.hpp"
#include "types.hpp"
#include "transform.hpp"
#include "misc.hpp"
#include "lines.hpp"
#include "stations.hpp"

using namespace std;

extern vector<STATION> stations;


void BUFFER::create(Point2d station1, Point2d station2){
	station1_tmp = station1;
	station2_tmp = station2;
	created =true;
	bottom_x = station1.x;
	bottom_y = station1.y;


	float dy = station1.y - station2.y;
	float dx = station1.x - station2.x;

	float length = sqrt( dy*dy + dx*dx );

	float norm_dy = dy/length;
	float norm_dx = dx/length;
	
	middle_x = station1.x + norm_dx*20;
	middle_y = station1.y + norm_dy*20;
	
	left_x = middle_x - norm_dy*10;
	left_y = middle_y + norm_dx*10;
	
	right_x = middle_x + norm_dy*10;
	right_y = middle_y - norm_dx*10;
}
bool BUFFER::handle_mouse(float mouse_x, float mouse_y){
	hovering=(
			distance_line_to_point({mouse_x, mouse_y},
				{left_x, left_y}, {right_x, right_y})<10
			||
			distance_line_to_point({mouse_x, mouse_y},
				{bottom_x,bottom_y}, {bottom_x, bottom_y})<10
	      );
	return(hovering);
}
void BUFFER::draw(SDL_Renderer* renderer,transform& trans, COLOUR colour, float mouse_x, float mouse_y){
	if(!created)
		return;
	if(handle_mouse(mouse_x, mouse_y)){
		trans.drawline(renderer, bottom_x, bottom_y, middle_x,
				middle_y, 8,0,0,0,255);
		trans.drawline(renderer, left_x, left_y, right_x,
				right_y, 8,0,0,0,255);
	}else{
		trans.drawline(renderer, bottom_x, bottom_y, middle_x,
				middle_y, 8,colour.r, colour.g, colour.b, 255);
		trans.drawline(renderer, left_x, left_y, right_x,
				right_y, 8,colour.r, colour.g, colour.b, 255);
	}

}

void TRAIN::draw(SDL_Renderer* renderer,transform& trans){
	transform train_trans;
        train_trans.m = identity;
	train_trans.rotate(atan(slope)+M_PI/2);
	train_trans.translate(x, y);
	train_trans.m = matrix_mul(trans.m,train_trans.m);
	train_trans.drawrectangle(renderer, -width, -height, width, height, colour.r,colour.g,colour.b,255);

	int passenger_i=0;
	for(int i=0; i<am_passengers_per_type[0]; i++){
		float mid_y = height-height/max_passengers-passenger_i*height*2/max_passengers;
		train_trans.drawrectangle(renderer, 4,mid_y+4,-4,mid_y-4, 0,0,0, 255);
		passenger_i++;
	}
	for(int i=0; i<am_passengers_per_type[1]; i++){
		train_trans.drawcircle(renderer, 0, height-height/max_passengers-passenger_i*height*2/max_passengers, 4, 0,0,0, 255);
		passenger_i++;
	}
}

void TRAIN::init(node_t* start_station, LINK_DIRECTION in_direction, COLOUR colour_new){
	initialised = true;
	start_line = start_station;
	station_id = start_line->value;
	direction = in_direction;
	colour = colour_new;
}
void TRAIN::move(float seconds){
	switch(location_type){
	case(AT_STATION):
		x = stations[station_id].pos.x;
		y = stations[station_id].pos.y;
		waiting_time_seconds-= seconds;
		if(waiting_time_seconds<0){
			location_type = ON_LINE;
			if(!start_line->links[direction])
					direction = direction ? PREV : NEXT;
				start_line = start_line->links[direction];
			break;
		}
		x0 = stations[start_line->value].pos.x;
		y0 = stations[start_line->value].pos.y;
		x1 = stations[start_line->links[direction]->value].pos.x;
		y1 = stations[start_line->links[direction]->value].pos.y;
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
			//remove passengers
			SHAPE shape = stations[station_id].shape;
			passengers-=am_passengers_per_type[shape];
			am_passengers_per_type[shape]=0;

			//add passsengers
			int delta;
			delta=stations[station_id].passenger_leavestation(
					max_passengers-passengers,SQUARE);
			am_passengers_per_type[SQUARE]+=delta;
			passengers+=delta;
			delta=stations[station_id].passenger_leavestation(
					max_passengers-passengers,CIRCLE);
			am_passengers_per_type[CIRCLE]+=delta;
			passengers+=delta;
		}
	}
}
void LINE::create(COLOUR new_colour){
	//cout << "create:"<<selected<<endl;
	colour = new_colour;
}
//make sure the station is already put in the right list when you add it
void LINE::click_add(int station_id){
	//cout << "click_add:"<<selected<<endl;
	if(select_direction==NEXT)
		selected = add_node_before(selected);
	else
		selected = add_node_after(selected);

	selected->value = station_id;
	length +=1;
	if(!selected->links[PREV]){
		first_station = selected;
	}
	if(!selected->links[NEXT]){
		last_station = selected;
	}
	if(length >=2 && !train.initialised)
		train.init(first_station, NEXT, colour);

	if(!first_station->links[PREV] && length>=2){
		bufferstop1.create(stations[first_station->value].pos, stations[first_station->links[NEXT]->value].pos);
	}
	if(!last_station->links[NEXT] && length>=2){
		bufferstop2.create(stations[last_station->value].pos, stations[last_station->links[PREV]->value].pos);
	}
}

void LINE::click_select(){
	//cout << "click_select:"<<selected<<endl;
	if(hovering){
		select_direction = NEXT;
		selected = hovering;
	}else if(bufferstop1.hovering){
		select_direction = NEXT;
		selected = first_station;
	}else if(bufferstop2.hovering){
		select_direction = PREV;
		selected = last_station;
	}else{
		select_direction = NEXT;
		selected = first_station;
	}
}
void LINE::unselect(){
	//cout << "unselect:"<<selected<<endl;
	selected = NULL;
}
bool LINE::handle_mouse(float mouse_x, float mouse_y){
	//cout << "handle_mouse:"<<selected<<endl;

	hovering = NULL;
	
	bool first = true;
	int prev = 0;
	node_t* head = first_station;
	int closest_line_distance=INT_MAX;
	bool return_val=false;
	while(head){
		if(first){
			first = false;
		}
		else{
		float x0 = stations[prev].pos.x;
		float y0 = stations[prev].pos.y;
		float x1 = stations[head->value].pos.x;
		float y1 = stations[head->value].pos.y;

		//calculate distance from line to mouse only if it the mouse is in its box
		float distance = distance_line_to_point({mouse_x,mouse_y}, {x0,y0},{x1,y1});
		if(distance<closest_line_distance&&distance<10){
			return_val = true;
			hovering= head;
			closest_line_distance = distance;
		}
		}
		prev = head->value;

		head = head->links[NEXT];

	}
	return_val = return_val || bufferstop1.handle_mouse(mouse_x, mouse_y)
		|| bufferstop2.handle_mouse(mouse_x, mouse_y);

	return(return_val);
}
void LINE::draw(SDL_Renderer* renderer,transform& trans,
		float mouse_x, float mouse_y){
	//cout << "draw:" << selected << endl;
	bool first = true;
	int prev = 0;

	bufferstop1.draw(renderer, trans, colour, mouse_x, mouse_y);
	bufferstop2.draw(renderer, trans, colour, mouse_x, mouse_y);

	node_t* head = first_station;


	while(head){

		if(first){
			prev = head->value;
			first = false;
			if(selected== head){
				trans.drawline(renderer, stations[head->value].pos.x, 
						stations[head->value].pos.y, mouse_x, 
						mouse_y, 8,colour.r, colour.g, colour.b, 255);
				trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
			}
				

		}
		else{
		float x0 = stations[prev].pos.x;
		float y0 = stations[prev].pos.y;
		float x1 = stations[head->value].pos.x;
		float y1 = stations[head->value].pos.y;


		if(selected == head && select_direction == PREV){
			trans.drawline(renderer, stations[head->value].pos.x, 
					stations[head->value].pos.y, mouse_x, 
					mouse_y, 8,colour.r, colour.g, colour.b, 255);
			trans.drawline(renderer, x0, y0, x1, y1, 8,colour.r, colour.g, colour.b, 255);
			trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
		}

		else if(selected == head){
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
	if(train.initialised && length >= 2){
		train.move(1/(double)framerate);
		train.draw(renderer, trans);
	}
}
