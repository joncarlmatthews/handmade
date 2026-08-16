/* ========================================================================
   $File: C:\work\handmade\misc\bcircle.cpp $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

#include <stdio.h>

#define WIDTH 40
#define HEIGHT 38

int main(int ArgCount, char **Args)
{
    char Pixels[HEIGHT][WIDTH] = {};
    
    short Cx = 13;
    short Cy = 15;
    short R = 12;
    
    short X = -R;
    short Y = 0;
    short Ep = 0;
    
    char *Cursor = &Pixels[Cy][Cx];
    while(-X > Y)
    {
        // TODO(casey): This cursor addressing seems way too expensive...
        // need to work out how a 286 would ever do this sort of thing
        // for 8-way symmetry...
        Cursor[ Y*WIDTH+X] = 1;
        Cursor[ Y*WIDTH-X] = 1;
        Cursor[-Y*WIDTH+X] = 1;
        Cursor[-Y*WIDTH-X] = 1;
        Cursor[ X*WIDTH+Y] = 1;
        Cursor[ X*WIDTH-Y] = 1;
        Cursor[-X*WIDTH+Y] = 1;
        Cursor[-X*WIDTH-Y] = 1;
        
        Ep += 1;
        short E0 = Ep + (X << 1);
        short E1 = Ep + (Y << 1);
        if(-E0 < E1)
        {
            Ep = E0;
            ++X;
        }
        else
        {
            Ep = E1;
            ++Y;
        }
    }
    
    for(int Y = 0;
        Y < HEIGHT;
        ++Y)
    {
        for(int X = 0;
            X < WIDTH;
            ++X)
        {
            if(Pixels[Y][X])
            {
                printf("*");
            }
            else
            {
                printf(".");
            }
        }
        printf("\n");
    }
}