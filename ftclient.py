'''
Prog Name: ftclient
Description: Simple client program for a file transfer system.
             Takes server IP and port from command line, and one of two flags
             for different commandes. -l requests the contents for the server
             directory. -g followed by a filename, requests the specific file
             from the server. -g overwrites any file at the destination wtih the
             same filename.
To run:  python ftclient.py <serverIP> <serverPort> <-l or -g> <filename if -g> <clientPort>
         Example to request directory contents from server running on localhost:12001: 
         python ftclient.py localhost 12001 -l 12002 

         Example to request file from server running on localhost:12001:
         python ftclient.py localhost 12001 -g file.txt 12002

CITATION: This code was influenced by:
   geeksforgeeks.org/socket-programming-python
'''


import socket
import fileinput
import sys
import time


#parse args
serverAddr = sys.argv[1] 
serverPort = int(sys.argv[2]) 
commandFlag = sys.argv[3]

#for -l, only save the clientPort
if commandFlag == "-l":
   clientPort = int(sys.argv[4])
   sendCommand = commandFlag + " " + str(clientPort) 

# for -g, save requested filename and client port
if commandFlag == "-g":
   transFile = sys.argv[4]
   clientPort = int(sys.argv[5])
   sendCommand = commandFlag + " " + transFile + " " + str(clientPort)

#create socket
clientSock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

try:
   #connect to server
   clientSock.connect((serverAddr, serverPort))

   #for - l, send message requesting server contents
   if commandFlag == "-l": 
      #send to server
      clientSock.send(sendCommand.encode())
      #wait for response then display contents of server
      msgIn = clientSock.recv(500)
      print('\n')
      sys.stdout.write("Receiving directory structure from " + str(serverAddr) + ":" + str(serverPort))
      print('\n')
      print(msgIn)
      print('\n')
      #assignment asks to close socket after each command
      clientSock.close() 

   #request a specific file from server
   elif commandFlag == "-g":
      #send request and wait for return message
      clientSock.send(sendCommand.encode())
      msgIn = clientSock.recv(500)
      
      #if file was not found
      if(msgIn == "File not found"):
         print('\n')
         sys.stdout.write(str(serverAddr) + ":" + str(serverPort) + " says FILE NOT FOUND")
         print('\n') 
         clientSock.close()

      #file was found, need to download and reconstruct
      else:
         print('\n')
         sys.stdout.write("Receiving \"" + str(transFile) + "\" from " + str(serverAddr) + ":" + str(serverPort))
         print("\n")
         #open file with same name and write received data to it 
         inFile = open(transFile, "w")
         if(msgIn.endswith("@End")):
            inFile.write(msgIn.replace("@End", ""))
            inFile.close()
            recFile = 0
         else:
            inFile.write(msgIn)
            inFile.close()
            recFile = 1
         #loop as long as needed to get all file contents
         while (recFile == 1):
            msgIn = clientSock.recv(500) 
            #for end of file contents 
            if(msgIn.endswith("@End")):
               recFile = 0
               inFile = open(transFile, "a")
               inFile.write(msgIn.replace("@End", ""))
               inFile.close() 
            #not the end of the file, so continue writing
            else:
               inFile = open(transFile, "a")
               inFile.write(msgIn)
               inFile.close() 
 
         print("File transfer complete.\n") 
         clientSock.close() 

   #unknown command
   else:
      print("Command not recognized. Try Again")

finally:
   clientSock.close() 
