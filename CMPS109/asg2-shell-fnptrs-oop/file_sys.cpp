// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

struct file_type_hash {
   size_t operator() (file_type type) const {
      return static_cast<size_t> (type);
   }
};

ostream& operator<< (ostream& out, file_type type) {
   static unordered_map<file_type,string,file_type_hash> hash {
      {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
      {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
   };
   return out << hash[type];
}

inode_state::inode_state() {
   DEBUGF ('i', "root = " << root << ", cwd = " << cwd
          << ", prompt = \"" << prompt() << "\"");

   inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);
   cwd = newNode;
   root = newNode;
   newNode.reset();
   //cout << cwd.use_count() << endl;
}

const string& inode_state::prompt() { 
  return prompt_; 
}

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

void inode_state::modify_dirent(const inode_ptr& node, const string& filename) {
  //map<string, inode_ptr>& dirents = getDirent();
  //dirents[filename]  = node;
  getDirent()[filename] = node;
}

size_t inode_state::getSize(const inode_ptr&) const {
  return cwd -> getContent() -> size();
}

inode_ptr inode_state::getRoot() {
  return root;
}

inode_ptr inode_state::getCwd() {
  return cwd;
}

map<string, inode_ptr>& inode_state::getDirent() {
  base_file_ptr content = cwd -> getContent();
  return content -> getDirent();
}

void inode_state::setCwd(const inode_ptr& that) {
  cwd = that;
}

void inode_state::setRoot(const inode_ptr& that) {
  root = that;
}

//    /aaa/eee/ccc/ddd
inode_ptr inode_state::peek_helper(string& s, const inode_ptr& node) {
  if(s.empty() || s.find_first_not_of("/") == size_t(-1)) return node;
  string dirname = s.substr(0, s.find_first_of("/"));
  map<string, inode_ptr> m = node -> getDirent();
  auto loc = m.find(dirname);
  s = s.substr(s.find_first_of("/"), s.length());
  if(s[0] == '/') s.erase(s.begin());
  if(loc == m.end())  return nullptr;
  else  return peek_helper(s, loc->second);
}

inode_ptr inode_state::peek(string& that) {
  if(that == "/")  return root;
  if(that[0] == '/') {
    that.erase(that.begin());
    return peek_helper(that, root);
  }else return peek_helper(that, cwd); 
}

string inode_state::getCwdName() {
  string result = "";
  if(cwd == root) return "/";
  map<string, inode_ptr> m = getDirent()[".."] -> getDirent();
  inode_ptr parent = getCwd();
  for(auto itor = m.cbegin(); itor != m.cend(); ++itor) {
    if(itor -> second == parent)  return itor -> first;
  }
  return "";
}

void inode_state::setPrompt(const string& that) {
  prompt_ = that;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

base_file_ptr& inode::getContent() {
  return contents;
}

map<string, inode_ptr>& inode::getDirent() {
  return contents -> getDirent();
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

size_t plain_file::size() const {
  size_t size {0};
  DEBUGF ('i', "size = " << size);
  for(const string& s: data) size += s.length();
  size = data.empty() ? size : size + data.size() - 1;

  return size;
}

const wordvec& plain_file::readfile() const {
   DEBUGF ('i', data);
   return data;
}

void plain_file::writefile (const wordvec& words) {
   DEBUGF ('i', words);
   data.clear();
   data = words;
}

void plain_file::remove (const string&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkdir (const string&, const inode_ptr&) {
   throw file_error ("is a plain file");
}

inode_ptr plain_file::mkfile (const string&) {
   throw file_error ("is a plain file");
}

wordvec& plain_file::getData() {
  return data;
}

map<string, inode_ptr>& plain_file::getDirent() {
  throw file_error ("is a plain file");
}

bool plain_file::isDirectory() const {
  return false;
}



size_t directory::size() const {
   size_t size {dirents.size()};
   DEBUGF ('i', "size = " << size);
   return size;
}

const wordvec& directory::readfile() const {
   throw file_error ("is a directory");
}

void directory::writefile (const wordvec&) {
   throw file_error ("is a directory");
}

void directory::remove (const string& filename) {
   DEBUGF ('i', filename);
}

inode_ptr directory::mkdir (const string& dirname, const inode_ptr& node) {
  inode_ptr newNode = make_shared<inode>(file_type::DIRECTORY_TYPE);                
  dirents[dirname] = newNode;
  newNode -> getDirent()["."]  = newNode;
  newNode -> getDirent()[".."] = node;                                 
  DEBUGF ('i', dirname);
  return nullptr;
}

inode_ptr directory::mkfile (const string& filename) {
  DEBUGF ('i', filename);
  inode_ptr newFile = make_shared<inode>(file_type::PLAIN_TYPE);
  dirents.insert(make_pair(filename, newFile));
  return newFile;
}

wordvec& directory::getData() {
  throw file_error ("is a diretory");
}

map<string, inode_ptr>& directory::getDirent() {
  return dirents;
}

bool directory::isDirectory() const {
  return true;
}

directory::directory() {
  //cout << "d_ctor is called" << endl;
  dirents["."]  = nullptr;
  dirents[".."] = nullptr;
}

