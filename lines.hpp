
const float STATION_WAIT_TIME = 0.1;


class BUFFER{
	private:

	float left_x;
	float left_y;
	float right_x;
	float right_y;
	float middle_x;
	float middle_y;
	float bottom_x;
	float bottom_y;

	Point2d station1_tmp;
	Point2d station2_tmp;

	
	public:
	bool hovering;
	bool created = false;
	void create(Point2d station1, Point2d station2);
	bool handle_mouse(float mouse_x, float mouse_y);
	void draw(SDL_Renderer* renderer,Transform& trans, COLOUR colour, float mouse_x, float mouse_y);
};

class TRAIN{
	public:

	bool initialised =false;

	enum {AT_STATION, ON_LINE} location_type = AT_STATION;
	LINK_DIRECTION direction;

	int passengers;
	int am_passengers_per_type[shapes];
	int max_passengers=4;
	
	int station_id;
	float waiting_time_seconds = STATION_WAIT_TIME;

	node_t** removed_segments;
	node_t* start_line;
	float velocity = 1;

	float width = 10;
	float height = 20;
	COLOUR colour;

	float x0;
	float y0;
	float x1;
	float y1;
	float slope;

	float x,y;
	void draw(SDL_Renderer* renderer,Transform& trans);

	void init(node_t* start_station,node_t** new_removed_segments, LINK_DIRECTION in_direction, COLOUR colour_new);
	void move(float seconds);
	bool should_enter(SHAPE shape, int station_id);
	bool should_leave(SHAPE shape, int station_id);
};
class LINE{
	private:
	node_t* first_station;
	node_t* last_station;
	public:
	node_t* selected;
	TRAIN train;
	int length=0;
	BUFFER bufferstop1;
	BUFFER bufferstop2;
	COLOUR colour;
	LINK_DIRECTION select_direction=NEXT;
	node_t* hovering;
	
	void create(COLOUR new_colour);
	//make sure the station is already put in the right list when you add it
	void click_add(int station_id);

	void click_select();
	void unselect();
	bool handle_mouse(float mouse_x, float mouse_y);
	void draw(SDL_Renderer* renderer,Transform& trans,
			float mouse_x, float mouse_y);

	//When selection starts it keeps track of stations that
	//get added or removed
	vector <int> actions;

	//keeps track of all tracks that the player wants to
	//remove, but cannot be removed, because a train
	//still has to go over it
	//TODO: Make it a dynamic
	node_t* removed_segments=NULL;
};
