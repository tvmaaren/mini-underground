

typedef struct{
	float m[3][3];
}Matrix;

const Matrix identity = {{{1,0,0},{0,1,0},{0,0,1}}};
const Matrix nul = {{{0,0,0},{0,0,0},{0,0,0}}};

Matrix matrix_mul(Matrix A, Matrix B);
class Transform{
	public:
	Matrix m;
	float Scale;
	void drawline(SDL_Renderer* renderer, float x1, float y1, float x2,
			float y2, int thickness, int r, int g, int b, int a);
	void drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawrectangle(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, 
		int r, int g, int b, int a);
	void drawsquare(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawgem(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawngon(SDL_Renderer* renderer, int n,float start_angle, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawtriangle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawpentagon(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawshape(SDL_Renderer* renderer, SHAPE shape, float x, float y, float radius,
		int r, int g, int b, int a);
	void init();
	void translate(int x,int y);
	void rotate(float angle);
	void scale(float scale_new);

	private:
	void transform_coordinates(float *x, float *y);
};
