#ifdef __ANDROID__
#include <SDL2_gfxPrimitives.h>
#include <SDL.h>
#else
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <iostream>
#endif
#include <stdlib.h>


#include "types.hpp"
#include "transform.hpp"


void Transform::drawline(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, int thickness,
		int r, int g, int b, int a){

	transform_coordinates(&x1, &y1);
	transform_coordinates(&x2, &y2);

	thickLineRGBA(renderer, x1, y1, x2, y2, thickness,r, g, b, a);
}

void Transform::drawrectangle(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, 
		int r, int g, int b, int a){
	
	
	float fvertices_x[] = {x1, x2, x2, x1};
	float fvertices_y[] = {y1, y1, y2, y2};

	Sint16 vertices_x[4];
	Sint16 vertices_y[4];
	
	for(int i=0; i<4; i++){
		transform_coordinates(fvertices_x+i,fvertices_y+i);
		vertices_x[i] = fvertices_x[i];
		vertices_y[i] = fvertices_y[i];
	}
	
	filledPolygonRGBA(renderer, vertices_x, vertices_y, 4,r, g, b, a);

}

void Transform::drawngon(SDL_Renderer* renderer, int n, float start_angle, float x, float y, float radius,
		int r, int g, int b, int a){
	float fvertices_x[n];
	float fvertices_y[n];
	
	Sint16 vertices_x[n];
	Sint16 vertices_y[n];

	for(int i=0; i<n; i++){
		fvertices_x[i] = cos(2.0/(float)n*M_PI*i+start_angle)*radius+x;
		fvertices_y[i] = sin(2.0/(float)n*M_PI*i+start_angle)*radius+y;
		transform_coordinates(fvertices_x+i, fvertices_y+i);
		vertices_x[i] = fvertices_x[i];
		vertices_y[i] = fvertices_y[i];
	}
	filledPolygonRGBA(renderer, vertices_x, vertices_y, n,r, g, b, a);
}

void Transform::drawtriangle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	drawngon(renderer, 3,-M_PI/2, x, y, radius, r, g, b, a);
}
void Transform::drawpentagon(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	drawngon(renderer, 5,-M_PI/2, x, y, radius, r, g, b, a);
}

void Transform::drawsquare(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	//drawrectangle(renderer, x-radius, y-radius, x+radius, y+radius, r, g, b, a);
	drawngon(renderer, 4,M_PI/4, x, y, radius, r, g, b, a);
}
void Transform::drawgem(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	//drawrectangle(renderer, x-radius, y-radius, x+radius, y+radius, r, g, b, a);
	drawngon(renderer, 4,0, x, y, radius, r, g, b, a);
}
	
void Transform::drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	drawngon(renderer, 100,0, x, y, radius, r, g, b, a);
	//transform_coordinates(&x, &y);
	//filledCircleRGBA(renderer*scale, x, y, radius, r, g, b, a);
}

void Transform::drawshape(SDL_Renderer* renderer, SHAPE shape, float x, float y, float radius,
		int r, int g, int b, int a){
	switch(shape){
		case(SQUARE):
			drawsquare(renderer, x,y,radius,r,g,b,a);
			break;
		case(CIRCLE):
			drawcircle(renderer, x,y,radius,r,g,b,a);
			break;
		case(TRIANGLE):
			drawtriangle(renderer, x,y,radius,r,g,b,a);
			break;
		case(PENTAGON):
			drawpentagon(renderer, x,y,radius,r,g,b,a);
			break;
		case(GEM):
			drawgem(renderer, x,y,radius,r,g,b,a);
			break;
	}
}

Matrix matrix_mul(Matrix A,Matrix B){
	//multiplies two 3x3 matrices
	Matrix out;
	for(int i = 0; i<3; i++){
		for(int j = 0; j<3; j++){
			float value = 0;
			for(int k = 0; k<3; k++){
				value+= A.m[i][k] * B.m[k][j];
			}
			out.m[i][j] = value;
		}
	}
	return out;
}
void Transform::init(){
	m  = identity;
}

void Transform::rotate(float angle){
	Matrix rotation_matrix ={{
	{ cos(angle), 	-sin(angle), 	0},
	{ sin(angle), 	cos(angle), 	0},
	{ 0, 		0, 		1}
	}};
	m = matrix_mul(rotation_matrix, m);
}


void Transform::translate(int x,int y){

	
	Matrix translation_matrix = identity;
	translation_matrix.m[0][2] = x;
	translation_matrix.m[1][2] = y;

	m = matrix_mul(translation_matrix, m);
}
void Transform::scale(float scale){
	Matrix scale_matrix = nul;
	for(int i=0; i<3; i++){
		scale_matrix.m[i][i] = scale;
	}
	m = matrix_mul(scale_matrix, m);

}
void Transform::transform_coordinates(float*x, float*y){
	float in_pos_vector[] = {*x, *y, 1};
	float out_pos_vector[3];
	for(int i = 0; i<3; i++){
		float value=0;
		for(int k = 0; k<3; k++){
			value+= m.m[i][k] * in_pos_vector[k];
		}
		out_pos_vector[i] = value;
	}
	*x = out_pos_vector[0];
	*y = out_pos_vector[1];
	return;
}
