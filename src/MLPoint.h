// Jon Bardin GPL


extern "C" {

#if !defined(MIN)
    #define MIN(A,B)  ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __a : __b; })
#endif

#if !defined(MAX)
    #define MAX(A,B)  ({ __typeof__(A) __a = (A); __typeof__(B) __b = (B); __a < __b ? __b : __a; })
#endif


typedef struct _ccColor3B {
  GLubyte r;
  GLubyte g;
  GLubyte b;
} ccColor3B;

typedef struct _ccColor4F {
  GLfloat r;
  GLfloat g;
  GLfloat b;
  GLfloat a;
} ccColor4F;

typedef struct _MLPoint {
  float x;
  float y;
} MLPoint;


MLPoint MLPointMake(float x, float y);

};
