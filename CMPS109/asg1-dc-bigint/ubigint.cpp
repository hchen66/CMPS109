// $Id: ubigint.cpp,v 1.8 2015-07-03 14:46:41-07 - - $
/*----------------------------------------------------------------------------------------
 *Class: CMPS109-wm.w16
 *
 *
 *
 *
 *
 *
 -----------------------------------------------------------------------------------------*/
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"

ubigint::ubigint (unsigned long that){                                                                   //ubig_value
   ubig_value.clear();                                                                                   //clear this->ubig_value
   string s = to_string(that);                                                                           //change long to string type
   for (char digit: s)  ubig_value.insert(ubig_value.begin(), digit);      
   while(ubig_value.size() > 1 && ubig_value.back() == '0') ubig_value.pop_back();                       //remove 0s
   DEBUGF ('~', this << " -> " << that)
}

ubigint::ubigint (const string& that){
   ubig_value.clear();                                
   for (char digit: that) ubig_value.insert(ubig_value.begin(), digit);                               
   while(ubig_value.size() > 1 && ubig_value.back() == '0') ubig_value.pop_back();                       //remove 0s
}

ubigint ubigint::operator+ (const ubigint& that) const {          
   ubigint res;
   int carry(0);                                                                                            
   auto iter1 = ubig_value.cbegin(), iter2 = that.ubig_value.cbegin();                                   //define iterator 

   while(iter1 != ubig_value.cend() || iter2 != that.ubig_value.cend() || carry) {                       //if there's any number left
      int sum = (iter1 != ubig_value.cend()      ? int(*iter1 - '0') : 0) +                              //if at left's end, left = 0, same as right
                (iter2 != that.ubig_value.cend() ? int(*iter2 - '0') : 0) + carry;
      carry = sum / 10;                                                                                  //carry = sum / 10; (e.g 18 -> 8, 8 -> 8)
      res.ubig_value.push_back((sum % 10) + '0');                                                        //1 + '0' = '1' (int to char)
      iter1 = (iter1 != ubig_value.cend())      ? (iter1 + 1) : iter1;                                   //recursion
      iter2 = (iter2 != that.ubig_value.cend()) ? (iter2 + 1) : iter2;                                   //recursion
   }
   return res;
}

ubigint ubigint::operator- (const ubigint& that) const {
   ubigint res;
   int borrow(0);
   auto iter1 = ubig_value.cbegin(), iter2 = that.ubig_value.cbegin();

   while(iter1 != ubig_value.cend() || iter2 != that.ubig_value.cend() || borrow) {                      //basically the algorithm is same as ubigint::operator+
      int result = (iter1 != ubig_value.cend()        ? int(*iter1 - '0') : 0) -
                   (iter2 != that.ubig_value.cend()   ? int(*iter2 - '0') : 0) + borrow;
      borrow = result < 0 ? -1 : 0;
      result = result < 0 ? (result + 10) : result;
      res.ubig_value.push_back(result + '0');
      iter1 = (iter1 != ubig_value.cend())      ? (iter1 + 1) : iter1;                                   //if iter1 reaches the end, iter1 = iter1, or iter1++
      iter2 = (iter2 != that.ubig_value.cend()) ? (iter2 + 1) : iter2;                                   //same as iter2
   }
   while(res.ubig_value.size() > 1 && res.ubig_value.back() == '0') {                                    //remove 0s
         res.ubig_value.pop_back();
   }
   return res;
   //if (*this < that) throw domain_error ("ubigint::operator-(a<b)");
   //return ubigint (uvalue - that.uvalue);
}

ubigint ubigint::operator* (const ubigint& that) const {
   ubigint res;
   vector<int> product;
      size_t m = ubig_value.size(), n = that.ubig_value.size();                                          //m = *this._u.size()  n = that._u.size()
      res.ubig_value.resize(m + n, '0');                                                                 //resive result.size() to m+n, all '0'
      product.resize(m + n, 0);
   for(size_t i = 0; i < m; i++) {                                                                 
      for(size_t j = 0; j < n; j++) {
            product[i + j]       += (ubig_value[i] - '0') * (that.ubig_value[j] - '0');                  //                                  4
            product[i + j + 1]   += product[i + j] / 10;                                                 //                             *    6
            product[i + j]       %= 10;                                                                  //                      --------------
      }                                                                                                  //      add to product[i+j+1] <- 2  4->  product[i][j]
   }                                                                                                     //               
   for(size_t i = 0; i < product.size(); i++)   res.ubig_value[i] = product[i] + '0';
   while(res.ubig_value.size() > 1 && res.ubig_value.back() == '0')  res.ubig_value.pop_back();
      return res;
}

void ubigint::multiply_by_2() {
   ubigint u1(2);
   *this = *this * u1;
}


void ubigint::divide_by_2() {                                                                           //  first res<- 3  7 -> seconde res      cur =   which num in the vector          
   int res = 0, reminder = 0, cur = 0;                                                                  //           -----------                         being opinted to
   ubigint result;                                                                                      //         2|   7  5
   for(auto itor = this->ubig_value.rbegin(); itor < this->ubig_value.rend(); itor++) {                //              6
      cur = *itor - '0';                                                                                //           ------------
      res = (cur + reminder*10) / 2;                                                                    // reminder<-   1  5
      reminder = (cur + reminder*10) %2;                                                                //              1  4 
      result.ubig_value.insert(result.ubig_value.begin(), res + '0');                                   //           ------------
   }                                                                                                    //       reminder<-1
   while(result.ubig_value.size() > 1 && result.ubig_value.back() == '0')                               
      result.ubig_value.pop_back();                                                                     // remove 0s

   *this = result;
}
//quot_rem divide (const ubigint&) const;
//using quot_rem = pair<ubigint,ubigint>;



ubigint::quot_rem ubigint::divide (const ubigint& that) const {
   static const ubigint zero = 0;
   if (that == zero) throw domain_error ("ubigint::divide: by 0");
   ubigint power_of_2 = 1;
   ubigint divisor = that; // right operand, divisor
   ubigint quotient = 0;
   ubigint remainder = *this; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor < remainder || divisor == remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {quotient, remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return divide (that).first;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return divide (that).second;
}



bool ubigint::operator== (const ubigint& that) const {                                                //check if is_negative and uvalue are the same
   if(ubig_value.size() != that.ubig_value.size()) return false;
   for(size_t i = 0; i < ubig_value.size(); i++)   
      if(ubig_value[i] != that.ubig_value[i])   return false;
   return true;
   //return uvalue == that.uvalue;
}

bool ubigint::operator< (const ubigint& that) const {
   size_t s1 = ubig_value.size(), s2 = that.ubig_value.size();                                       //ubigint with longer value is bigger
   if(s1 != s2)   return s1 < s2 ? true:false;
   else {
      for(size_t i = s1; i > 0; i--) {                                                               //compare from the end
         if(ubig_value[i - 1] > that.ubig_value[i - 1]) return false;
         else if(ubig_value[i- 1] < that.ubig_value[i - 1])  return true;
         else  continue;
      }
   }
   return false;
}

ostream& operator<< (ostream& out, const ubigint& that) {
   string res = "";
   for(auto itor = that.ubig_value.rbegin(); itor < that.ubig_value.rend(); itor++)
      res.push_back(*itor);
   return out << "ubigint(" << res << ")";
}

