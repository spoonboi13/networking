#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h> // for rand time 

#define PORT 1235 //what port to listen to


// struct of tcp header 
struct tcp_hdr{
                unsigned short int src;
                unsigned short int des;
                unsigned int seq;
                unsigned int ack;
                unsigned short int hdr_flags;
                unsigned short int rec;
                unsigned short int cksum;
                unsigned short int ptr;
                char data[512];
              };

unsigned short int computeChecksum(struct tcp_hdr tcp_seg);//computes checksum
int verifyChecksum(struct tcp_hdr tcp_seg);//verifies checksum 
void print_write(struct tcp_hdr tcp_seg);
void logMessage(char* message); //prints message so i can follow what step im on 
void logHeader(struct tcp_hdr tcp_seg); //prints the actual data

int main(int argc, char *argv[])
{
        struct tcp_hdr client_tcp_seg, server_tcp_seg;
        int sockfd, connected, bytes_recieved , true = 1;
        unsigned short int segment_arr[266];
       

        struct sockaddr_in server_addr,client_addr;
        int sin_size;

        //creating sockfd
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("error connecting socket");
            exit(1);
        }
	//setting sockopt
        if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
            perror("error in setsockopt");
            exit(1);
        }
	
	//setting the struct of server addr 
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT); // port set to 1235
        server_addr.sin_addr.s_addr = INADDR_ANY;
        memset(&(server_addr.sin_zero),0,8);
        if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))== -1)
        {
            perror("Unable to bind");
            exit(1);
        }

        if (listen(sockfd, 5) == -1)
        {
            perror("Listen");
            exit(1);
        }

        printf("\n\nPort #: %d \n", PORT); // prints port number 
        fflush(stdout);


        sin_size = sizeof(struct sockaddr_in);

        connected = accept(sockfd, (struct sockaddr *)&client_addr,&sin_size);

        read(connected, segment_arr,sizeof(segment_arr)); //reading segment sent from client 
	 memcpy(&client_tcp_seg, segment_arr, 532); //Copying 532 bytes from client 

 logMessage("---------- Starting TCP 3-way handshake ---------\n");
 
 logMessage("---------- Received TCP segment from the client -----------\n");
        
 	// print the header
        print_write(client_tcp_seg);

        // if checksum is not verified correctly, print the corrupted data message
        if(!verifyChecksum(client_tcp_seg)) {
                logMessage("corrupted data!\n");
        } else
        
	logMessage("checksum verification successfull!\n");

	srand(time(0)); // setting up srand for random number

  // The  server  responds  to  the  request  by  creating  a  connection  granted  TCP  segment
  server_tcp_seg.src = PORT;
  server_tcp_seg.des = (unsigned short int) ntohs(client_addr.sin_port);
  server_tcp_seg.seq = rand(); // random sequence number
  server_tcp_seg.ack = client_tcp_seg.seq + 1; //client seq number +1 for ackn server
  server_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)- 512)/4 << 12) | 0b010010; //setting ACK and SYN to 1 
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);
  bzero(server_tcp_seg.data,512);
  server_tcp_seg.data[512] = '\0'; // setting data to null characters 
        
  
  	// print the header
        logMessage("------------ Response segment, created by server -----------\n");
        print_write(server_tcp_seg); //print response from server

    memcpy(segment_arr, &server_tcp_seg, 532); //Copying 152 bytes
    //write  the tcp segment to the socket
    //sleep(1); //sleeps for 1 seconds 
    
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

        read(connected, segment_arr,sizeof(segment_arr));
        memcpy(&client_tcp_seg, segment_arr, 532); //Copying 152 bytes

        logMessage("--------- Received TCP segment from the client -------------\n");
        print_write(client_tcp_seg);
        
	if(!verifyChecksum(client_tcp_seg)) {
                logMessage("corrupted data!\n");
        }
        else if(client_tcp_seg.ack == (server_tcp_seg.seq + 1)) {
                logMessage("Received ACK! \n\n");
        }

 logMessage("------------------- Finished the TCP 3-way handshake ----------------\n\n");


       //read(connected, segment_arr,sizeof(segment_arr));
        //memcpy(&client_tcp_seg, segment_arr, 532); //Copying 152 bytes

   // if(!verifyChecksum(client_tcp_seg)) {
     //           logMessage("corrupted data\n");
       // }

 

   //print_write(client_tcp_seg);

   // memcpy(segment_arr, &server_tcp_seg, 532); //Copying 152 bytes
    //Write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("write failed");
        return 1;
    }

   // print_write(server_tcp_seg);
    
    read(connected, segment_arr,sizeof(segment_arr)); //read segment from client 
    memcpy(&client_tcp_seg, segment_arr, 532); //Copying 152 bytes
    
    if(!verifyChecksum(client_tcp_seg)) {
                logMessage("corrupted data!\n");
        }
    

 logMessage("------------- Starting the closing of TCP connection ----------------------\n");
 
 logMessage("------------- Received TCP close request from the client --------------------\n");
        // print the header
        print_write(client_tcp_seg);

        if(!verifyChecksum(client_tcp_seg)) {
                logMessage("corrupted data!\n");
        } else
                logMessage("checksum verification successfull!\n");

  // create new struct to respond to client 
  server_tcp_seg.seq = 1024;
  server_tcp_seg.ack = client_tcp_seg.seq + 1;
  server_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b010010; //sets ACK bit to 1 
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);

        // print the header
        logMessage("---------------- Response segment of connection close request, created by server --------------\n");
        print_write(server_tcp_seg);

        memcpy(segment_arr, &server_tcp_seg, 532); //Copying 152 bytes
    //write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }
  
  server_tcp_seg.seq = 1024;
  server_tcp_seg.ack = client_tcp_seg.seq + 1;
  server_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b000001; //sets FIN bit to 1
  server_tcp_seg.rec = 0;
  server_tcp_seg.ptr = 0;
  server_tcp_seg.cksum = computeChecksum(server_tcp_seg);

        // print the header
        logMessage("------------ Close request from  server, sending to client ------------\n");
        print_write(server_tcp_seg);

    memcpy(segment_arr, &server_tcp_seg, 532); //Copying 152 bytes

    //Write  the tcp segment to the socket
    if( write(connected , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

        read(connected, segment_arr,sizeof(segment_arr));
        memcpy(&client_tcp_seg, segment_arr, 532); //Copying 152 bytes

        logMessage("---------------- Received final ACK from the client --------------\n");

        // print the header
        print_write(client_tcp_seg);

        if(!verifyChecksum(client_tcp_seg)) {
                logMessage("corrupted data!\n");
        }  else
                logMessage("checksum verification successfull!\n");

        logMessage("------------ Finished closing the TCP connection ---------------\n\n");

        close(connected);


        return 0;
}
//compute checksum 
unsigned short int computeChecksum(struct tcp_hdr tcp_seg) {

  unsigned short int cksum_arr[266];
  unsigned int i,sum=0, cksum;
  tcp_seg.cksum = 0; // reset the checksum field to zero

  memcpy(cksum_arr, &tcp_seg, 532); //Copying 152 bytes

  for (i=0;i<266;i++)               // Compute sum
   sum = sum + cksum_arr[i];

  cksum = sum >> 16;              // fold once
  sum = sum & 0x0000FFFF;
  sum = cksum + sum;

  cksum = sum >> 16;             // fold again
  sum = sum & 0x0000FFFF;
  cksum = cksum + sum;

  //XOR the sum for checksum 
  return (0xFFFF^cksum);
}

//function to verify checksum to make it easier
int verifyChecksum(struct tcp_hdr tcp_seg) {
        return (computeChecksum(tcp_seg) == tcp_seg.cksum);
}

//function to print to terminal
void print_write(struct tcp_hdr tcp_seg) {

   printf("Src: %d\n", tcp_seg.src); //prints the whole segment
  printf(" Dest: %d", tcp_seg.des);
  printf("Seq: %d\n", tcp_seg.seq);
  printf(" Ack: %d", tcp_seg.ack);
  printf("Header Flags: 0x%04X\n", tcp_seg.hdr_flags);
  printf("Checksum: 0x%04X\n", tcp_seg.cksum);
  
  // log the header to a file
  logHeader(tcp_seg);

}

//function to print header to file 
void logMessage(char* message) {
  // print the message to the terminal
  printf("%s",message);
  FILE *logFile = fopen("server.txt", "a+");
  // writing the values to a file
  fputs(message, logFile);
  fclose(logFile);

}
//function to print struct to file 
void logHeader(struct tcp_hdr tcp_seg) {

  FILE *logFile = fopen("server.txt", "a+");
  // writing the values to a file
  fprintf(logFile,"Src: %d\n", tcp_seg.src);
  fprintf(logFile," Dest: %d", tcp_seg.des);
  fprintf(logFile,"Seq: %d\n", tcp_seg.seq);
  fprintf(logFile," Ack: %d", tcp_seg.ack);
  fprintf(logFile,"Header Flags: 0x%04X\n", tcp_seg.hdr_flags);
  fprintf(logFile,"Checksum: 0x%04X\n", tcp_seg.cksum);
  
  fclose(logFile);
}


