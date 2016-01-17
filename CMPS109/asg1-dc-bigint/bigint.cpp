// $Id: bigint.cpp,v 1.73 2015-07-03 14:46:41-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"

bool bigint::isZero(const bigint& that) const {
  return that.uvalue == 0;
}

bigint::bigint (long that) {
  is_negative = that < 0 ? true:false;                    
  that = is_negative ? that*(-1) : that;                                                  //if negative, change that to positive
  uvalue = that;
  DEBUGF ('~', this << " -> " << uvalue)
}

bigint::bigint (const ubigint& uvalue, bool is_negative):
                uvalue(uvalue), is_negative(is_negative) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

//+ doesn't change is_negative
bigint bigint::operator+() const {  
   return *this;
}
//change the is_negative
bigint bigint::operator-() const {
   return {uvalue, !is_negative};
}


bigint bigint::operator+ (const bigint& that) const {
  bigint res;
  if(!is_negative && !that.is_negative) res.uvalue = (uvalue + that.uvalue);              //if both positive, directly use ubigint::operator+ to calculate
  else if(is_negative && that.is_negative) {                                              //if both negative, same as case1 except for is_negative = true, 
    res.uvalue = uvalue + that.uvalue;
    res.is_negative = true;
  }
  else {                                                                                  //if one pos, one neg.  uvalue = abs(big) - abs(small)
    res.uvalue = uvalue < that.uvalue ? (that.uvalue - uvalue) : (uvalue - that.uvalue);  
    res.is_negative = is_negative ? (uvalue < that.uvalue ? false:true) :
                    (uvalue < that.uvalue ? true:false);
  }
  if(isZero(res))  res.is_negative = false;                                               //make sure zero.is_negative = false
  return res;
}

bigint bigint::operator- (const bigint& that) const {
  bigint res;
  if(!is_negative && !that.is_negative)   return *this + (-that);                         //2 - 3 = 2 + (_3)
  else if(is_negative && that.is_negative)  return  (-that + *this);                      //_2 - (_3) = 3 + (_2)
  else  return is_negative ? -(-(*this) + that) : *this + (-that);                        //if left is negative: _2 - 3 = -(2+3);   if left is pos: 2 - (_3) = 2 + 3
  
  if(isZero(res))  res.is_negative = false;                                               //zero.is_negative = false
  return res;
}

bigint bigint::operator* (const bigint& that) const {
  bigint res;
  res.uvalue = uvalue * that.uvalue;
  res.is_negative = (is_negative == that.is_negative) ? false:true;                       //res.is_negative = left and right has the same is_negative
  return res;
}

bigint bigint::operator/ (const bigint& that) const {
  bigint res;
  res.uvalue = uvalue / that.uvalue;
  res.is_negative = (is_negative == that.is_negative) ? false:true;                       //res.is_negative = left and right has the same is_negative
  return res;
}

bigint bigint::operator% (const bigint& that) const {
  bigint res;
  res.uvalue = uvalue % that.uvalue;
  res.is_negative = (is_negative == that.is_negative) ? false:true;                       //res.is_negative = left and right has the same is_negative
  return res;
}


bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;                      //check is_negative and uvalue
}

bool bigint::operator< (const bigint& that) const {
  if(is_negative && that.is_negative) return uvalue < that.uvalue ? false:true;           //if both negative
  else if(!is_negative  && !that.is_negative) return uvalue < that.uvalue ? true:false;   //if both positive
  else return is_negative ? true:false;                                                   //pos > neg
}

ostream& operator<< (ostream& out, const bigint& that) {
   return out << "bigint(" << (that.is_negative ? "'-'" : "'+'")
              << that.uvalue << ")";
}

