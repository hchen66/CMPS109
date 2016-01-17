// $Id: scanner.cpp,v 1.8 2015-07-02 16:48:18-07 - - $

#include <iostream>
#include <locale>
using namespace std;

#include "scanner.h"
#include "debug.h"

scanner::scanner() {
   seen_eof = false;                      //end of file = false;
   advance();
}

void scanner::advance() {
   if (not seen_eof) {
      cin.get (lookahead);                //getline 
      if (cin.eof()) seen_eof = true;     //end of file = true;
   }
}

token_t scanner::scan() {
   token_t result;
   while (not seen_eof and isspace (lookahead)) advance();
   if (seen_eof) {
      result.symbol = tsymbol::SCANEOF;
   }else if (lookahead == '_' or isdigit (lookahead)) {        //the first is '_' or digit
      result.symbol = tsymbol::NUMBER;
      do {
         result.lexinfo += lookahead;
         advance();
      }while (not seen_eof and isdigit (lookahead));
   }else {                                                     //operator
      result.symbol = tsymbol::OPERATOR;
      result.lexinfo += lookahead;
      advance();
   }
   DEBUGF ('S', result);                                     
   return result;
}

ostream& operator<< (ostream& out, const tsymbol& symbol) {
   switch (symbol) {
      case tsymbol::NUMBER  : out << "NUMBER"  ; break;
      case tsymbol::OPERATOR: out << "OPERATOR"; break;
      case tsymbol::SCANEOF : out << "SCANEOF" ; break;
   }
   return out;
}

ostream& operator<< (ostream& out, const token_t& token) {
   out << token.symbol << ": \"" << token.lexinfo << "\"";
   return out;
}

