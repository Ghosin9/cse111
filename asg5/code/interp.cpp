// $Id: interp.cpp,v 1.3 2019-03-19 16:18:22-07 - - $

#include <memory>
#include <string>
#include <vector>
using namespace std;

#include <GL/freeglut.h>

#include "debug.h"
#include "interp.h"
#include "shape.h"
#include "util.h"
#include <string.h>

unordered_map<string,interpreter::interpreterfn>
interpreter::interp_map {
   {"define" , &interpreter::do_define },
   {"draw"   , &interpreter::do_draw   },
   {"moveby" , &interpreter::do_moveby },
   {"border" , &interpreter::do_border },
};

unordered_map<string,interpreter::factoryfn>
interpreter::factory_map {
   {"text"     , &interpreter::make_text     },
   {"ellipse"  , &interpreter::make_ellipse  },
   {"circle"   , &interpreter::make_circle   },
   {"polygon"  , &interpreter::make_polygon  },
   {"rectangle", &interpreter::make_rectangle},
   {"square"   , &interpreter::make_square   },
   {"triangle" , &interpreter::make_triangle },
   {"diamond"  , &interpreter::make_diamond  },
   {"equilateral", &interpreter::make_equilateral},
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

interpreter::shape_map interpreter::objmap;
static rgbcolor border_color;

interpreter::~interpreter() {
   for (const auto& itor: objmap) {
      cout << "objmap[" << itor.first << "] = "
           << *itor.second << endl;
   }
}

void interpreter::interpret (const parameters& params) {
   DEBUGF ('i', params);
   param begin = params.cbegin();
   string command = *begin;
   auto itor = interp_map.find (command);
   if (itor == interp_map.end()) throw runtime_error ("syntax error");
   interpreterfn func = itor->second;
   func (++begin, params.cend());
}

void interpreter::do_define (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string name = *begin;
   objmap.emplace (name, make_shape (++begin, end));
}


void interpreter::do_draw (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   if (end - begin != 4) throw runtime_error ("syntax error");
   string name = begin[1];
   shape_map::const_iterator itor = objmap.find (name);
   if (itor == objmap.end()) {
      throw runtime_error (name + ": no such shape");
   }
   rgbcolor color {begin[0]};
   vertex where {from_string<GLfloat> (begin[2]),
                 from_string<GLfloat> (begin[3])};
   window::push_back (object (itor->second, where, color));
}

void interpreter::do_moveby (param begin, param end)
{
   DEBUGF ('f', range (begin, end));
   ++begin;
   window::move_by_pixels = std::stof((*begin).c_str(), 0);
}

void interpreter::do_border (param begin, param end)
{
   DEBUGF ('f', range (begin, end));
   rgbcolor bColor {begin[1]};

   window::border_color = bColor;

   window::border_width = std::stof((begin[2]).c_str(), 0);
}

shape_ptr interpreter::make_shape (param begin, param end) {
   DEBUGF ('f', range (begin, end));
   string type = *begin++;
   auto itor = factory_map.find(type);
   if (itor == factory_map.end()) {
      throw runtime_error (type + ": no such shape");
   }
   factoryfn func = itor->second;
   return func (begin, end);
}

shape_ptr interpreter::make_text (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   auto fontC = fontcode.find(*begin++);
   if (fontC == fontcode.end()) 
      throw runtime_error("No such font in this program");

   void* font = fontC->second;

   string text_line;
   for(; begin != end; ++begin)
   {
      text_line += *begin;
      text_line += " ";
   }

   return make_shared<text> (font, text_line);
}

shape_ptr interpreter::make_ellipse (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat width = std::stof((*begin).c_str(), 0);
   ++begin;
   GLfloat height = std::stof((*begin).c_str(), 0);

   return make_shared<ellipse> (width, height);
}

shape_ptr interpreter::make_circle (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat radius = std::stof((*begin).c_str(), 0);

   if (++begin != end)
      throw runtime_error 
         ("Define failed: incorrect input for making a circle");

   return make_shared<circle> (radius);
}

shape_ptr interpreter::make_polygon (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   vertex_list list;

   GLfloat xCord;
   GLfloat yCord;

   for(; begin != end; ++begin)
   {
      xCord = std::stof((*begin).c_str(), 0);
      ++begin;
      yCord = std::stof((*begin).c_str(), 0);
      list.push_back({xCord, yCord});
   }
   return make_shared<polygon> (list);
}

shape_ptr interpreter::make_rectangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat width = std::stof((*begin).c_str(), 0);
   ++begin;
   GLfloat height = std::stof((*begin).c_str(), 0);

   return make_shared<rectangle> (width, height);
}

shape_ptr interpreter::make_square (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat width = std::stof((*begin).c_str(), 0);

   if (++begin != end)
      throw runtime_error 
         ("Define failed: incorrect input for making a square");

   return make_shared<square> (width);
}

shape_ptr interpreter::make_triangle (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   vertex_list vertices;

   for(; begin != end; ++begin)
   {
      GLfloat x = std::stof((*begin).c_str(), 0);
      ++begin;
      GLfloat y = std::stof((*begin).c_str(), 0);

      vertices.push_back({x, y});
   }

   return make_shared<triangle> (vertices);
}

shape_ptr interpreter::make_equilateral (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat width = std::stof((*begin).c_str(), 0);

   return make_shared<equilateral> (width);
}

shape_ptr interpreter::make_diamond (param begin, param end) {
   DEBUGF ('f', range (begin, end));

   GLfloat width = std::stof((*begin).c_str(), 0);
   ++begin;
   GLfloat height = std::stof((*begin).c_str(), 0);

   return make_shared<diamond> (width, height);
}
