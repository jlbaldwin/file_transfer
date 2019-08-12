# File transfer server/client

This project consists of two programs: ftserver and ftclient. ftserver is written in c and requires compilation. The server program will 
remain active and listening until it is terminated from the terminal (with ctrl-c, for example). The server accepts two request flags: -l and -g.
-l returns a current directory listing of the server. -g has an additional argument for a filename, and will transfer the file if it exists in the 
server's current directory. If the file does not exist, a message with be sent to the client. 

ftclient, is written in python and does not need to be compiled. Once running, ftclient will accept the -l and -g commands and 
send to the server input at the command line. -l returns the server contents and is displayed on client. -g returns the specific file to 
client, and the file will be written to the client current directory.

## Launch

This was tested in the bash shell running on windows, with python 2.7.15

To compile ftserver.c:  
$ gcc -o ftserver ftserver.c

To run program:  
Start ftserver after it has been compiled. From command line:  
$ ftserver `<portNum>`   
`<portNum>` is the port the server will be listening on  

Start ftclient after ftserver is running:  
$python ftclient.py <server IP> <server Port> <-l -g flag> <filename, if -g> <client port>  

Example to request directory contents from server running on localhost:12001:  
$ python ftclient.py localhost 12001 -l 12002  

Example to request file from server running on localhost:12001:  
$ python ftclient.py localhost 12001 -g file.txt 12002    	   				   	

ftclient closes after each request per specs. ftserver remians open until it is terminated at the terminal (with ctrl-c, for example)

