
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

void TRAIN::init(node_t* start_station, node_t** new_removed_segments, LINK_DIRECTION in_direction, COLOUR colour_new){
	initialised = true;
	start_line = start_station;
	removed_segments = new_removed_segments;
	station_id = start_line->value;
	direction = in_direction;
	colour = colour_new;
	for(int i=0; i<shapes; i++){
		am_passengers_per_type[i]=0;
	}
}
void TRAIN::move(float seconds){
	switch(location_type){
	case(AT_STATION):
		x = stations.stations[station_id].pos.x;
		y = stations.stations[station_id].pos.y;
		waiting_time_seconds-= seconds;
		if(waiting_time_seconds<0){
			location_type = ON_LINE;
			if(!start_line->links[direction])
					direction = direction ? PREV : NEXT;
				start_line = start_line->links[direction];
			break;
		}
		x0 = stations.stations[start_line->value].pos.x;
		y0 = stations.stations[start_line->value].pos.y;
		x1 = stations.stations[start_line->links[direction]->value].pos.x;
		y1 = stations.stations[start_line->links[direction]->value].pos.y;
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
			//remove passengers from train
			SHAPE shape = stations.stations[station_id].shape;
			passengers-=am_passengers_per_type[shape];
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
}
//Returns true if the passengers of shape %shape% should leave the train
bool TRAIN::should_leave(SHAPE shape, int station_id){
	return(!should_enter(shape, station_id));
}

//Returns true if the passengers of shape %shape% at station %station_id% should
//enter train %train%
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
