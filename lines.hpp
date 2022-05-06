
const float STATION_WAIT_TIME = 0.5;


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
	void draw(SDL_Renderer* renderer,Transform& trans, Uint32 colour, float mouse_x, float mouse_y);
};

class TRAIN{
	public:

	bool initialised =false;

	LOCATION_TYPE location_type = AT_STATION;
	LINK_DIRECTION direction;

	int passengers;
	int am_passengers_per_type[shapes];
	int max_passengers=4;
	
	int next_station=0;
	int station_id;
	int prev_station;
	float waiting_time_seconds = STATION_WAIT_TIME;

	node_t** removed_segments;
	int line_id;
	node_t* start_line;
	float velocity = 1;

	float width = 10;
	float height = 20;
	Uint32 colour;

	float x0;
	float y0;
	float x1;
	float y1;
	float slope;

	float x,y;
	void draw(SDL_Renderer* renderer,Transform& trans);

	void init(node_t* start_station,int new_line_id, node_t** new_removed_segments, LINK_DIRECTION in_direction, Uint32 colour_new);
	void move(float seconds);
	bool should_enter(SHAPE shape, int station_id);
	bool should_leave(SHAPE shape, int station_id);
	int get_next_station();
	bool find_next_station();
};
class LINE{
	public:
	bool used=false;
	unsigned int id;
	node_t* first_station;
	node_t* last_station;
	node_t* selected;
	bool set_train_id=false;//true if it has assigned train_id
	int train_id;
	int length=0;
	BUFFER bufferstop1;
	BUFFER bufferstop2;
	Uint32 colour;
	LINK_DIRECTION select_direction=NEXT;
	node_t* hovering;
	
	void create(Uint32 new_colour, int new_id);
	void use();
	void stop_using();
	//make sure the station is already put in the right list when you add it
	void click_add(int station_id);//true if the line should be removed

	void click_select();
	void unselect();
	bool handle_mouse(float mouse_x, float mouse_y);
	void draw(SDL_Renderer* renderer,Transform& trans,
			float mouse_x, float mouse_y);
	void remove_station(node_t* node/*node belonging to station*/);
	void check_removed();//remove as many elemements from removed_segments as possible

	//When selection starts it keeps track of stations that
	//get added or removed
	vector <int> actions;

	//keeps track of all tracks that the player wants to
	//remove, but cannot be removed, because a train
	//still has to go over it
	node_t* removed_segments=NULL;
};
