// $Id: cix.cpp,v 1.9 2019-04-05 15:04:28-07 - - $

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdio.h>
#include <sstream>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream outlog (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"get" , cix_command::GET},
   {"put" , cix_command::PUT},
   {"rm"  , cix_command::RM},
};

static const char help[] = R"||(
exit         - Exit the program.  Equivalent to EOF.
get filename - Copy remote file to local host.
help         - Print help summary.
ls           - List names of files on remote server.
put filename - Copy local file to remote host.
rm filename  - Remove file from remote server.
)||";

void cix_help() {
   cout << help;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;

   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   if (header.command != cix_command::LSOUT) {
      outlog << "sent LS, server did not return LSOUT" << endl;
      outlog << "server returned " << header << endl;
   }else {
      size_t host_nbytes = ntohl (header.nbytes);
      auto buffer = make_unique<char[]> (host_nbytes + 1);
      recv_packet (server, buffer.get(), host_nbytes);
      outlog << "received " << host_nbytes << " bytes" << endl;
      buffer[host_nbytes] = '\0';
      cout << buffer.get();
   }
}

void cix_get (client_socket& server, string filename)
{
   cix_header header;
   header.command = cix_command::GET;

   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   //file does not exist
   if(header.command != cix_command::FILEOUT)
   {
      outlog << filename << " does not exist on the server" << endl;
      outlog << "server returned " << header << endl;
   }
   else
   {
      //set up buffer
      int length = ntohl(header.nbytes);
      auto buffer = make_unique<char[]> (length + 1);
      buffer[length] = '\0';

      //payload
      recv_packet (server, buffer.get(), length);
      outlog << "received " << length << " bytes" << endl;

      //writing file
      std::ofstream os (header.filename, std::ofstream::binary);
      os.write(buffer.get(), length);
   }
}

void cix_put (client_socket& server, string filename)
{
   cix_header header;

   std::ifstream is (header.filename, std::ifstream::binary);

   if(is) //file exists
   {
      // get length of file:
      is.seekg (0, is.end);
      int length = is.tellg();
      is.seekg (0, is.beg);

      //create buffer for reading
      auto buffer = make_unique<char[]> (length);

      //read the file
      is.read(buffer.get(), length);

      //set command to get, set nbytes
      header.command = cix_command::PUT;
      header.nbytes = length;

      //payload
      send_packet (server, buffer.get(), length);
   }
   else //file does not exist
   {
      outlog << filename << " failed to transfer to server" << endl;
      return;
   }

   is.close();

   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   //successful put
   if(header.command == cix_command::ACK)
   {
      cout << "Sucessfully put: " << filename;
      cout << " on the server" << endl;
   }
   else if (header.command == cix_command::NAK) //failed put
   {
      cout << "Failed to put: " << filename;
      cout << " on the server" << endl;
   }
}

void cix_rm (client_socket& server, string filename)
{
   cix_header header;

   header.command = cix_command::RM;
   header.nbytes = 0;

   outlog << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   outlog << "received header " << header << endl;

   //successful prm
   if(header.command == cix_command::ACK)
   {
      cout << "Sucessfully removed: " << filename;
      cout << " from the server" << endl;
   }
   else if (header.command == cix_command::NAK) //failed put
   {
      cout << "Failed to remove: " << filename;
      cout << " from the server" << endl;
   }
}


void usage() {
   cerr << "Usage: " << outlog.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

int main (int argc, char** argv) {
   outlog.execname (basename (argv[0]));
   outlog << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   outlog << to_string (hostinfo()) << endl;
   try {
      outlog << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      outlog << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         outlog << "command " << line << endl;

         //split line into tokens
         vector<string> contents;
         std::istringstream ss(line);
         std::string token;
         while(std::getline(ss, token, ' ')) 
         contents.push_back(token);

         const auto& itor = command_map.find (contents[0]);
         cout << contents[0] << " " << contents[1] << endl;
         cix_command cmd = itor == command_map.end()
                         ? cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
               throw cix_exit();
               break;
            case cix_command::HELP:
               cix_help();
               break;
            case cix_command::LS:
               cix_ls (server);
               break;
            case cix_command::GET:
               cix_get(server, contents[1]);
               break;
            case cix_command::PUT:
               cix_put(server, contents[1]);
               break;
            case cix_command::RM:
               cix_rm(server, contents[1]);
               break;
            default:
               outlog << line << ": invalid command" << endl;
               break;
         }
      }
   }catch (socket_error& error) {
      outlog << error.what() << endl;
   }catch (cix_exit& error) {
      outlog << "caught cix_exit" << endl;
   }
   outlog << "finishing" << endl;
   return 0;
}

