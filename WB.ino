#include <Stepper.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

const int steps_rev = 200;                    //Steps per revolution of the stepper motor
const int steps_per_inch = 105;               //Tested, how many steps 
int go = 1;

const float width = 34.0;
const float height = 22.0;
const int base_speed = 200;

Stepper ulstep(steps_rev, 8, 9, 10, 11);
Stepper urstep(steps_rev, 4, 5, 6, 7);


int scount = 0;

void setup() {
  // nothing to do inside the setup
}

typedef struct point {
  float x;
  float y;
  int p;
} point ;

//typedef struct point point;

struct point pt_maker(float x, float y) {
  point ret;// = malloc(sizeof(point));
  ret.x = x;
  ret.y = y;
  ret.p = 1;                 //UPGRADE
  return ret;
}

point *small_inc(point *pts, int len) {
  //    This divides an array of points into small increments since Arduino can't drive two steppers at once
  int needed = 0;
  int i;
  float lendif;
  int valid = 1;
  for(i=0; i < len; i++) {
    point curr = pts[i];
    point next = pts[i+1];
    lendif = sqrt((((next.x - curr.x) * (next.x - curr.x)) + ((next.y - curr.y) * (next.y - curr.y))));
    if(lendif < 0.25) {
      needed += (int) round(lendif / 0.25);
    }
  }
  needed += i;
  int ind = 0;
  point *res = (point *) malloc((needed + 1) * sizeof(point));
  for(i = 0; i < (len - 1); i++) {
    res[ind] = pts[i];
    point curr = pts[i];
    point next = pts[i+1];
    ++ind;
    lendif = sqrt((((next.x - curr.x) * (next.x - curr.x)) + ((next.y - curr.y) * (next.y - curr.y))));
    if(lendif < 0.25) {
      int extra = (int) round(lendif / 0.25);
      float xdir = (float) ((next.x - curr.x) / (float) extra); 
      float ydir = (float) ((next.y - curr.y) / (float) extra); 
      for(int j=0; j<extra; j++) {
        res[ind].x = res[ind - 1].x + xdir;
        res[ind].y = res[ind - 1].y + ydir;
        ++ind;
      }
    }
  }
  res[ind] = pts[i];
  return res;
}


float lenL(point pt) {     
  //      Returns the length from the left stepper that the point is 
  float v = height - pt.y;
  return sqrt((pt.x * pt.x) + (v * v));
}


float lenR(point pt) {      
  //      Returns the length from the right stepper that the point is
  float u = width - pt.x;
  float v = height - pt.y;
  return sqrt((u * u) + (v * v));
}

void motor_move(point src, point dst) {   
   //     Moves the motors from the source point to the destination point       
  float lenR_src = lenR(src);
  float lenL_src = lenL(src);
  float lenR_dst = lenR(dst);
  float lenL_dst = lenL(dst);
  float lenL_diff = lenL_dst - lenL_src;
  float lenR_diff = lenR_dst - lenR_src;

    float lenratio = fabs(lenR_diff / lenL_diff);
  int rspeed, lspeed;
  if(lenratio > 1) {
    rspeed = (int) round(base_speed);
    lspeed = (int) round(base_speed / lenratio);
  } else {
    rspeed = (int) round(base_speed * lenratio);
    lspeed = (int) round(base_speed);
  }
  int rsteps, lsteps;
  rsteps =  (int) round(lenR_diff * steps_per_inch);
  lsteps =  (lenL_diff * steps_per_inch);
  urstep.setSpeed(rspeed);
  ulstep.setSpeed(lspeed);
  urstep.step(rsteps);
  ulstep.step(lsteps);
}


void points_run(point *pts, int num_pts) { 
      //          Runs through an array of points, moving to each after the last
  point src = pts[0];
  point dst;
  int i;
  for(i = 1; i<num_pts; i++) {
    dst = pts[i];
    motor_move(src, dst); 
    src = dst;                                    //IMPROVE THIS: Doesn't take into consideration that the actual motor
                                                  // movement might not be to the exact point specified (ex. halv-steps)
  }
}

struct point *make_circle(float rad, int pts, float center_y, float center_x) {
        // Returns an array of the points for a circle
  point *res = (point*) malloc((pts + 1) * sizeof(point));
  for(int i=0; i<=pts; i++) {
    double deg = (double) ((2 * 3.1415926535 * i) / pts);
    float y = (float) rad * sin(deg) + center_y;
    float x = (float) rad * cos(deg) + center_x;
    res[i].x = x;
    res[i].y = y;
  }
  return res;
}


void doit() {
    //          Makes a circle, then plots it
  point *pts = make_circle(2, 100, 3.5, 5.5);
  points_run(pts, 100);
}


void loop() {
  while(go == 1) {
  doit();
    go = 0;
  }
}



