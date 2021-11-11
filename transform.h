typedef struct{
	float m[3][3];
}Matrix;

class transform{
	public:
	Matrix m;
	void drawline(SDL_Renderer* renderer, float x1, float y1, float x2,
			float y2, int thickness, int r, int g, int b, int a);
	void drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a);
	void init();
	void translate(int x,int y);

	private:
	Matrix matrix_mul(Matrix A, Matrix B);
	void transform_coordinates(float *x, float *y);
};
