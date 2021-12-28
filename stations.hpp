class STATION{
	private:
	public:
	int am_passengers=0;
	int am_passengers_per_type[2]={0,0};
	Point2d pos;
	int id;
	int randomval;
	void draw(SDL_Renderer* renderer,transform& trans, COLOUR colour);
	void create(float x, float y, int id);
	void add_passenger();
	SHAPE shape;
	int passenger_leavestation(int room_in_train, SHAPE shape);
};

class STATION_LIST{
	private:
		vector<STATION> stations;
		float max_station_x=0;
		float max_station_y=0;
		float min_station_x=0;
		float min_station_y=0;
		bool hovering;//true if mouse is hovering above a station
	public:
		int hovering_id;
		int add(float x, float y);
		int random_add();
		bool check_hovering(float mouse_x, float mouse_y);
		void draw(SDL_Renderer* renderer, transform& trans);

};
