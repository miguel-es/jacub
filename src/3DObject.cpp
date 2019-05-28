#include "3DObject.h"

3DObject(char* name, int x,int y, int z, int r, int g, int b, int osize,bool fixed)
{
    this.name = name;
    int originx = x;
    int originy = y;
    int originz = z;
    int actualx = x;
    int actualy = y;
    int actualz = z;
    int r = r;
    int g = g;
    int b = b;
    float osize ;
    bool fixed = fixed;
    bool moving = false;
}

bool isMoving(){
    return originx==actualx && originy==actualy && originx==actualz;
}



~3DObject()
{
    //dtor
}
