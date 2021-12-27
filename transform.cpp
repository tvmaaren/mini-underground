#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <stdlib.h>


#include "transform.hpp"


void transform::drawline(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, int thickness,
		int r, int g, int b, int a){

	transform_coordinates(&x1, &y1);
	transform_coordinates(&x2, &y2);

	thickLineRGBA(renderer, x1, y1, x2, y2, thickness,r, g, b, a);
}

void transform::drawrectangle(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, 
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

void transform::drawsquare(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	drawrectangle(renderer, x-radius

	
void transform::drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	transform_coordinates(&x, &y);
	filledCircleRGBA(renderer, x, y, radius, r, g, b, a);
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
void transform::init(){
	m  = identity;
}

void transform::rotate(float angle){
	Matrix rotation_matrix ={{
	{ cos(angle), 	-sin(angle), 	0},
	{ sin(angle), 	cos(angle), 	0},
	{ 0, 		0, 		1}
	}};
	m = matrix_mul(rotation_matrix, m);
}

void transform::translate(int x,int y){

	
	Matrix translation_matrix = identity;
	translation_matrix.m[0][2] = x;
	translation_matrix.m[1][2] = y;

	m = matrix_mul(translation_matrix, m);
}

void transform::transform_coordinates(float*x, float*y){
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
