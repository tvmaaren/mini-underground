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
