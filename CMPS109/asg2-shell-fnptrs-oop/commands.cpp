// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $
#include <typeinfo>

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
   {"cat"   , fn_cat   },
   {"cd"    , fn_cd    },
   {"echo"  , fn_echo  },
   {"exit"  , fn_exit  },
   {"ls"    , fn_ls    },
   {"lsr"   , fn_lsr   },
   {"make"  , fn_make  },
   {"mkdir" , fn_mkdir },
   {"prompt", fn_prompt},
   {"pwd"   , fn_pwd   },
   {"rm"    , fn_rm    },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   const auto result = cmd_hash.find (cmd);
   if (result == cmd_hash.end()) {
      throw command_error (cmd + ": no such function");
   }
   return result->second;
}

command_error::command_error (const string& what):
            runtime_error (what) {
}

int exit_status_message() {
   int exit_status = exit_status::get();
   cout << execname() << ": exit(" << exit_status << ")" << endl;
   return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string error = "";

   switch(words.size()) {
      case 1:
            error += "cat: Please specify a filename";
            break;
      case 2:
      {       
            string filename = words[1];
            auto this_dirent = state.getDirent();
            bool isExist = this_dirent.find(filename) != this_dirent.cend() ? true:false;
            if(isExist) {
               bool isDirectory = this_dirent[filename] -> getContent() -> isDirectory();
               if(isDirectory)   error += "cat: " + filename + ": No such file or directory";
               else {
                  wordvec data = this_dirent[filename] -> getContent() -> getData();
                  for(const string& text: data) cout << text << " ";
                  cout << endl;
               }
            }
            else  error += "cat: " + filename + ": does not exist";
      }
      break;

      default:
            error += "cat: filename error";
            break;
   }
   if(!error.empty())   throw command_error(error);
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string s = words[1] + "/";
   string error = "";
   inode_ptr newCwd = state.peek(s);

   switch(words.size()) {
      case 1: 
            state.setCwd(state.getRoot());
            break;
      case 2:
         {
            if(newCwd == nullptr) {
               error += "cd: " + words[1] + ": No such file or directory";
               DEBUGF('c', "Invalid directory name");
            }else {
               state.setCwd(newCwd);
               DEBUGF('c', "Entry into new cwd");
            } 
         }
         break;

      default:
         {
            error += "cd: ";
            auto itor = words.cbegin() + 1;
            while(itor != words.cend())   error += " " + *itor++;
            error += " : No such file or directory"; 
         }
         break;
   }
   if(!error.empty())   throw command_error(error);
}

void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   inode_ptr this_cwd = state.getCwd();
   string error = "";

   switch(words.size()) {
      case 1:
      {
            auto map = state.getCwd() -> getContent() -> getDirent();
            for(auto itor = map.cbegin(); itor != map.cend(); ++itor) {
            string name = itor -> first;
            inode_ptr node = itor -> second;
            if(node -> getContent() -> isDirectory() && name != "." && name != "..")   name += "/"; 
            cout  <<"\t"   << node -> get_inode_nr()  << "\t"
                  << node -> getContent() -> size()   << " "
                  << name                             << endl;
            }
      }
      break;

      case 2:
      {
            string dest = words[1];
            dest += "/";
            inode_ptr newAddress = state.peek(dest);
            if(newAddress == nullptr) {
               error += "ls: " + words[1] + ": No such file or directory";
               DEBUGF('c', "Invalid directory name");
            }else {
               state.setCwd(newAddress);
               fn_ls(state, wordvec(words.begin(), words.begin() + 1));
               state.setCwd(this_cwd);
            }
      }
      break;

      default:
      {
            error += "ls: ";
            auto itor = words.cbegin() + 1;
            while(itor != words.cend())   error += " " + *itor++;
            error += " : No such file or directory";
      }
      break;
   }
   if(!error.empty())   throw command_error(error);
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string error = "";
   wordvec text{};

   if(words.size() == 1)   error += "make: Please specify a filename";
   string filename = words[1];
   for(auto itor = words.cbegin() + 2; itor != words.cend(); ++itor) text.push_back(*itor);
   
   auto this_dirent = state.getDirent();
   bool isExist = this_dirent.find(filename) != this_dirent.cend() ? true:false;
   if(isExist) 
   {
      bool isDirectory = this_dirent[filename] -> getContent() -> isDirectory();
      if(isDirectory)   error += "make: " + filename + ": is a directory";
      else {
         inode_ptr newFile = state.getCwd() -> getContent() -> mkfile(filename);
         newFile -> getContent() -> writefile(text);
      }
   }
   else
   {
      inode_ptr newFile = state.getCwd() -> getContent() -> mkfile(filename);
      newFile -> getContent() -> writefile(text);
   }
   if(!error.empty())   throw command_error(error);
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string dirname("");
   string error = "";
   
   switch(words.size()) {
      case 1:
            error += "mkdri: Please a specify a directory name";
            break;
      case 2:
      {     
            for(auto itor = words.cbegin() + 1; itor != words.cend(); ++itor) dirname += *itor;
            auto this_dirent = state.getDirent();
            bool isExist = this_dirent.find(dirname) != this_dirent.cend() ? true:false;

            if(isExist) error += "mkdir: " + dirname + ": file or directory already exists";
            else  state.getCwd() -> getContent() -> mkdir(dirname, state.getCwd());
      }
      break;

      default:
            error += "mkdir: directory name error";
            break;
   }
   if(!error.empty())   throw command_error(error);
}

void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string result = "";
   for(auto itor = words.cbegin() + 1; itor != words.cend(); ++itor) result += *itor + " ";
   state.setPrompt(result);
}

void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   string result = "";
   inode_ptr this_cwd = state.getCwd(), this_root = state.getRoot();
   if(state.getCwd() == state.getRoot())  result += "/";
   while(state.getCwd() != state.getRoot()) {
      result = "/" + state.getCwdName() + result;
      state.setCwd(state.getDirent()[".."]);
   }
   state.setCwd(this_cwd);
   state.setRoot(this_root);
   cout << result << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
}

