// $Id: file_sys.cpp,v 1.7 2019-07-09 14:05:44-07 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

size_t inode::next_inode_nr {1};

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
   // DEBUGF ('i', "root = " << root << ", cwd = " << cwd
   //        << ", prompt = \"" << prompt() << "\"");

  //create the root directory
   root = make_shared<inode>(file_type::DIRECTORY_TYPE);
   //set current dir
   cwd = root;
   root->contents->addEntry(".", root);
   currentDir = "/";
   //set root as its own parent
   root->contents->addEntry("..", root);
}

const string& inode_state::prompt() const { return prompt_; }

ostream& operator<< (ostream& out, const inode_state& state) {
   out << "inode_state: root = " << state.root
       << ", cwd = " << state.cwd;
   return out;
}

inode::inode(file_type type): inode_nr (next_inode_nr++) {
   switch (type) {
      case file_type::PLAIN_TYPE:
           contents = make_shared<plain_file>();
           contents->setFile(true);
           break;
      case file_type::DIRECTORY_TYPE:
           contents = make_shared<directory>();
           contents->setFile(false);
           break;
   }
   DEBUGF ('i', "inode " << inode_nr << ", type = " << type);
}

int inode::get_inode_nr() const {
   DEBUGF ('i', "inode = " << inode_nr);
   return inode_nr;
}

//for compilier
void base_file::addEntry(const string& filename, inode_ptr pointer)
{
  DEBUGF('i', filename);
  DEBUGF('i', pointer);
  throw file_error("is a " + error_file_type());
}

//for compilier
map<string,inode_ptr> base_file::getDirents()
{
  throw file_error("is a " + error_file_type());
}


file_error::file_error (const string& what):
            runtime_error (what) {
}

const wordvec& base_file::readfile() const {
   throw file_error ("is a " + error_file_type());
}

void base_file::writefile (const wordvec&) {
   throw file_error ("is a " + error_file_type());
}

void base_file::remove (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkdir (const string&) {
   throw file_error ("is a " + error_file_type());
}

inode_ptr base_file::mkfile (const string&) {
   throw file_error ("is a " + error_file_type());
}


size_t plain_file::size() const {
   size_t size = 0;

   //loop through data array,
   //adding lengths of each string
   for(unsigned int itor = 0; itor < data.size(); ++itor)
   {
    string tempString = data[itor];
    size += tempString.length();
   }

   DEBUGF ('i', "size = " << size);
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

size_t directory::size() const {
   size_t size = dirents.size();
   return size;
}

void directory::addEntry(const string& filename, inode_ptr pointer)
{
  dirents.insert(std::pair<string, inode_ptr>(filename, pointer));
}

void directory::remove (const string& filename) {
  dirents.erase(filename);
}

inode_ptr directory::mkdir (const string& dirname) {
  inode_ptr newDir = make_shared<inode>(file_type::DIRECTORY_TYPE);

  //set name of new directory
  dirents.emplace(dirname, newDir);

   DEBUGF ('i', dirname);
   return newDir;
}

inode_ptr directory::mkfile (const string& filename) {
   DEBUGF ('i', filename);

   inode_ptr newFile = make_shared<inode>(file_type::PLAIN_TYPE);

   //set filename
   dirents.emplace(filename, newFile);

   return newFile;
}

