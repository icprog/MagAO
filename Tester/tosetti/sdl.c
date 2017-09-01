
#include <stdlib.h>
#include "SDL/SDL.h"

#define SPACING (20)
#define SIZE    (10)

void drawRect( SDL_Surface *screen, int x1, int y1, int x2, int y2, int color)
{
   int x,y;

   for (y=y1; y<y2; y++)
      for (x=x1; x<x2; x++)
         ((Uint8 *)screen->pixels)[ y*screen->pitch + x] = color;
}

void update( SDL_Surface *screen)
{
   int x,y;

   if ( SDL_MUSTLOCK(screen) )
          if ( SDL_LockSurface(screen) < 0 ) 
                 return;

   for (x=0; x<30; x++)
      for (y=0; y<30; y++)
            {
            int color = rand() % 256;
            drawRect( screen, x*SPACING, y*SPACING, x*SPACING + SIZE, y*SPACING + SIZE, color);
            }


   if ( SDL_MUSTLOCK(screen) ) 
           SDL_UnlockSurface(screen);

    SDL_UpdateRect(screen, 0, 0, 639, 479);
    
}

main(int argc, char *argv[])
{
       if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) {
                  fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
                          exit(1);
                              }
           atexit(SDL_Quit);

           SDL_Surface *screen;

           screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE);
           if ( screen == NULL ) {
                 fprintf(stderr, "Unable to set 640x480 video: %s\n", SDL_GetError());
                 exit(1);
                 }

   while(1)
   {
   update(screen);
   usleep(50*1000);
   } 
}



