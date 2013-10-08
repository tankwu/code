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
#define LENGTH 10
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
double Kernal(int s, int L)
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

double LIC(int L, list<pnt2d_t> pnts)
{
	double s_0 = 0.0;

	double a = s_0 - L * 1.0;
	double b = s_0 + L * 1.0;
	
	double s = a;
	double func;
	
	double step = (b - a) / (10);
//	double step = (b - a) / (100);
	double result = 0.0;
	
	int m = 0;
	while (s < b)
	{
		int n = int(s - a);
		if (m == 10 && !pnts.empty()) {
			pnts.pop_front();
			m = 0;
		}
//		cout << pnts.front().y << " ";
		func = Kernal(s, L) * Text(pnts.front().x, pnts.front().y);
		result += step * func;
		s += step;
		m++;
	}
	
	return result;
}

pnt2d_t p_t;
pnt2d_t RKA(int x, int y, float deltaT)
{
//	float deltaT = 0.1;
	vec2d_t v_i;
	v_i = bilinear((x * 1.0) / (SCALE_X * 1.0),
		       (y * 1.0) / (SCALE_Y * 1.0), v_i);
	p_t.x = 0;
	p_t.y = 0;
//	pnt2d_t p;
	
	vec2d_t v_1_i;
	vec2d_t v_2_i;
	vec2d_t v_3_i;
	
	v_1_i.x = (float)x + (1.0 / 2.0) * deltaT * v_i.x;
	v_2_i.x = (float)x + (1.0 / 2.0) * deltaT * v_1_i.x;
	v_3_i.x = (float)x + deltaT * v_2_i.x;
	
	v_1_i.y = (float)y + (1.0 / 2.0) * deltaT * v_i.y;
	v_2_i.y = (float)y + (1.0 / 2.0) * deltaT * v_1_i.y;
	v_3_i.y = (float)y + deltaT * v_2_i.y;
	
	float tmpX = (float)x + (1.0 / 6.0) * deltaT * (v_i.x + 2 * v_1_i.x +
			 			        2 * v_2_i.x +v_3_i.x);
	float tmpY = (float)y + (1.0 / 6.0) * deltaT * (v_i.y + 2 * v_1_i.y +
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


int main()
{
// read the file
	data = new vec2d_t[WIDTH * HEIGHT];
	data_c = new vec2d_t[WIDTH * HEIGHT];	
	ifstream inf("tornado_64_2d.a2v", ios::binary);
	inf.read(reinterpret_cast<char*>(data),
		 WIDTH * HEIGHT * sizeof(vec2d_t));
	
	generateNoise();
	
// make a simple image 
	int width = WIDTH * SCALE_X;
   	int height = HEIGHT * SCALE_Y;
    
    	printf("Generate a simple image\n");
	pixel_t *image = (pixel_t *)malloc(width * height * sizeof(pixel_t));
    	if (!image) {
        	fprintf(stderr, "Cannot allocate memory\n");
        	return 0;            
    	} 
    
    	int x, y, index, tmpLen;

	float delta1 = 1;
	float delta2 = -1;
    
	pnt2d_t pnt0, pnt1;
	pnt2d_t pnt2, pnt3;
	for (y = 0; y < height; y++) {
        	for (x = 0; x < width; x++) {
            		index = x + y * width;
			pnt0.x = x;
			pnt0.y = y;
			pnt2.x = x;
			pnt2.y = y;
			tmpLen = 0;
			list<pnt2d_t> pnts;
			for (int i = 0; i < LENGTH; i++) {
				pnt1 = RKA(pnt0.x, pnt0.y, delta1);
				pnt3 = RKA(pnt2.x, pnt2.y, delta2);
				
				if (0 <= pnt1.x && pnt1.x <= width
				    && 0 <= pnt1.y && pnt1.y <= height) {
					pnts.push_back(pnt1);
					tmpLen++;
				}
				
				if (0 <= pnt3.x && pnt3.x <= width
				    && 0 <= pnt3.y && pnt3.y <= height) {
					pnts.push_front(pnt3);
					tmpLen++;
				}
				
				pnt0.x = pnt1.x;
				pnt0.y = pnt1.y;
				pnt2.x = pnt3.x;
				pnt2.y = pnt3.y;
			}
//			cout << x << " " << y << " ";
//			cout << pnts.size() << " " << endl;
//			cout << pnt1.x << " " << pnt1.y << endl;
//			cout << pnt3.x << " " << pnt3.y << endl;
			
			image[index].r = LIC(tmpLen, pnts);
			image[index].g = LIC(tmpLen, pnts);
			image[index].b = LIC(tmpLen, pnts);
			image[index].a = 1;
        }
    }
//	pnt0.x = 54;
//	pnt0.y = 8;
//	pnt2.x = 54;
//	pnt2.y = 8;
//	tmpLen = 0;
	
//	pnt1 = RKA(pnt0.x, pnt0.y);
//	pnt3 = RKA(pnt2.x, pnt2.y);
	
//	cout << data[1].x << " " << data[1].y << endl;
//	cout << data_c[1].x << " " << data_c[1].y << endl;
/*	
		list<pnt2d_t> pnts;
			for (int i = 0; i < LENGTH; i++) {
				pnt1 = RKA(pnt0.x, pnt0.y, delta1);
				pnt3 = RKA_c(pnt2.x, pnt2.y, delta2);
				
				cout << pnt1.x << " " << pnt1.y << endl;
				cout << pnt3.x << " " << pnt3.y << endl;
	
	
				if (0 <= pnt1.x && pnt1.x <= width
				    && 0 <= pnt1.y && pnt1.y <= height) {
					pnts.push_back(pnt1);
					tmpLen++;
				}
				if (0 <= pnt3.x && pnt3.x <= width
				    && 0 <= pnt3.y && pnt3.y <= height) {
					pnts.push_front(pnt3);
					tmpLen++;
				}
//				cout << "test" << endl;
				pnt0.x = pnt1.x;
				pnt0.y = pnt1.y;
				pnt2.x = pnt3.x;
				pnt2.y = pnt3.y;
			}
//			cout << tmpLen << endl;
//			cout << pnts.size() << endl;
			
	for (int i = 0; i < tmpLen; i++) {
		cout << pnts.front().x << " " << pnts.front().y << endl;
		cout << pnts.size() << endl;
		pnts.pop_front();
	}	
*/
// save the image to a ppm file 
	char str[] = "test.ppm";
    	printf("Save the image to test.ppm\n");
    	saveppm(width, height, image, str);
    
    	free(image);
    
    	return 0;
}
