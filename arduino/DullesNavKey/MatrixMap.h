#ifndef MATRIX_MAP_H
#define MATRIX_MAP_H

#define MatrixWidth 16
#define MatrixHeight 8
#define NUM_LEDS (MatrixWidth * MatrixHeight)
uint16_t MatrixMap[MatrixHeight][MatrixWidth] = {
  {7,8,23,24,39,40,55,56,71,72,87,88,103,104,119,120},
  {6,9,22,25,38,41,54,57,70,73,86,89,102,105,118,121},
  {5,10,21,26,37,42,53,58,69,74,85,90,101,106,117,122},
  {4,11,20,27,36,43,52,59,68,75,84,91,100,107,116,123},
  {3,12,19,28,35,44,51,60,67,76,83,92,99,108,115,124},
  {2,13,18,29,34,45,50,61,66,77,82,93,98,109,114,125},
  {1,14,17,30,33,46,49,62,65,78,81,94,97,110,113,126},
  {0,15,16,31,32,47,48,63,64,79,80,95,96,111,112,127}
};

uint16_t XY(int x, int y){
  uint16_t out;
  x = MatrixWidth - x - 1;
  y = MatrixHeight - y - 1;
  if(0 <= x && x < MatrixWidth &&
     0 <= y && y < MatrixHeight){
    out = MatrixMap[y][x];
  }
  else{
    out = 0;
  }
  return out;
}

#endif
