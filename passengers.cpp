#ifdef __ANDROID__
#include <SDL.h>
#else
#include <SDL2/SDL.h>
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

extern STATION_LIST stations;

//recursively determine the smallest distance to
//a station with shape %SHAPE% from station %station_id%
int min_distance_stations(SHAPE shape, int station_id, bool* visited){

	if(stations.stations[station_id].shape == shape)
		return 0;

	if(!visited){
	
		visited = (bool*)malloc(sizeof(bool)*stations.stations.size());
		for(unsigned int i=0; i<stations.stations.size(); i++){
			visited[i]=false;
		}
	}

	visited[station_id] = true;
	int min = INT_MAX;

	//find all the lines that the station is on.
	for(int link_i = 0; link_i<2; link_i++){
	for(node_t* node : stations.stations[station_id].nodes){
		node_t* adjacent_node =node->links[link_i];
		if(!adjacent_node)
			continue;
		int adjacent_station = adjacent_node->value;
		
		if(visited[adjacent_station])
			continue;
		visited[adjacent_station]= 1;

		if(stations.stations[adjacent_station].shape == shape){
			return(1);
		}else{
			int min_adjcent = min_distance_stations(shape, adjacent_station, visited);
			if(min_adjcent+1<=min) min=min_adjcent+1;

		}
	}}
	return(min);
}
