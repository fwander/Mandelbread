#include "lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
  unsigned char* top;
  char* name;
  unsigned width;
  unsigned height;
} img;

typedef struct{
  double a;
  double b;
} cmplx;

cmplx* newCmplx(double a, double b){
  cmplx* ret = (cmplx*) malloc(sizeof(cmplx));
  ret->a = a;
  ret->b = b;
  return ret;
}

cmplx* mult(cmplx* a, cmplx* b){
  cmplx* res = (cmplx*) malloc(sizeof(cmplx));
  res->a = a->a * b->a - a->b * b->b;
  res->b = a->a * b->b + a->b * b->a;
  return res;
}

cmplx* multInPlace(cmplx* a){
  double a_temp = a->a;
  double b_temp = a->b;

  a->a = a_temp * a_temp - b_temp * b_temp;
  a->b = a_temp * b_temp + b_temp * a_temp;
  return a;
}

cmplx* addInPlace(cmplx* a, cmplx* b){
  a->a = a->a + b->a;
  a->b = a->b + b->b;
  return a;
}

double sqrMag(cmplx* input){
  return input->a * input->a + input->b * input->b;
}

int escape(cmplx* input){
  int i = 0;
  int BO = 101;
  cmplx* temp = newCmplx(0,0);
  while(sqrMag(temp) < 4.0 && i++ < BO){
    addInPlace(multInPlace(temp), input);
  }
  free(temp);
  return i;
}

void encode(img* input) {
  /*Encode the image*/
  unsigned error = lodepng_encode32_file(input->name, input->top, input->width, input->height);

  /*if there's an error, display it*/
  if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

img* newImg(char* name, unsigned width, unsigned height){
  img* ret = (img*) malloc(sizeof(img));
  ret->name = (char*) malloc(strlen(name));
  memcpy(ret->name, name, strlen(name));
  ret->top = (unsigned char*) malloc(width * height * 4);
  ret->width = width;
  ret->height = height;
  return ret;
}

char* getPix(img* input, int x, int y){
  return input->top + (4 * input->width * y + 4 * x);  
}

void setPix(img* input, int x, int y, unsigned char r, unsigned char g, unsigned char b, unsigned char a){
  unsigned char* pix = getPix(input,x,y);
  pix[0] = r;
  pix[1] = g;
  pix[2] = b;
  pix[3] = a;
}

int main(int argc, char *argv[]) {
  char* filename =  "test.png";

  unsigned width = 2048, height = 2048;
  img* image = newImg(filename, width, height);
  unsigned x, y;
  double x1 = atof(argv[2]);
  double y1 = atof(argv[3]);
  
  double x2 = atof(argv[4]);
  double y2 = atof(argv[5]);
  
  cmplx* num = newCmplx(0.0,0.0);
#pragma omp parallel for
  for(y = 0; y < height; y++){
  for(x = 0; x < width; x++) {
    num->a = x1 + ((double)x/width)*(x2); 
    num->b = y1 + ((double)y/height)*(y2);
    int it = escape(num);
    setPix(image, x, y, 255 * (it > 100), 255 * (it > 100), 255 * (it > 100) ,255);
  }
  }
  encode(image);

  free(image);

  return 0;
}
