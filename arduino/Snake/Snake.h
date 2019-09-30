#ifndef SNAKE_H
#define SNAKE_H
#include <FastLED.h>
#include "MatrixMap.h"

const int NORTH[2] = { 0,  1};
const int  WEST[2] = { 1,  0};
const int SOUTH[2] = { 0, -1};
const int  EAST[2] = {-1,  0};
const int *snake_direction = NORTH;

bool GameOver;

// Snack
class Snack{
 public:
  int8_t x;
  int8_t y;
  int16_t pos;
  
  CRGB color;
  Snack(){
  }
  Snack(int _x, int _y, CRGB _color){
    x = _x;
    y = _y;
    pos = XY(x, y);
    
    color = _color;
  }
  void draw(){
    leds[pos] = color;
  }
};

const int MAX_SNACKS = 100;
class Snacks{
 public:
  Snack snacks[MAX_SNACKS];
  int length;

  Snacks(){
    length = 0;
  }
  void New(){
    int x, y;
    int pos;
    bool done = false;

    while(!done){
      x = random(0, MatrixWidth - 1);
      y = random(0, MatrixHeight - 1);
      pos = XY(x, y);

      if(leds[pos].red == 0 &&
	 leds[pos].green == 0 &&
	 leds[pos].blue == 0){
	snacks[length] = Snack(x, y, CRGB::Green);
	Serial.print("Snack(");
	Serial.print(x);
	Serial.print(", ");
	Serial.print(y);
	Serial.println(")");
	done = true;
      }
    }
    length += 1;
  }
  void eat(int idx){
    for(int ii=idx; ii<length - 1; ii++){
      snacks[ii] = snacks[ii + 1];
    }
    length--;
  }
  void draw(){
    for(int ii=0; ii<length; ii++){
      snacks[ii].draw();
    }
  }
  Snack get(int idx){
    if(idx < length){
      return snacks[idx];
    }
  }
};
Snacks snacks;

// End Snack

// Snake Definition
class Segment{
 public:
  int8_t x;
  int8_t y;
  CRGB color;

  Segment(){
  }
  Segment(uint8_t x0, uint8_t y0, CRGB _color){
    x = x0;
    y = y0;
    color = _color;
  }
  void moveto(int8_t new_x, int8_t new_y, CRGB new_color){
    x = new_x;
    y = new_y;
    color = new_color;
  }
  void draw(){
    setPixel(x, y, color);
  }
};

const int MAX_SEGMENTS = 100;

class Snake{
 public:
  Segment segments[MAX_SEGMENTS];
  uint8_t length;
  void (*interact_callback)();
  
  Snake(){
    segments[0] = Segment(8, 5, CRGB::Blue); // tail
    segments[1] = Segment(8, 4, CRGB::Red ); // head
    reset();
  }
  void draw(){
    for(int i=0; i<length; i++){
      segments[i].draw();
    }
  }
  void reset(){
    Serial.println("reset()");
    segments[0].moveto(8, 5, CRGB::Blue); // tail
    segments[1].moveto(8, 4, CRGB::Red ); // head
    length = 2;
    snake_direction = SOUTH;
    GameOver = false;
  }
  void gameover(){
    GameOver = true;
    Serial.println("Game Over!");
    for(int ii=length-1; ii >= 0; ii--){
      setPixel(segments[ii].x, segments[ii].y, CRGB::Red);
      for(int jj=ii-1; jj >= 0; jj--){
	setPixel(segments[jj].x, segments[jj].y, CRGB::Blue);
      }
      FastLED.show();
      delay(100);
    }
    while(GameOver){
      (*interact_callback)();
      delay(100);
    }
    reset();
  }
  void move(){
    int new_y = segments[length-1].y + snake_direction[1];
    int new_x = segments[length-1].x + snake_direction[0];
    int pos = XY(new_x, new_y);
    int ii;
    boolean ate = false;

    if(new_x < 0 || new_x >= MatrixWidth){
      Serial.println("Out of bounds x!");
      gameover();
    }
    else if(new_y < 0 || new_y >= MatrixHeight){
      Serial.println("Out of bounds y!");
      gameover();
    }
    else{
      // check if position already occupied
      for(ii=0; ii<length; ii++){
	if(segments[ii].x == new_x && segments[ii].y == new_y){
	  Serial.println("Hit tail!");
	  gameover();
	  return;
	}
      }
      for(ii=0; ii<snacks.length; ii++){
	if(pos == snacks.get(ii).pos){
	  snacks.eat(pos);
	  ate = true;
	}
      }
      if(ate){ // extend snake
	segments[length-1].color = CRGB::Blue;
	if(length < MAX_SEGMENTS){
	  length++;
	  segments[length-1].moveto(new_x, new_y, CRGB::Red);
	}
	snacks.New();
      }
      else{
	for(ii=0; ii < length-1; ii++){
	  segments[ii].moveto(segments[ii+1].x, segments[ii+1].y, CRGB::Blue);
	}
	segments[length - 1].moveto(new_x, new_y, CRGB::Red);
      }
    }
  }
};
// End Snake
#endif
