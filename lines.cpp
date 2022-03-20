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

#include "settings.hpp"
#include "types.hpp"
#include "transform.hpp"
#include "misc.hpp"
#include "lines.hpp"
#include "stations.hpp"
#include "passengers.hpp"

#define trackwidth 8*trans.Scale

extern STATUS status;

extern STATION_LIST stations;
extern vector<TRAIN> trains;
extern vector<LINE> lines;


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
void BUFFER::draw(SDL_Renderer* renderer,Transform& trans, COLOUR colour, float mouse_x, float mouse_y){
	if(!created)
		return;
	if(status.play_status == PLAYING && handle_mouse(mouse_x, mouse_y)){
		trans.drawline(renderer, bottom_x, bottom_y, middle_x,
				middle_y, trackwidth,0,0,0,255);
		trans.drawline(renderer, left_x, left_y, right_x,
				right_y, trackwidth,0,0,0,255);
	}else{
		trans.drawline(renderer, bottom_x, bottom_y, middle_x,
				middle_y, trackwidth,colour.r, colour.g, colour.b, 255);
		trans.drawline(renderer, left_x, left_y, right_x,
				right_y, trackwidth,colour.r, colour.g, colour.b, 255);
	}

}

void TRAIN::draw(SDL_Renderer* renderer,Transform& trans){
	Transform train_trans;
        train_trans.m = identity;
	train_trans.rotate(atan(slope)+M_PI/2);
	train_trans.translate(x, y);
	train_trans.m = matrix_mul(trans.m,train_trans.m);
	train_trans.drawrectangle(renderer, -width, -height, width, height, colour.r,colour.g,colour.b,255);

	int passenger_i=0;
	for(int i=0; i<shapes; i++){
		for(int ii=0; ii<am_passengers_per_type[i]; ii++){
			train_trans.drawshape(renderer, int_to_shape(i),0, height-height/max_passengers-passenger_i*height*2/max_passengers, 4, 0,0,0, 255);
			passenger_i++;
		}
	}

}

//returns false if it has to go over
//removed tracks
bool TRAIN::find_next_station(){
	if(!start_line || !start_line->links[direction]){
		//first try to find te station on the line
		for(node_t* head = lines[line_id].first_station; head;
				head=head->links[NEXT]){
			if(head->value==station_id){
				start_line=head;
			}
		}
	}
	if(!start_line){
		for(node_t* head = *removed_segments; head;
				head=head->links[NEXT]->links[NEXT]){
			if(head->value==station_id){
				next_station = head->links[NEXT]->value;

				//make sure the train doesn't go back 
				//and forth
				if(next_station!=prev_station)
					break;
			}else if(head->links[NEXT]->value==station_id){
				next_station = head->value;
				//make sure the train doesn't go back 
				//and forth
				if(next_station!=prev_station)
					break;
			}
		}
		return false;
	}
	if(!start_line->links[direction])
		direction = direction ? PREV : NEXT;
	if(!start_line->links[direction]){
		for(node_t* head = *removed_segments; head;
				head=head->links[NEXT]->links[NEXT]){
			if(head->value==station_id){
				next_station = head->links[NEXT]->value;
			}else if(head->links[NEXT]->value==station_id){
				next_station = head->value;
			}
		}
		return false;
	}else{
		next_station = start_line->links[direction]->value;
	}
	return true;

}

void TRAIN::init(node_t* start_station,int new_line_id, node_t** new_removed_segments, LINK_DIRECTION in_direction, COLOUR colour_new){
	initialised = true;
	start_line = start_station;
	line_id = new_line_id;
	removed_segments = new_removed_segments;
	station_id = start_line->value;
	prev_station = start_line->value;
	direction = in_direction;
	colour = colour_new;
	for(int i=0; i<shapes; i++){
		am_passengers_per_type[i]=0;
	}
}
void TRAIN::move(float seconds){
	float dx;
	float dy;
	switch(location_type){
	case(AT_STATION):
		x = stations.stations[station_id].pos.x;
		y = stations.stations[station_id].pos.y;
		waiting_time_seconds-= seconds;
		if(waiting_time_seconds<0){
			location_type = ON_LINE;
			if(find_next_station())
				start_line = start_line->links[direction];
			x0 = x;
			y0 = y;

			x1 = stations.stations[next_station].pos.x;
			y1 = stations.stations[next_station].pos.y;
			slope = (y1-y0)/(x1-x0);
		}
		break;
			
	case(ON_LINE):
		dx = sqrt( velocity/(slope*slope+1) );
		if(x1-x0<0)
			dx = -dx;
		dy = slope*dx;
		x+=dx;
		y+=dy;


		//check if the train is beyond the station
		if( (x-x0)/(x1-x0) >1 ||
		    (y-y0)/(y1-y0) >1){
			prev_station = station_id;
			station_id = next_station;
			location_type = AT_STATION;
			waiting_time_seconds = STATION_WAIT_TIME;
			bool handle_passengers = find_next_station();
			lines[line_id].check_removed();
			if(handle_passengers){
			SHAPE shape = stations.stations[station_id].shape;
			passengers-=am_passengers_per_type[shape];
			status.points += am_passengers_per_type[shape];;
			am_passengers_per_type[shape]=0;

			//remove passsengers from train
			//and add to station
			for(int i=0; i<shapes; i++){
				if(should_leave(int_to_shape(i), station_id)){
					
					stations.stations[station_id].am_passengers_per_type[i]+=
						am_passengers_per_type[i];
					stations.stations[station_id].am_passengers+=
						am_passengers_per_type[i];
					passengers-=am_passengers_per_type[i];
					am_passengers_per_type[i]=0;
				}
			}
			

			//add passsengers

			int delta;
			for(int i=0; i<shapes; i++){
				if(!stations.used_shape[i])
					continue;
				if(stations.stations[station_id].am_passengers_per_type[i]==0)
					continue;
				if(should_enter(int_to_shape(i), station_id)){
					delta=stations.stations[station_id].passenger_leavestation(
							max_passengers-passengers,int_to_shape(i));
					am_passengers_per_type[i]+=delta;
					passengers+=delta;
				}
			}
			}
		}
		break;
	case(ON_REMOVED_LINE):
		float dx = sqrt( velocity/(slope*slope+1) );
		if(x1-x0<0)
			dx = -dx;
		float dy = slope*dx;
		x+=dx;
		y+=dy;
		//check if the train is beyond the station
		if( (x-x0)/(x1-x0) >1 ||
		    (y-y0)/(y1-y0) >1){
			station_id = next_station;
			location_type = AT_STATION;
			waiting_time_seconds = STATION_WAIT_TIME;
		}
		
		break;
	}

}
//Returns true if the passengers of shape %shape% should leave the train
bool TRAIN::should_leave(SHAPE shape, int station_id){
	return(!should_enter(shape, station_id));
}

//Returns true if the passengers of shape %shape% at station %station_id% should
//enter train
bool TRAIN::should_enter(SHAPE shape, int station_id){
	//compare the minimum distance of the next station
	//to the current station
	
	int dist_current_station = min_distance_stations(shape, station_id, NULL);
	int next_station;
	if(start_line->links[direction]){
		next_station = start_line->links[direction]->value;
	}else{
		//search next station in removed lines list
		for(node_t* head = *removed_segments; head; head=head->links[NEXT]->links[NEXT]){
			if(head->value==station_id){
				next_station = head->links[NEXT]->value;
			}else if(head->links[NEXT]->value==station_id){
				next_station = head->value;
			}
		}
	}


	int dist_next_station = min_distance_stations(shape, next_station, NULL);

	return(dist_next_station<=dist_current_station);
}

void LINE::create(COLOUR new_colour){
	colour = new_colour;
}



//make sure the station is already put in the right list when you add it
void LINE::click_add(int station_id){
	//If you try add the station you are selecting remove te selected
	//station from the line
	for(int  action : actions){
		if(station_id==action)return;
	}
	actions.push_back(station_id);

	bool remove_next_station = selected && selected->value == station_id;
	bool remove_prev_station = selected && (selected->links[PREV] &&
			selected->links[PREV]->value == station_id);

	node_t* to_be_removed;
	if(remove_next_station){
		to_be_removed = selected;
	}else if(remove_prev_station){
		to_be_removed = selected->links[PREV];
	}

	//remove a station
	//if the station is next to itself
	if((remove_next_station || remove_prev_station)){
		if(length>2){
		//add segments to removed_segments
		if(to_be_removed->links[NEXT]){
			removed_segments=add_node_before(removed_segments);
			removed_segments->value = to_be_removed->value; 

			removed_segments=add_node_before(removed_segments);
			removed_segments->value=to_be_removed->links[NEXT]->value;
		}
		if(selected->links[PREV]){
			removed_segments=add_node_before(removed_segments);
			removed_segments->value = to_be_removed->value; 

			removed_segments=add_node_before(removed_segments);
			removed_segments->value=to_be_removed->links[PREV]->value;
		}



		//removes nodes from the list assuming
		//it only occures once
		for(unsigned int i =0;  i<stations.stations[station_id].nodes.size();i++){
			if(stations.stations[station_id].nodes[i] == to_be_removed){
				stations.stations[station_id].nodes.erase(stations.stations[station_id].nodes.begin()+i);
				break;
			}
		}

		bool replace_first_station=
			first_station==to_be_removed;
		bool replace_last_station=
			first_station==to_be_removed;
		node_t* adjacent = remove_node(to_be_removed);
		selected = adjacent;
		if(replace_first_station)
			first_station = adjacent;
		if(replace_last_station)
			first_station = adjacent;

		//go through all trains to check if start_line
		//is equal to the now removed node
		if(trains[train_id].start_line==to_be_removed){
			trains[train_id].start_line = NULL;
		}

		length--;
		check_removed();
		}
	//add a station	
	}else{
		if(selected){

			//maybe remove from removed_segments
			int start_id = selected->value;
			int end_id = station_id;

			node_t* following;

			for(node_t* head = removed_segments; head; head = following){
				if((head->value==start_id && head->links[NEXT]->value==end_id)||
					(head->links[NEXT]->value==start_id && head->value==end_id)){

					if(head == removed_segments)
						removed_segments = head->links[NEXT]->links[NEXT];
				
					node_t* next = head->links[NEXT];
					following = next->links[NEXT];
					remove_node(head);
					remove_node(next);
				}else{
					following = head->links[NEXT]->links[NEXT];
				}
			}
		}


		if(select_direction==NEXT)
			selected = add_node_before(selected);
		else
			selected = add_node_after(selected);

		if(selected->links[NEXT] && selected->links[PREV]){
			removed_segments = add_node_before(removed_segments);//.push_back(selected->links[NEXT]->value);
			removed_segments->value=selected->links[NEXT]->value; 
			removed_segments = add_node_before(removed_segments);
			removed_segments->value=selected->links[NEXT]->value; 
		}

		selected->value = station_id;
		stations.stations[station_id].nodes.push_back(selected);


		length ++;
	}
	if(!selected->links[PREV]){
		first_station = selected;
	}
	if(!selected->links[NEXT]){
		last_station = selected;
	}

	if(!set_train_id){
		train_id = trains.size();
		trains.resize(trains.size()+1);
		set_train_id=true;
	}
	if(length >=2 && !trains[train_id].initialised){
		trains[train_id].init(first_station,id,&removed_segments, NEXT, colour);
	}

	if(!first_station->links[PREV] && length>=2){
		bufferstop1.create(stations.stations[first_station->value].pos, stations.stations[first_station->links[NEXT]->value].pos);
	}
	if(!last_station->links[NEXT] && length>=2){
		bufferstop2.create(stations.stations[last_station->value].pos, stations.stations[last_station->links[PREV]->value].pos);
	}
	return;
}

void LINE::click_select(){
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
	selected = NULL;
	actions.clear();
}
bool LINE::handle_mouse(float mouse_x, float mouse_y){

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
		float x0 =stations.stations[prev].pos.x;
		float y0 =stations.stations[prev].pos.y;
		float x1 =stations.stations[head->value].pos.x;
		float y1 =stations.stations[head->value].pos.y;

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
void LINE::draw(SDL_Renderer* renderer,Transform& trans,
		float mouse_x, float mouse_y){
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
				trans.drawline(renderer, stations.stations[head->value].pos.x, 
						stations.stations[head->value].pos.y, mouse_x, 
						mouse_y, trackwidth,colour.r, colour.g, colour.b, 255);
				trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
			}
				

		}
		else{
		float x0 = stations.stations[prev].pos.x;
		float y0 = stations.stations[prev].pos.y;
		float x1 = stations.stations[head->value].pos.x;
		float y1 = stations.stations[head->value].pos.y;


		if(selected == head && select_direction == PREV){
			trans.drawline(renderer, stations.stations[head->value].pos.x, 
					stations.stations[head->value].pos.y, mouse_x, 
					mouse_y, trackwidth,colour.r, colour.g, colour.b, 255);
			trans.drawline(renderer, x0, y0, x1, y1, trackwidth,colour.r, colour.g, colour.b, 255);
			trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
		}

		else if(selected == head){
			trans.drawline(renderer, x0, y0, x1, y1, trackwidth,colour.r, colour.g, colour.b, 127);
			trans.drawline(renderer, x0, y0, mouse_x, mouse_y, trackwidth,colour.r, colour.g, colour.b, 255);
			trans.drawline(renderer, mouse_x, mouse_y, x1, y1, trackwidth,colour.r, colour.g, colour.b, 255);
			trans.drawcircle(renderer, mouse_x, mouse_y, 8, 255, 0, 0, 255);
		}

		else if(hovering== head){
			trans.drawline(renderer, x0, y0, x1, y1, trackwidth,0, 0, 0, 255);
		}
		else
			trans.drawline(renderer, x0, y0, x1, y1, trackwidth,colour.r, colour.g, colour.b, 255);
		prev = head->value;
		}
		head = head->links[NEXT];
	}
	if(trains[train_id].initialised && length >= 2){
		if(status.play_status == PLAYING)
			trains[train_id].move(1/(double)framerate);
		trains[train_id].draw(renderer, trans);
	}

	//draw the removed sections
	for(node_t* head = removed_segments; head; head = head->links[NEXT]->links[NEXT]){
		trans.drawline(renderer, stations.stations[head->value].pos.x,
					 stations.stations[head->value].pos.y,
					 stations.stations[head->links[NEXT]->value].pos.x,
					 stations.stations[head->links[NEXT]->value].pos.y,
					 trackwidth,colour.r, colour.g, colour.b, 127);
	}

}

void LINE::check_removed(){
	node_t* following;
	for(node_t* head = removed_segments; head; head = following){
		//it will only remove segment from removed_segments if
		//there are no trains on the the segment and start_line is not set to null

		following = head->links[NEXT]->links[NEXT];
		node_t* next = head->links[NEXT];
		int station_1 = head->value;
		int station_2 = head->links[NEXT]->value;

		if(trains[train_id].start_line && !(
			((station_1 == trains[train_id].station_id) 	&&
			 (station_2 == trains[train_id].next_station)) 	||
			((station_2 == trains[train_id].station_id) 	&&
			 (station_1 == trains[train_id].next_station)))
		  ){
			
			
			if(head == removed_segments)
				removed_segments = head->links[NEXT]->links[NEXT];

			remove_node(head);
			remove_node(next);
		}
			
	}
}
