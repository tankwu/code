#include <fstream>
#include <iostream>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <list>

#define WIDTH 64
#define HEIGHT 64
#define SCALE_X 3
#define SCALE_Y 3
#define LENGTH 4
#define PI 3.14159265


using namespace std;

typedef struct vec2d_t {
float x, y;
} vec2d_t;

typedef struct {
    float r, g, b, a;
} pixel_t;

struct colorMap
{
	double r;
	double g;
	double b;
}color_Max, color_Min, color;

typedef struct {
	int x;
	int y;
} pnt2d_t;

int saveppm(int width, int height, pixel_t *pixels, char *filename)
{
    uint64_t i;
    uint8_t r = 0, g = 0, b = 0;
    int t, x, y;

    FILE *fp;
    
    /* open in text mode */
    fp = fopen(filename, "w" );
    if (!fp) {
        fprintf(stderr, "Cannot open file %s.\n", filename);
        return 0;
    }
    
    fprintf(fp,"P6\n");    
    fprintf(fp,"%i %i\n", width, height);
    fprintf(fp,"255\n");
    fclose(fp);
    
    /* reopen in binary append mode */
    fp = fopen( filename, "ab" );  
    if (!fp) {
        fprintf(stderr, "Cannot open file %s.\n", filename);
        return 0;
    }

    for (y = height - 1; y >= 0; y--) {
        for (x = 0; x < width; x++) {
            i = x+ y * width;

            r = (uint8_t)(pixels[i].r * 0xFF);
            g = (uint8_t)(pixels[i].g * 0xFF);
            b = (uint8_t)(pixels[i].b * 0xFF);
            
            fputc(r, fp);
            fputc(g, fp);
            fputc(b, fp);
        }
    }
    
    fclose(fp);
    
    return 0;
}

double f_distance;
double distance(int p1x, int p1y, int p2x, int p2y)
{
	float f_distance = sqrt((p1x - p2x) * (p1x - p2x)
			 + (p1y - p2y) * (p1y - p2y));
	return f_distance;		
}

//vec2d_t *test;
vec2d_t *data, *data_c;
vec2d_t bilinear(float X, float Y, vec2d_t v)
{
	int X_0 = int(X);
	int Y_0 = int(Y);
	int X_1 = int(X + 1);
	int Y_1 = int(Y + 1);
	
	if (X_1 == WIDTH) {
		X_1 = X_1 - 1;
		X_0 = X_0 - 1;
	}
	
	if (Y_1 == HEIGHT) {
		Y_1 = Y_1 - 1;
		Y_0 = Y_0 - 1;
	}
	
	vec2d_t v_0, v_1;
	vec2d_t v_0_0, v_0_1, v_1_0, v_1_1;

	float U = (X - X_0) * 1.0 / (X_1 - X_0) * 1.0;
	float V = (Y - Y_0) * 1.0 / (Y_1 - Y_0) * 1.0;

	v_0_0.x = data[X_0 + WIDTH * Y_0].x;
	v_0_0.y = data[X_0 + WIDTH * Y_0].y;
	v_1_0.x = data[X_1 + WIDTH * Y_0].x;
	v_1_0.y = data[X_1 + WIDTH * Y_0].y;
	v_0_1.x = data[X_0 + WIDTH * Y_1].x;
	v_0_1.y = data[X_0 + WIDTH * Y_1].y;
	v_1_1.x = data[X_1 + WIDTH * Y_1].x;
	v_1_1.y = data[X_1 + WIDTH * Y_1].y;

	v_0.x = (1.0 - U) * v_0_0.x + (U) * v_1_0.x;
	v_0.y = (1.0 - U) * v_0_0.y + (U) * v_1_0.y;
	
	v_1.x = (1.0 - U) * v_0_1.x + (U) * v_1_1.x;
	v_1.y = (1.0 - U) * v_0_1.y + (U)* v_1_1.y;
	
	v.x = (1.0 - V) * v_0.x + (V) * v_1.x;
	v.y = (1.0 - V) * v_0.y + (V) * v_1.y;
	
	return v;
}

// create the color map
void buildColor ()
{
//	white
	color_Min.r = 0.0f;
	color_Min.g = 0.0f;
	color_Min.b = 0.0f;

	color.r = 0.0f;
	color.g = 0.0f;
	color.b = 0.0f; 
} 

// find the color of each pixel on the image
colorMap findColor (double colorValue) 
{
	double f_Grad = 0.3;
	double n_NorMax = 1.0;
	double n_NorMin = 0.0;
	
	double d_NGradient = n_NorMin - n_NorMax;
	double d_PGradient = n_NorMax - n_NorMin;
		
//	white
	if (colorValue <= 0) {
		color.r = color_Min.r;
		color.g = color_Min.g;
		color.b = color_Min.b;
		return color;
	}

	if (colorValue <= 1.0) {
		color.r = color_Min.r + (colorValue / 1.0f) * d_PGradient;
		color.g = color_Min.g + (colorValue / 1.0f) * d_PGradient;
		color.b = color_Min.b + (colorValue / 1.0f) * d_PGradient;
		return color;
	}
}

// the noise array
double noise[WIDTH * SCALE_X][HEIGHT * SCALE_Y]; 
void generateNoise()
{
    for (int x = 0; x < WIDTH * SCALE_X; x++)
    for (int y = 0; y < HEIGHT * SCALE_Y; y++)
    {
        noise[x][y] = (rand() % 32768) / 32768.0;
    }
}

double f_kFilter;
double Kernal(float s, float L)
{
	if (s < 0) {
		s = -s;
	}	
	double S = 90.0 * ((s * 1.0) / (L * 1.0));
	f_kFilter = cos(S * PI / 180.0);
	return f_kFilter;
}

double Text(int x, int y)
{
	return noise[x][y];
}

pnt2d_t p_t;
pnt2d_t RKA(pnt2d_t pnt, float deltaT)
{
//	float deltaT = 0.1;
	vec2d_t v_i;
	v_i = bilinear((pnt.x * 1.0) / (SCALE_X * 1.0),
		       (pnt.y * 1.0) / (SCALE_Y * 1.0), v_i);
	p_t.x = 0;
	p_t.y = 0;
//	pnt2d_t p;
	
	vec2d_t v_1_i;
	vec2d_t v_2_i;
	vec2d_t v_3_i;
	
	v_1_i.x = (float)pnt.x + (1.0 / 2.0) * deltaT * v_i.x;
	v_2_i.x = (float)pnt.x + (1.0 / 2.0) * deltaT * v_1_i.x;
	v_3_i.x = (float)pnt.x + deltaT * v_2_i.x;
	
	v_1_i.y = (float)pnt.y + (1.0 / 2.0) * deltaT * v_i.y;
	v_2_i.y = (float)pnt.y + (1.0 / 2.0) * deltaT * v_1_i.y;
	v_3_i.y = (float)pnt.y + deltaT * v_2_i.y;
	
	float tmpX = (float)pnt.x + (1.0 / 6.0) * deltaT * (v_i.x + 2 * v_1_i.x +
			 			        2 * v_2_i.x +v_3_i.x);
	float tmpY = (float)pnt.y + (1.0 / 6.0) * deltaT * (v_i.y + 2 * v_1_i.y +
						 	2 * v_2_i.y +v_3_i.y);
	
	if (tmpX - int(tmpX) > 0.5) {
		p_t.x = int(tmpX) + 1;
	} else {
		p_t.x = int(tmpX);
	}	
	if (tmpY - int(tmpY) > 0.5) {
		p_t.y = int(tmpY) + 1;
	} else {
		p_t.y = int(tmpY);
	}
	
	return p_t;
}

double LIC(double L, pnt2d_t pnt)
{
	float delta1 = 1.0;
	float delta2 = -1.0;
	double tmpLen_1, tmpLen_2;	
	double func;
	double weight;
	double tmpLen;
	int length;
	int count = 0;
	tmpLen = tmpLen_1 = tmpLen_2 = weight = 0.0;
	length = 2 * LENGTH;
	pnt2d_t p1, p2, p3, p4;
	p1 = pnt;
	p2 = pnt;
	while (tmpLen < LENGTH && count <= LENGTH) {
		p3 = RKA(p1, delta1);
		p4 = RKA(p2, delta2);
				
		if (0 <= p3.x && p3.x <= WIDTH * SCALE_X
			&& 0 <= p3.y && p3.y <= HEIGHT * SCALE_Y) {
			tmpLen_1 = tmpLen_1 + distance(p1.x, p1.y, p3.x, p3.y);
			func += Kernal(tmpLen_1, LENGTH * 1.0) * Text(p3.x, p3.y);
			weight += Kernal(tmpLen_1, LENGTH * 1.0);
		}
				
		if (0 <= p4.x && p4.x <= WIDTH * SCALE_X
			&& 0 <= p4.y && p4.y <= HEIGHT * SCALE_Y) {
			tmpLen_2 = tmpLen_2 + distance(p2.x, p2.y, p4.x, p4.y);
			func += Kernal(tmpLen_2, LENGTH * 1.0) * Text(p4.x, p4.y);
			weight += Kernal(tmpLen_2, LENGTH * 1.0);
		}	
		
		tmpLen = (tmpLen_1 <= tmpLen_2) ? tmpLen_2 : tmpLen_1;
		p1.x = p3.x;
		p1.y = p3.y;
		p2.x = p4.x;
		p2.y = p4.y;
		count++;
	}
	double final_result = tmpLen_1 / weight;
	return final_result;	
}


int main()
{
//  read the file
	data = new vec2d_t[WIDTH * HEIGHT];
	data_c = new vec2d_t[WIDTH * HEIGHT];
	ifstream inf("tornado_64_2d.a2v", ios::binary);
	inf.read(reinterpret_cast<char*>(data),
		 WIDTH * HEIGHT * sizeof(vec2d_t));
	
	generateNoise();
	
//  make a simple image 
	int width = WIDTH * SCALE_X;
   	int height = HEIGHT * SCALE_Y;
    
    printf("Generate a simple image\n");
	pixel_t *image = (pixel_t *)malloc(width * height * sizeof(pixel_t));
    if (!image) {
       	fprintf(stderr, "Cannot allocate memory\n");
        return 0;            
    } 
    
    int x, y, index;
	
	for (y = 0; y < height; y++) {
        	for (x = 0; x < width; x++) {
				pnt2d_t pnt;
            	index  = x + y * width;
				pnt.x = x;
				pnt.y = y;
				
				image[index].r = LIC(LENGTH * 1.0d, pnt);
				image[index].g = LIC(LENGTH * 1.0d, pnt);
				image[index].b = LIC(LENGTH * 1.0d, pnt);
				image[index].a = 1;
//				cout << x << " ";
       }
    }
	
//  save the image to a ppm file 
	char str[] = "test.ppm";
    printf("Save the image to test.ppm\n");
    saveppm(width, height, image, str);
    
	delete data, data_c, image;
    
    return 0;
}
