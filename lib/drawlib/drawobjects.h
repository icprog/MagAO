#ifndef DRAWOBJECT_H_INCLUDED
#define DRAWOBJECT_H_INCLUDED

#include <time.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>

#include <string>
#include <fstream>
#include <sstream> 
using namespace std;

#include "qpainter.h"

typedef struct {
   float x;
   float y;
} point;

class drawobject {

   public:
      drawobject( time_t timeValid, int useCcdCoord=0) { _timeValid = timeValid; _timestamp = time(NULL); _CCDtoScreenF = NULL; _useCcdCoord = useCcdCoord; }
      drawobject() { _CCDtoScreenF = NULL; };
      virtual ~drawobject() {}

   public:
      virtual void import( string s) {
         int pos = s.find("TIMESTAMP");
         _timestamp = atoi( s.substr(pos+9).c_str());
         pos = s.find("TIMEVALID");
         _timeValid = atoi( s.substr(pos+9).c_str());
         pos = s.find("USECCDCOORD");
         _useCcdCoord = atoi( s.substr(pos+11).c_str());
      }
      virtual string export2() {
         ostringstream ostr;
         ostr << " TYPE " << _type << " TIMESTAMP" << _timestamp << " TIMEVALID" << _timeValid << " USECCDCOORD" << _useCcdCoord;
         return ostr.str();
      }

   public:
      void Draw( QPainter *p, float xzoom, float yzoom, void (*CCDtoScreenF)( float, float, float *, float *) ) {
         if ((!p) || (!CCDtoScreenF))
            return;

         if (!isValid())
            return;

         _xzoom = xzoom;
         _yzoom = yzoom;
         _CCDtoScreenF = CCDtoScreenF;
         DrawInternal(p);
      }

      point p( float x, float y) {
         point p1;
         if (_useCcdCoord)
            (*_CCDtoScreenF)( x, y, &p1.x, &p1.y);
         else {
            p1.x =x;
            p1.y =y;
         }

         p1.x *= _xzoom;
         p1.y *= _yzoom;
         return p1;
      }
      point p( point p_from) {
         return p( p_from.x, p_from.y);
      }

      bool isValid() {
         time_t now = time(NULL);
         return (_timestamp + _timeValid >= now);
      } 

   public:
      virtual void DrawInternal( QPainter *painter)=0;

   public:
      static drawobject *create(string s); 



   protected:
      float _xzoom;
      float _yzoom;
      void (*_CCDtoScreenF)( float, float, float *, float *);
      time_t _timestamp;
      time_t _timeValid;
      string _type;
      int _useCcdCoord;
};

// A simple line

class line : public drawobject {

   public:
      line( float x1, float y1, float x2, float y2, unsigned int color, int timeValid, int useCcdCoord=0) : drawobject( timeValid, useCcdCoord) {
         _x1=x1; _y1=y1; _x2=x2; _y2=y2; _color=color;
         _type = "LINE";
      }
      line() : drawobject() {
         _type = "LINE";
      }


   void import (string s) {
      drawobject::import(s);
      sscanf(s.c_str(), "%f %f %f %f %d", &_x1, &_y1, &_x2, &_y2, &_color);
   }
   string export2() {
      ostringstream ostr;
      ostr << _x1 << " " << _y1 << " " << _x2 << " " << _y2 << " " << _color << drawobject::export2();
      return ostr.str();
   }

   virtual void DrawInternal( QPainter *painter) {
      painter->setPen(_color);
      point p1 = p( _x1, _y1);
      point p2 = p( _x2, _y2);
      painter->drawLine( (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
   }

   protected:
      float _x1, _y1, _x2, _y2;
      unsigned int _color;
};

class arrow : public line {

   public:
      arrow( float x1, float y1, float x2, float y2, unsigned int color, int timeValid, int useCcdCoord=0) : line( x1, y1, x2, y2, color, timeValid, useCcdCoord) {
         _type = "ARRO";
      }
      arrow() : line() {
         _type = "ARRO";
      }

   virtual void DrawInternal( QPainter *painter) {
      painter->setPen(_color);
      point p1 = p( _x1, _y1);
      point p2 = p( _x2, _y2);
      painter->drawLine( (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);

      float dx = _x1-_x2;
      float dy = _y1-_y2;
      float dist = sqrt(dx*dx + dy*dy);
      float dir = atan2(dy, dx);
      float ang1 = dir + 3.1415/4;
      float ang2 = dir - 3.1415/4;
      p1 = p( _x2, _y2);

      p2 = p( _x2 + cos(ang1)*dist/3, _y2 + sin(ang1)*dist/3); 
      painter->drawLine( (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);

      p2 = p( _x2 + cos(ang2)*dist/3, _y2 + sin(ang2)*dist/3); 
      painter->drawLine( (int)p1.x, (int)p1.y, (int)p2.x, (int)p2.y);
   }
};

// A circle

class circle : public drawobject {
   public:
      circle( float cx, float cy, float diam, unsigned int color, int timeValid, int useCcdCoord=0) : drawobject( timeValid, useCcdCoord) {
         _cx = cx; _cy = cy; _diam = diam; _color = color; 
         _type = "CIRC";
      }
      circle() : drawobject() {
         _type = "CIRC";
      }

      void import( string s) {
         drawobject::import(s);
         sscanf(s.c_str(), "%f %f %f %d", &_cx, &_cy, &_diam, &_color);
      }
      string export2() {
         ostringstream ostr;
         ostr << _cx << " " << _cy << " " << _diam << " " << _color << drawobject::export2();
         return ostr.str();
      }

      void DrawInternal( QPainter *painter) {
         painter->setPen(_color);

         float x1 = _cx-_diam/2.0-0.5;
         float y1 = _cy-_diam/2.0+0.5;
         float x2 = _cx+_diam/2.0-0.5;
         float y2 = _cy+_diam/2.0+0.5;

         point p1 = p(x1, y1);
         point p2 = p(x2, y2);

         painter->drawEllipse( (int)p1.x, (int)p1.y, (int)(p2.x-p1.x), (int)(p2.y-p1.y));
      }

   protected:
      float _cx;
      float _cy;
      float _diam;
      unsigned int _color;
};


void sendObject( char *clientName, drawobject *object);
drawobject *getObject( char *clientName);


#endif // DRAWOBJECT_H_INCLUDED

