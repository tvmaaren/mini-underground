
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
	void draw(SDL_Renderer* renderer,transform& trans, COLOUR colour, float mouse_x, float mouse_y);
};

class TRAIN{
	public:

	bool initialised =false;

	private:
	enum {AT_STATION, ON_LINE} location_type = AT_STATION;
	LINK_DIRECTION direction;
	
	int station_id;
	float waiting_time_seconds = STATION_WAIT_TIME;

	node_t* start_line;
	float velocity = 1;

	float width = 10;
	float height = 20;

	float x0;
	float y0;
	float x1;
	float y1;
	float slope;

	float x,y;
	public:
	void draw(SDL_Renderer* renderer,transform& trans);

	void init(node_t* start_station, LINK_DIRECTION in_direction);
	void move(float seconds);
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
	
	
	public:
	void create(COLOUR new_colour);
	//make sure the station is already put in the right list when you add it
	void click_add(int station_id);

	void click_select();
	void unselect();
	bool handle_mouse(float mouse_x, float mouse_y);
	void draw(SDL_Renderer* renderer,transform& trans,
			float mouse_x, float mouse_y);
};
