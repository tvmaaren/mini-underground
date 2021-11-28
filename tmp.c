float distance_to_line(Point2d point, float x1, float y1,float x2,float y2){
	float slope = (y2-y1)/(x2-x1);
	float slope_perpendicular = -1/slope;
	float start_perpendicular = (x1-point.x)*slope_perpendicular+point.y;

	//The points where the distance to the line will be measured
	//from.
	float distance_x = (start_perpendicular-y1)/(slope-slope_perpendicular)+x1;
	float distance_y = slope*(distance_x-x1)+y1;

	float distance = distance2d({distance_x, distance_y, 0}, point);

	if(distance<closest_line_distance&&distance<10){
		cout << "found" << endl;
		hovering.type = LINEPIECE_OBJECT;
		hovering.node_p = head;
		closest_line_distance = distance;
	}
}


