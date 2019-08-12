/*****************************************************************************
 *Prog Name: ftserver
 *Description: This is a simple server program for a file transfer system.
 *             It returns the folder contents, and a file requested if the file
 *             exists.
 *To compile: gcc -o ftserver ftserver.c
 *To run:     ftserver <portNum>               
 *CITATION: This code was influenced by:
 *          Beej's Guide https://beej.us/guide/bgnet
 * **************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>

#define MAXDATASIZE 500

//error handling function
void error(const char *msg) {perror(msg); exit(0);}


int main(int argc, char *argv[]){

   //save port number from command line 
   //setup the various vars for the socket 
   int serverPort = atoi(argv[1]);
   int i, j, msgLen;  
   int sockfd, newsock_fd, msgSize;
   socklen_t clientAddrSize;
   char msgBuffer[MAXDATASIZE];
   char sendBuffer[MAXDATASIZE];
   struct sockaddr_in serverAddr, clientAddr;

   memset((char*)&serverAddr, '\0', sizeof(serverAddr));
   serverAddr.sin_family = AF_INET;
   serverAddr.sin_port = htons(serverPort);
   serverAddr.sin_addr.s_addr = INADDR_ANY;

   //create listen socket, sock_stream = tcp  
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if(sockfd < 0){
      error("Error opening socket.");
   } 

   if(bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
      close(sockfd);
      error("Error binding.");
   }
   
   //server is now listening   
   printf("Server open on %d\n", serverPort);
   printf("\n");

   //keep connection open on server
   while(1){
      char *args[5];
      int argCt = 0;

      //this allows one connection at a time per project specs
      listen(sockfd, 1);

      clientAddrSize = sizeof(clientAddr);
      //new socket for the connection from client
      newsock_fd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientAddrSize);

      //recieve message and handle
      memset(msgBuffer, '\0', sizeof(msgBuffer));
      msgSize = recv(newsock_fd, msgBuffer, MAXDATASIZE, 0);
      if(msgSize < 0){
         error("Error recieving message.");
      }

      //save client address
      //Citation: linux.die.net/man/3/inet_ntoa
      char *clientName = inet_ntoa(clientAddr.sin_addr);
      printf("Connection from %s\n", clientName);

      //parse command and save to array for processing 
      char *token = strtok(msgBuffer, " "); 
      while(token != '\0'){
         args[argCt] = strdup(token);
         token = strtok(NULL, " ");
         argCt++; 
      }

      //-l flag is a request for files in current directory
      if(strcmp(args[0], "-l") == 0){
         printf("List directory requested on port %s.\n", args[1]);
         printf("Sending directory contents to %s:%s\n", clientName, args[1]); 

         char *dirList[100];
         int dirListCt = 0;
            
         //get contents and return
         DIR *dirStream;
         struct dirent *dirContent;

         //open current dir
         dirStream = opendir("./"); 
         if(dirStream != NULL){
            //save file names to array
            while(dirContent = readdir(dirStream)){
               dirList[dirListCt] = dirContent->d_name;
               dirListCt++; 
            } 
         }
         else{
            error("Error opening directory.");
         }

         (void)closedir(dirStream);
   
         //create string to send from array file list
         memset(sendBuffer, '\0', sizeof(sendBuffer));
         
         for(i = 0; i < dirListCt; i++){
            strcat(sendBuffer, dirList[i]);
            strcat(sendBuffer, " "); 
         }
         printf("\n");
         msgLen = strlen(sendBuffer);

         //send directory connects to client 
         send(newsock_fd, sendBuffer, msgLen, 0);
         close(newsock_fd);
      }

      //-g flag is a request for a specific file
      else if(strcmp(args[0], "-g") == 0){
         printf("File \"%s\" requested on port %s.\n", args[1], args[2]);
         char *dirList[100];
         int dirListCt = 0;

         //get contents of directory
         DIR *dirStream;
         struct dirent *dirContent;

         //open current dir
         dirStream = opendir("./"); 
         if(dirStream != NULL){
            //save file names to array
            while(dirContent = readdir(dirStream)){
               dirList[dirListCt] = dirContent->d_name;
               dirListCt++; 
            } 
         }
         else{
            error("Error opening directory.");
         }

         (void)closedir(dirStream);
  
         //check if requested file is in current directory
         int fileExists = 0;
         for(j = 0; j < dirListCt; j++){
            if(strcmp(args[1], dirList[j]) == 0){
               fileExists = 1;
            }
         }

         //if file does not exist, send message to client and close client socket
         if(fileExists == 0){ 
            printf("File not found. Sending error message to %s:%s\n", clientName, args[2]);
            send(newsock_fd, "File not found", 14, 0);
            close(newsock_fd);
         }
         //file does exist, so send it to client
         else{
            printf("Sending \"%s\" to %s:%s\n", args[1], clientName, args[2]);

            //open file and read in chunks if necessary
            memset(sendBuffer, '\0', sizeof(sendBuffer)); 

            //Citation: linux.die.net/man/3/fread
            //ssize_t bytes_read;
            size_t wereBytesRead;
            int keepReading = 1;
            FILE *readFile;
            readFile = fopen(args[1], "r"); 

            //this loop continues reading until the end of file is read
            while(keepReading == 1){
               memset(sendBuffer, '\0', sizeof(sendBuffer));
               wereBytesRead = fread(sendBuffer, 1, 500, readFile);

               //if data was read, send it to client 
               if(wereBytesRead > 0){
                  msgLen = strlen(sendBuffer);
                  send(newsock_fd, sendBuffer, msgLen, 0);
               } 

               //no data was read, send term message
               else{
                  char endBuffer[5];
                  memset(endBuffer, '\0', sizeof(endBuffer));
                  strcpy(endBuffer, "@End");
                  msgLen = strlen(endBuffer);
                  keepReading = 0;
                  send(newsock_fd, endBuffer, msgLen, 0);  
               }   
            }
  
            //close file and client socket 
            fclose(readFile);
            close(newsock_fd);
         }
      }
      //if original command not recognized
      else{
         printf("Command not recognized. Try again.\n");
      }
   }

   return 0;
}
