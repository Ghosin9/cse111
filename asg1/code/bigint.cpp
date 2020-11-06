// $Id: bigint.cpp,v 1.2 2020-01-06 13:39:55-08 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bigint::bigint (long that): uvalue (that), is_negative (that < 0) {
   DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
  bigint result;

  //if same sign you add
  if(is_negative == that.is_negative)
  {
    result.uvalue = uvalue + that.uvalue;
    result.is_negative = is_negative;
  }
  else //not same sign you sub
  {
    //case 1 if this > that
    if (uvalue > that.uvalue)
    {
      result.uvalue = uvalue - that.uvalue;
      result.is_negative = is_negative;
    }
    else if(uvalue < that.uvalue) //case 2 if this < that
    {
      result.uvalue = that.uvalue - uvalue;
      result.is_negative = that.is_negative;
    }
    else //case 3 if they are equal
    {
      result.uvalue = uvalue - that.uvalue;
      result.is_negative = false;
    }
  }

   return result;
}

bigint bigint::operator- (const bigint& that) const {
   bigint result;

   //if same sign you sub
   if(is_negative == that.is_negative)
   {
      //case one if this > that
      if(uvalue > that.uvalue)
      {
        result.uvalue = uvalue - that.uvalue;
        result.is_negative = is_negative;
      }
      else if (uvalue < that.uvalue) // case 2 if this < that
      {
        result.uvalue = that.uvalue - uvalue;
        result.is_negative = !(that.is_negative);
      }
      else //case 3 if they are equal
      {
        result.uvalue = uvalue - that.uvalue;
        result.is_negative = false;
      }
   }
   else //if different signs then you add
   {
    result.uvalue = uvalue + that.uvalue;
    result.is_negative = is_negative;
   }

   return result;
}


bigint bigint::operator* (const bigint& that) const {
   bigint result;
   static const ubigint ZERO (0);

   result.uvalue = uvalue * that.uvalue;

   if(is_negative == that.is_negative)
    result.is_negative = false;
   else
    result.is_negative = true;

  if(result.uvalue==ZERO)
    result.is_negative = false;

   return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result;
   static const ubigint ZERO (0);

   result.uvalue = uvalue / that.uvalue;

   if(is_negative==that.is_negative)
    result.is_negative = false;
   else
    result.is_negative = true;

  if(result.uvalue==ZERO)
    result.is_negative = false;

   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result;
   static const ubigint ZERO (0);

   result.uvalue = uvalue%that.uvalue;

   result.is_negative = is_negative;

   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << (that.is_negative ? "-" : "") << that.uvalue;
}

