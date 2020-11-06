// $Id: ubigint.cpp,v 1.8 2020-01-06 13:39:55-08 - - $

#include <cctype>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
#include <vector>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that){
   // DEBUGF ('~', this << " -> " << uvalue)
   while(that >0)
   {
      int temp = that %10;
      ubig_value.push_back(temp - '0');
      that = that/10;
   }

   //cout << "yeet" << endl;
}

ubigint::ubigint (const string& that){
   // DEBUGF ('~', "that = \"" << that << "\"");
   for (char digit: that) {
      if (not isdigit (digit)) {
         throw invalid_argument ("ubigint::ubigint(" + that + ")");
      }
   }
   //cout << "hello" << endl;

   for(auto itor = that.crbegin(); itor != that.crend(); ++itor)
   {
      ubig_value.push_back(*itor);
   }

   //print value
   // for(auto itor = that.crbegin(); itor != that.crend(); ++itor)
   // {
   //    cout << *itor;
   // }
   //cout << endl;
}

ubigint ubigint::operator+ (const ubigint& that) const {
   int carry = 0;
   ubigint result;
   int shorter = 0;
   int longer = 0;
   int whichNum;

   //find the length of the shorter one
   //and loop through for that many iterations
   if(ubig_value.size() < that.ubig_value.size())
   {
      shorter = ubig_value.size();
      longer = that.ubig_value.size();
      whichNum = 1;
   }
   else //ubig_value.size() > that.size()
   {
      shorter = that.ubig_value.size();
      longer = ubig_value.size();
      whichNum = 0;
   }

   //itors for both big ints
   int yes;
   //for loop that goes for the shorter number
   for(yes= 0; yes < shorter; ++yes)
   {
      int temp = 0;
      int num1 = ubig_value[yes] - 48; //convert from char to int
      int num2 = that.ubig_value[yes] - 48; //convert from char to int
      //temporary number
      temp += carry + num1 + num2;
      
      //store the remainder as a character
      int store = (temp%10);
      result.ubig_value.push_back(store + '0');
      //calculate the carry
      carry = temp/10;
   }

   //done with shorter one
   //now loop through the longer and just copy its elements
   for(int no=yes; no < longer; ++no)
   {
      int num1;
      int store;
      if(whichNum == 0) //this was longer than that
      {
         num1 = carry + (ubig_value[no] - 48);
         carry = num1/10;

         store = num1%10;
         result.ubig_value.push_back(store + '0');
      }
      else //that was longer than this
      {
         num1 = carry + (that.ubig_value[no] - 48);
         carry = num1/10;

         store = num1%10;
         result.ubig_value.push_back(store + '0');
      }
   }

   //after the loop if there still is carry left, add it
   if(carry>0)
   {
      result.ubig_value.push_back(carry + '0');
   }

   //remove leading zeros
   while (result.ubig_value.size() > 0 and 
      result.ubig_value.back() == '0') 
      result.ubig_value.pop_back();

   return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
   if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   //this is always bigger than that because of bigint
   ubigint result;
   int shorter = that.ubig_value.size();
   int longer = ubig_value.size();
   int itor;
   bool sub = false;
   for(itor = 0; itor < shorter; ++itor)
   {
      int temp = 0;
      int num1 = ubig_value[itor] - 48;
      int num2 = that.ubig_value[itor] - 48;

      //cout << "subtracting: " << num1 << " - " << num2 << endl;

      if(sub == true)
         num1--;

      if(num1 > num2) //case 1 if no need to carry over a 1
      {
         temp = num1-num2;
         sub = false;
      }
      else if (num1 < num2) //case 2 if need carry over
      {
         num1 += 10;

         //subtract one from the next number
         sub = true;
         temp = num1 - num2;
      }
      else //case 3 they are equal
      {
         temp = num1- num2;
         sub = false;
      }

      result.ubig_value.push_back(temp + '0');
   }

   //adding the rest of the longer element
   for(; itor< longer; ++itor)
   {
      int num1 = ubig_value[itor] - 48;
      result.ubig_value.push_back(num1 + '0');
   }

   //remove leading zeros
   while (result.ubig_value.size() > 0 and 
      result.ubig_value.back() == '0') 
      result.ubig_value.pop_back();

   return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint result;
   int len1 = ubig_value.size();
   int len2 = that.ubig_value.size();
   int carry = 0;
   int d = 0;

   for(int itor = 0; itor < len1+len2; ++itor)
   {
      result.ubig_value.push_back('0');
   }

   for(int thisItor = 0; thisItor < len1; ++thisItor)
   {
      int num1 = ubig_value[thisItor] - 48;
      carry = 0;

      for(int thatItor = 0; thatItor < len2; ++thatItor)
      {
         int num2 = that.ubig_value[thatItor] - 48;

         d = (result.ubig_value[thisItor + thatItor]-48) + (num1* num2) + carry;

         result.ubig_value[thisItor + thatItor] = ((d%10) + '0');

         carry = d/10;
      }

      result.ubig_value[thisItor + len2] = carry + '0';
   }

   //remove leading zeros
   while (result.ubig_value.size() > 0 and 
      result.ubig_value.back() == '0') 
      result.ubig_value.pop_back();

   return result;
}

void ubigint::multiply_by_2() {
   int len = ubig_value.size();
   int carry = 0;
   int d = 0;

   for(int itor = 0; itor < len; ++itor)
   {
      int num1 = ubig_value[itor] - 48;

      d = (num1* 2) + carry;

      carry = 0;

      ubig_value[itor] = ((d%10) + '0');

      carry = d/10;
   }
   ubig_value.push_back(carry + '0');

   //remove leading zeros
   while (ubig_value.size() > 0 and 
      ubig_value.back() == '0') 
      ubig_value.pop_back();
}

void ubigint::divide_by_2() {
   int len = ubig_value.size();
   int temp = 0;

   for(int itor =0; itor < len; ++itor)
   {
      temp = (ubig_value[itor] - 48)/2;

      if((itor+1) < len)
      {
         int even = ubig_value[itor+1] - 48;
         even = even%2;
         if(even==1)
         {
            temp+=5;
         }
      }

      ubig_value[itor] = temp + '0';
   }

   //remove leading zeros
   while (ubig_value.size() > 0 and 
      ubig_value.back() == '0') 
      ubig_value.pop_back();
}


struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, const ubigint& divisor_) {
   // NOTE: udivide is a non-member function.
   ubigint divisor {divisor_};
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
   int len1 = ubig_value.size();
   int len2 = that.ubig_value.size();

   if(len1 != len2)
      return false;
   else
   {
      for(int itor = 0; itor < len1; ++itor)
      {
         if(ubig_value[itor]!= that.ubig_value[itor])
            return false;
      }
   }

   return true;
}

bool ubigint::operator< (const ubigint& that) const {
   int len1 = ubig_value.size();
   int len2 = that.ubig_value.size();

   if(len1 < len2)
      return true;
   else if (len1 > len2)
      return false;
   else
   {
      for(int itor = 0; itor < len1; ++itor)
      {
         if(ubig_value[itor]< that.ubig_value[itor])
            return true;
         else if (ubig_value[itor]> that.ubig_value[itor])
            return false;
      }
   }

   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
   int counter = 0;
   for(auto itor = that.ubig_value.crbegin(); 
      itor != that.ubig_value.crend(); ++itor)
   {
      if(counter == 69)
      {
         out << "\\"<< endl;
         counter = 0;
      }
      out << (*itor);
       ++counter;
   }
   return out;
}

