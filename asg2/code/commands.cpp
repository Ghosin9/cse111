// $Id: commands.cpp,v 1.18 2019-10-08 13:55:31-07 - - $

#include "commands.h"
#include "debug.h"
#include <sstream>
#include <locale>
#include <iomanip>

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
   {"#"     , fn_hash  },
};

command_fn find_command_fn (const string& cmd) {
   // Note: value_type is pair<const key_type, mapped_type>
   // So: iterator->first is key_type (string)
   // So: iterator->second is mapped_type (command_fn)
   DEBUGF ('c', "[" << cmd << "]");
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
   int status = exec::status();
   cout << exec::execname() << ": exit(" << status << ")" << endl;
   return status;
}

void fn_hash (inode_state& state, const wordvec& words)
{
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   //method to do nothing for comments
}

void fn_cat (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //if no arguments given
   if(words.size()==1)
   {
      cout << "cat: need a filename" << endl;
      return;
   }

   map<string, inode_ptr> elements = 
      state.getCwd()->getContents()->getDirents();
   //check if file exits
   for(unsigned int itor = 1; itor<words.size(); ++itor)
   {
      //if cannot find element in dirents
      if(elements.find(words[itor]) == elements.end())
      {
         cout << "cat: " << words[itor];
         cout << ": No such file or directory" << endl;
         return;
      }

      //cout << "isfile of : " << itor->first << " is " << 
      //itor->second->getContents()->isFile() << endl;

      //if the file is found, but a directory
      if(!(elements.find(words[itor])->second->getContents()->isFile()))
      {
         cout << "cat: " << words[itor];
         cout << ": Is a directory" << endl;
         return;
      }
   }
   
   //looping through
   for(unsigned int itor = 1; itor<words.size(); ++itor)
   {
      auto file = elements.find(words[itor]);
      cout << file->second->getContents()->readfile() << endl;
   }
}

void fn_cd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //no arguments
   if(words.size() == 1 or words[1] == "/")
   {
      state.setCwd(state.getRoot());
      state.setCurrentDir("/");
      return;
   }

   //checking if dir exists
   map<string, inode_ptr> elements = 
      state.getCwd()->getContents()->getDirents();
   for(unsigned int itor = 1; itor<words.size(); ++itor)
   {
      //if cannot find element in dirents
      if(elements.find(words[itor]) == elements.end())
      {
         cout << "cd: " << words[itor];
         cout << ": No such directory" << endl;
         return;
      }

      //if the file is found, but a directory
      if((elements.find(words[itor])->second->getContents()->isFile()))
      {
         cout << "cd: " << words[itor];
         cout << ": Is a file" << endl;
         return;
      }
   }

   //change the state name
   state.setCurrentDir(state.getCurrentDir() + words[1] + "/");

   //change the actual state
   state.setCwd(elements.find(words[1])->second);

}

//done****************************************************************
void fn_echo (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);
   cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


//done****************************************************************
void fn_exit (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   int temp;

   //if no status given
   if(words.size() ==1)
   {
      //cout << "exiting with 0" << endl;
      exec::status(0);
   }
   else
   {
      istringstream convert(words[1]);
      //if given exit status without digits
      if(!(convert >> temp))
      {
         //cout << "exiting without digits" << endl;
         exec::status(127);
      }
      //if exit status with digits
      else
      {
         //cout << "exiting with digits" << endl;
         exec::status(temp);
      }
   }

   throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   map<string, inode_ptr> elements;

   //if no arguments
   if(words.size()==1 or words[1] == "/" or words[1] == ".")
   {
      //print cur dir
      cout << state.getCurrentDir() << ":" << endl;

      elements = 
      state.getCwd()->getContents()->getDirents();
      for(auto itor = elements.begin(); itor != elements.end(); ++itor)
      { 
         cout << setw(6);
         cout << itor->second->get_inode_nr() << " ";
         cout << setw(6);
         cout << itor->second->getContents()->size();
         cout << " ";
         cout << itor->first;
         //if the element is a directory
         if(!(itor->second->getContents()->isFile()))
         {
            cout << "/";
         }
         cout << endl;
      }
   }
   else
   {
      if (words[1] == "..")
      {
         //cout << "inside ls .. " << endl;

         elements = 
         state.getCwd()->getContents()->getDirents();

         //get returning directory
         inode_ptr returnDir = state.getCwd();
         //get returning name
         string returnName = state.getCurrentDir();

         //temp change to new directory
         state.setCwd(elements.find("..")->second);
         //create wordvec for new name
         wordvec newName = split(state.getCurrentDir(), "/");
         //change to new name and remove the last one 
         string newCurrentDir = "/";
         for(unsigned int itor = 0; itor < newName.size()-1; ++itor)
         {
            newCurrentDir += newName[itor] + "/";
         }
         state.setCurrentDir(newCurrentDir);

         //cout << "temp cwd: " << endl;

         //print cur dir
         cout << state.getCurrentDir() << ":" << endl;

         elements = 
         state.getCwd()->getContents()->getDirents();
         for(auto itor = elements.begin(); 
            itor != elements.end(); ++itor)
         { 
            cout << setw(6);
            cout << itor->second->get_inode_nr() << " ";
            cout << setw(6);
            cout << itor->second->getContents()->size();
            cout << " ";
            cout << itor->first;
            //if the element is a directory
            if(!(itor->second->getContents()->isFile()))
            {
               cout << "/";
            }
            cout << endl;
         }

         state.setCwd(returnDir);
         state.setCurrentDir(returnName);
      }
      else
      {
         //for other ls calls
      }
   }
}

void fn_lsr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if(words.size()==1 or words[1] == "/" or words[1] == ".")
   {
      fn_ls(state, words);
   }
   else
   {
      //check for subdirectories
      map <string, inode_ptr> elements = 
            state.getCwd()->getContents()->getDirents();
      for(auto itor = elements.begin(); itor != elements.end(); ++itor)
      {
         if(itor->first == "." or itor->first == "..")
            continue;

         //if the itor is a directory
         if(!(itor->second->getContents()->isFile()))
         {
            fn_ls(state, words);
         }
      }
   }
}

void fn_make (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //check for arguments
   if (words.size() == 1)
   {
      cout << "make: need filename" << endl;
      return;
   }

   map<string, inode_ptr> elements = 
   state.getCwd()->getContents()->getDirents();
   //check if file exits
   //if cannot find element in dirents
   if(elements.find(words[1]) != elements.end())
   {
      cout << "make: " << words[1];
      cout << ": File or directory already exists" << endl;
      return;
   }

   //cout << "inside make" << endl;

   //new file name
   string newFile = words[1];

   //cout << "name of file: " << newFile << endl;

   //new file contents
   string fileContents;
   for(unsigned int itor = 2; itor < words.size(); ++itor)
   {
      fileContents += words[itor] + " ";
   }
   wordvec fileWords = split(fileContents, " ");

   //cout<< "wordvec contents: " << fileWords << endl;

   inode_ptr file = state.getCwd()->getContents()->mkfile(newFile);
   file->getContents()->writefile(fileWords);

   //cout << "boolean of file: " << 
   //file->getContents()->isFile() << endl;
}

void fn_mkdir (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //check for arguments
   if (words.size() == 1)
   {
      cout << "mkdir: need directory name" << endl;
      return;
   }

   map<string, inode_ptr> elements = 
   state.getCwd()->getContents()->getDirents();
   //check if file exits
   //if cannot find element in dirents
   if(elements.find(words[1]) != elements.end())
   {
      cout << "mkdir: " << words[1];
      cout << ": File or directory already exists" << endl;
      return;
   }

   //new directory name
   string newDir = words[1];

   //creating new dir
   inode_ptr dir = state.getCwd()->getContents()->mkdir(newDir);

   //adding . and ..
   dir->getContents()->addEntry(".", dir);
   dir->getContents()->addEntry("..", state.getCwd());
}

//done****************************************************************
void fn_prompt (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   if(words.size()==1)
   {
      cout << "prompt: need a new prompt";
      return;
   }

   string newPrompt;

   for(unsigned int itor = 1; itor < words.size(); ++itor)
   {
      newPrompt += words[itor] + " ";
   }

   state.setPrompt(newPrompt);
}

//done****************************************************************
void fn_pwd (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   cout << state.getCurrentDir() << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //checking argument size
   if(words.size() == 1)
   {
      cout << "rm: need file or directory name" << endl;
      return;
   }

   //looking if file/directory exists
   map<string, inode_ptr> elements = 
   state.getCwd()->getContents()->getDirents();
   if(elements.find(words[1]) != elements.end())
   {
      cout << "rm: " << words[1];
      cout << ": File or directory does not exist" << endl;
      return;
   }

   //checking if it is a directory
   if(!(elements.find(words[1])->second->getContents()->isFile()))
   {

   }

   state.getCwd()->getContents()->remove(words[1]);
}

void fn_rmr (inode_state& state, const wordvec& words){
   DEBUGF ('c', state);
   DEBUGF ('c', words);

   //checking argument size
   if(words.size() == 1)
   {
      cout << "rmr: need file or directory name" << endl;
      return;
   }

   //rmr recursively calls rm and rmr
}

