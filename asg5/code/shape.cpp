// $Id: shape.cpp,v 1.2 2019-02-28 15:24:20-08 - - $

#include <typeinfo>
#include <unordered_map>
using namespace std;

#include "shape.h"
#include "util.h"
#include "math.h"
#include "graphics.h"

static unordered_map<void*,string> fontname {
   {GLUT_BITMAP_8_BY_13       , "Fixed-8x13"    },
   {GLUT_BITMAP_9_BY_15       , "Fixed-9x15"    },
   {GLUT_BITMAP_HELVETICA_10  , "Helvetica-10"  },
   {GLUT_BITMAP_HELVETICA_12  , "Helvetica-12"  },
   {GLUT_BITMAP_HELVETICA_18  , "Helvetica-18"  },
   {GLUT_BITMAP_TIMES_ROMAN_10, "Times-Roman-10"},
   {GLUT_BITMAP_TIMES_ROMAN_24, "Times-Roman-24"},
};

static unordered_map<string,void*> fontcode {
   {"Fixed-8x13"    , GLUT_BITMAP_8_BY_13       },
   {"Fixed-9x15"    , GLUT_BITMAP_9_BY_15       },
   {"Helvetica-10"  , GLUT_BITMAP_HELVETICA_10  },
   {"Helvetica-12"  , GLUT_BITMAP_HELVETICA_12  },
   {"Helvetica-18"  , GLUT_BITMAP_HELVETICA_18  },
   {"Times-Roman-10", GLUT_BITMAP_TIMES_ROMAN_10},
   {"Times-Roman-24", GLUT_BITMAP_TIMES_ROMAN_24},
};

ostream& operator<< (ostream& out, const vertex& where) {
   out << "(" << where.xpos << "," << where.ypos << ")";
   return out;
}

shape::shape() {
   DEBUGF ('c', this);
}

text::text (void* glut_bitmap_font_, const string& textdata_):
      glut_bitmap_font(glut_bitmap_font_), textdata(textdata_) {
   DEBUGF ('c', this);
}

ellipse::ellipse (GLfloat width, GLfloat height):
dimension ({width, height}) {
   DEBUGF ('c', this);
}

circle::circle (GLfloat diameter): ellipse (diameter, diameter) {
   DEBUGF ('c', this);
}


polygon::polygon (const vertex_list& vertices_): vertices(vertices_) {
   DEBUGF ('c', this);
}

rectangle::rectangle (GLfloat width, GLfloat height):
   polygon({ 
      {-width/2, height/2},   //top left
      {width/2, height/2},    //top right
      {width/2, -height/2},   //bot right
      {-width/2, -height/2}   //bot left
   }) {
   DEBUGF ('c', this << "(" << width << "," << height << ")");
}

square::square (GLfloat width): rectangle (width, width) {
   DEBUGF ('c', this);
}

diamond::diamond (const GLfloat width, const GLfloat height):
   polygon({
      {-width/2, 0},    //left
      {0, height/2},    //top
      {width/2, 0},     //right
      {0, -height/2}    //bot
   }){
   DEBUGF ('c', this);
}

triangle::triangle (const vertex_list& vertices_):
   polygon(vertices_){
   DEBUGF ('c', this);
}

equilateral::equilateral (GLfloat width):
   triangle({
      {-width/2, -width/4*sqrtf(3)},   //bot left
      {0, width/4*sqrtf(3)},           //top
      {width/2, -width/4*sqrtf(3)}     //bot right
   }){
   DEBUGF ('c', this);
}

void text::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");
      auto text_ = reinterpret_cast<const GLubyte*>(textdata.c_str());

      glColor3ubv (color.ubvec);
      glRasterPos2f (center.xpos, center.ypos);
      glutBitmapString (glut_bitmap_font, text_);
      glutSwapBuffers();
}

void text::draw_border (const vertex& center, 
   const rgbcolor& color) const
{
   glColor3ubv(color.ubvec);
   glLineWidth(window::border_width);
   glBegin(GL_LINE_LOOP);

   auto text_ = reinterpret_cast<const GLubyte*>(textdata.c_str());
   size_t width = glutBitmapLength(glut_bitmap_font, text_);
   size_t height = glutBitmapHeight(glut_bitmap_font);

   glVertex2f(center.xpos, center.ypos); //bot left
   glVertex2f(center.xpos, center.ypos + height); //bot left
   glVertex2f(center.xpos + width, center.ypos + height); //top right
   glVertex2f(center.xpos + width, center.ypos); // bot right

   glEnd();
}

void ellipse::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glBegin(GL_POLYGON);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv (color.ubvec);

   const float delta = 2 * M_PI / 32;
   for (float theta = 0; theta < 2 * M_PI; theta += delta) {
      float xpos = dimension.xpos * cos (theta) + center.xpos / 2;
      float ypos = dimension.ypos * sin (theta) + center.ypos / 2;
      glVertex2f (xpos, ypos);
   }
   glEnd();
}

void ellipse::draw_border (const vertex& center, 
   const rgbcolor& color) const
{
   glLineWidth(window::border_width);
   glBegin(GL_LINE_LOOP);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);

   const float delta = 2 * M_PI / 32;
   for (float theta = 0; theta < 2 * M_PI; theta += delta) {
      float xpos = dimension.xpos * cos (theta) + center.xpos / 2;
      float ypos = dimension.ypos * sin (theta) + center.ypos / 2;
      glVertex2f (xpos, ypos);
   }
   
   glEnd();
}

void polygon::draw (const vertex& center, const rgbcolor& color) const {
   DEBUGF ('d', this << "(" << center << "," << color << ")");

   glBegin(GL_POLYGON);
   glColor3ubv(color.ubvec);

   for(unsigned int ver = 0; ver < vertices.size(); ++ver)
   {
      glVertex2f(vertices[ver].xpos + center.xpos, 
         vertices[ver].ypos + center.ypos);
   }

   glEnd();
}

void polygon::draw_border (const vertex& center, 
   const rgbcolor& color) const
{
   glLineWidth(window::border_width);
   glBegin(GL_LINE_LOOP);
   glEnable(GL_LINE_SMOOTH);
   glColor3ubv(color.ubvec);

   for(unsigned int ver = 0; ver < vertices.size(); ++ver)
   {
      glVertex2f(vertices[ver].xpos + center.xpos, 
         vertices[ver].ypos + center.ypos);
   }

   glEnd();
}

void shape::show (ostream& out) const {
   out << this << "->" << demangle (*this) << ": ";
}

void text::show (ostream& out) const {
   shape::show (out);
   out << glut_bitmap_font << "(" << fontname[glut_bitmap_font]
       << ") \"" << textdata << "\"";
}

void ellipse::show (ostream& out) const {
   shape::show (out);
   out << "{" << dimension << "}";
}

void polygon::show (ostream& out) const {
   shape::show (out);
   out << "{" << vertices << "}";
}

ostream& operator<< (ostream& out, const shape& obj) {
   obj.show (out);
   return out;
}

