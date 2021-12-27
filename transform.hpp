

typedef struct{
	float m[3][3];
}Matrix;

const Matrix identity = {{{1,0,0},{0,1,0},{0,0,1}}};

Matrix matrix_mul(Matrix A, Matrix B);
class transform{
	public:
	Matrix m;
	void drawline(SDL_Renderer* renderer, float x1, float y1, float x2,
			float y2, int thickness, int r, int g, int b, int a);
	void drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void drawrectangle(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, 
		int r, int g, int b, int a);
	void drawsquare(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void init();
	void translate(int x,int y);
	void rotate(float angle);

	private:
	void transform_coordinates(float *x, float *y);
};
