#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL.h>
#include <iostream>
#include <stdlib.h>


#include "transform.h"

const Matrix identity = {{{1,0,0},{0,1,0},{0,0,1}}};

void transform::drawline(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, int thickness,
		int r, int g, int b, int a){

	transform_coordinates(&x1, &y1);
	transform_coordinates(&x2, &y2);

	thickLineRGBA(renderer, x1, y1, x2, y2, thickness,r, g, b, a);
}

void transform::drawcircle(SDL_Renderer* renderer, float x, float y, float radius,
		int r, int g, int b, int a){
	transform_coordinates(&x, &y);
	filledCircleRGBA(renderer, x, y, radius, r, g, b, a);
}

Matrix transform::matrix_mul(Matrix A,Matrix B){
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

void transform::translate(int x,int y){

	
	Matrix translation_matrix = identity;
	translation_matrix.m[0][2] = x;
	translation_matrix.m[1][2] = y;

	m = matrix_mul(m, translation_matrix);
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
