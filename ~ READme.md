This program will send a udp datagram or segment to a server and compute the checksum on both sides. 
Then program will execute the .log files on either side after everything is outputted and write to those log files.
If the checksum is matched, It will sucessfully write to it. 


How to compile and run: connect the server side to cse03. connect the client side to cse04.
Then compile: gcc ufserver.c -o ufserver on cse03
compile client: gcc ufclient.c -o ufclient on cse04. 

compile and run SERVER SIDE FIRST, then proceed to compile and run client side.

You will have to control c on the server side to stop it.

Use the makefile to compile and run as well.  
