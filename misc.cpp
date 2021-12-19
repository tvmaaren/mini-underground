#include <stdlib.h>
#include <math.h>

#include "types.hpp"
#include "misc.hpp"

float distance2d(Point2d a, Point2d b){
	return(sqrt(pow(a.x-b.x, 2) + pow(a.y-b.y, 2)));
}
float distance_line_to_point(Point2d point, Point2d line1, Point2d line2){
	float distance =MAXFLOAT;
	if( !
		((point.x >= 10+line1.x && point.x >= 10+line2.x) ||
		(point.x <= -10+line1.x && point.x <= -10+line2.x) ||
		(point.y >= 10+line1.y && point.y >= 10+line2.y) ||
		(point.y <= -10+line1.y && point.y <= -10+line2.y))
		){

	
		float slope = (line2.y-line1.y)/(line2.x-line1.x);
		float slope_perpendicular = -1/slope;
		float start_perpendicular = (line1.x-point.x)*slope_perpendicular+point.y;

		//The points where the distance to the line will be measured
		//from.
		float distance_x = (start_perpendicular-line1.y)/(slope-slope_perpendicular)+line1.x;
		float distance_y = slope*(distance_x-line1.x)+line1.y;
		
		distance = distance2d({distance_x, distance_y}, {point.x, point.y});
		


	}
	return(distance);

}
node_t* add_node_before(node_t* node){
	node_t *new_node_p  = (node_t*)malloc(sizeof(node_t));
	new_node_p->links[NEXT] = node;
	if(node){
		new_node_p->links[PREV] = node->links[PREV];
		if(node->links[PREV])
			node->links[PREV]->links[NEXT] = new_node_p;
		node->links[PREV] = new_node_p;
	}else{
		new_node_p->links[PREV] = NULL;
	}

	return(new_node_p);
}

node_t* add_node_after(node_t* node){
	node_t *new_node_p  = (node_t*)malloc(sizeof(node_t));
	new_node_p->links[PREV] = node;
	if(node){
		new_node_p->links[NEXT] = node->links[NEXT];
		if(node->links[NEXT])
			node->links[NEXT]->links[PREV] = new_node_p;
		node->links[NEXT] = new_node_p;
	}else{
		new_node_p->links[NEXT] = NULL;
	}

	return(new_node_p);
}
