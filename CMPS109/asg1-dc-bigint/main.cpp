// $Id: main.cpp,v 1.48 2015-07-02 16:48:18-07 - - $
#include <stack>
#include <cassert>
#include <deque>
#include <iostream>
#include <map>
#include <stdexcept>
#include <utility>
using namespace std;

#include <unistd.h>

#include "bigint.h"
#include "debug.h"
#include "general.h"
#include "iterstack.h"
#include "libfns.h"
#include "scanner.h"

using bigint_stack = iterstack<bigint>;                        //stack with bigint inside

void do_arith (bigint_stack& stack, const char oper) {
   if (stack.size() < 2) throw ydc_exn ("stack empty");        //empty stack case
   bigint right = stack.top();                                 //right number
   stack.pop();                                                //pop the right number
   DEBUGF ('d', "right = " << right);
   bigint left = stack.top();                                  //left number
   stack.pop();                                                //pop the left number
   DEBUGF ('d', "left = " << left);
   bigint result;                                              
   switch (oper) {                                             //do calculation
      case '+': result = left + right; break;
      case '-': result = left - right; break;
      case '*': result = left * right; break;
      case '/': result = left / right; break;
      case '%': result = left % right; break;
      case '^': result = pow (left, right); break;
      default: throw invalid_argument (
                     string ("do_arith operator is ") + oper);
   }
   DEBUGF ('d', "result = " << result);
   stack.push (result);                                        //push the result into the stack
}

void do_clear (bigint_stack& stack, const char) {              //char = c
   DEBUGF ('d', "");
   stack.clear();
}


void do_dup (bigint_stack& stack, const char) {                //char = d
   bigint top = stack.top();
   DEBUGF ('d', top);
   stack.push (top);
}

void do_printall (bigint_stack& stack, const char) {           //char = f
   //for (const auto &elem: stack) cout << elem << endl;
   for (auto itor = stack.begin(); itor < stack.end(); itor++)
      cout << *itor << endl;
}

void do_print (bigint_stack& stack, const char) {              //?????
   cout << stack.top() << endl;
}

void do_debug (bigint_stack& stack, const char) {
   (void) stack; // SUPPRESS: warning: unused parameter 'stack'
   cout << "Y not implemented" << endl;
}

class ydc_quit: public exception {};
void do_quit (bigint_stack&, const char) {
   throw ydc_quit();
}

using function_t = void (*)(bigint_stack&, const char);        //pointer to any data structor
using fn_map = map<string,function_t>;                         //map for selecting operations
fn_map do_functions = {
   {"+", do_arith},
   {"-", do_arith},
   {"*", do_arith},
   {"/", do_arith},
   {"%", do_arith},
   {"^", do_arith},
   {"Y", do_debug},
   {"c", do_clear},
   {"d", do_dup},
   {"f", do_printall},
   {"p", do_print},
   {"q", do_quit},
};


//
// scan_options
//    Options analysis:  The only option is -Dflags. 
//
void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            debugflags::setflags (optarg);
            break;
         default:
            error() << "-" << static_cast<char> (optopt)
                    << ": invalid option" << endl;
            break;
      }
   }
   if (optind < argc) {
      error() << "operand not permitted" << endl;
   }
}


//
// Main function.
//
int main (int argc, char** argv) {
   exec::execname (argv[0]);
   scan_options (argc, argv);
   bigint_stack operand_stack;
   scanner input;
   try {
      for (;;) {
         try {
            token_t token = input.scan();                      //get a char from cin
            if (token.symbol == tsymbol::SCANEOF) break;        
            switch (token.symbol) {                            //case for symbol 
               case tsymbol::NUMBER:
                  operand_stack.push (bigint (token.lexinfo)); //Number: push it into stack
                  break;
               case tsymbol::OPERATOR: {                       
                  fn_map::const_iterator fn
                           = do_functions.find (token.lexinfo);
                  if (fn == do_functions.end()) {
                     throw ydc_exn (octal (token.lexinfo[0])
                                    + " is unimplemented");
                  }
                  fn->second (operand_stack, token.lexinfo.at(0));
                  break;
                  }
               default:
                  assert (false);
            }
         }catch (ydc_exn& exn) {
            cout << exn.what() << endl;
         }
      }
   }catch (ydc_quit&) {
      // Intentionally left empty.
   }
   return exec::status();
}

