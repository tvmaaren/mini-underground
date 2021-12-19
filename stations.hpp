class STATION{
	private:
	public:
	int am_passengers=0;
	Point2d pos;
	int id;
	int randomval;
	void draw(SDL_Renderer* renderer,transform& trans, COLOUR colour);
	void create(float x, float y, int id);
	void add_passenger();
};
