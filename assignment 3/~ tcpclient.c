// this program demonstrates a client and server tcp interaction with sending over a struct. computes checksum and prints to the files provided and terminal.


#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define SERVER "129.120.151.96" //cse03 server
#define PORT 1235


//initialize TCP header struct
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

unsigned short int computeChecksum(struct tcp_hdr tcp_seg); //computes checksum
int verifyChecksum(struct tcp_hdr tcp_seg);     //verifies checksum
void print_write(struct tcp_hdr tcp_seg);
void logMessage(char* message);
void logHeader(struct tcp_hdr tcp_seg);


int main(int argc,char **argv)
{
    struct tcp_hdr client_tcp_seg, server_tcp_seg;
    int sockfd, n, tmpres, portno;
    unsigned short int segment_arr[266];
    char* buffer;
    struct hostent *server; //stores the server ip


    int len = sizeof(struct sockaddr);

    struct sockaddr_in servaddr, clientadrr;

    
    sockfd=socket(AF_INET,SOCK_STREAM,0);
        if (sockfd < 0){
                printf("ERROR opening socket");
        }
        server = gethostbyname(SERVER);//changed to SERVER for ip address
        if(server == NULL){
                fprintf(stderr,"Error, no such host");
                exit(0);
        }

        bzero(&servaddr,sizeof(servaddr));
        servaddr.sin_family=AF_INET;
        servaddr.sin_port=htons(PORT); // changed to port 

        // server ip
    memcpy(&servaddr.sin_addr, server->h_addr, server->h_length);

    // establish a socket connection with the server
    connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr));

//getsockname 
if (getsockname(sockfd, (struct sockaddr *)&clientadrr, &len) == -1)
    perror("getsockname");

	printf("\n\n");
	printf("Port #: %d\n\n ",PORT);
	fflush(stdout);

// creating first tcp segment
  srand(time(0)); //srand for seeding random number 
  client_tcp_seg.src = (unsigned short int) ntohs(clientadrr.sin_port);
  client_tcp_seg.des = (PORT); // changed to PORT instead of argv[1]
  client_tcp_seg.seq = rand(); // random sequence number initiated
  client_tcp_seg.ack = 0;
  client_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b000010; // 1 set to ack
  client_tcp_seg.rec = 0;
  client_tcp_seg.ptr = 0;
  client_tcp_seg.cksum = computeChecksum(client_tcp_seg);
  bzero(client_tcp_seg.data,512);
  client_tcp_seg.data[512] = '\0'; // setting char of 512 to null

 logMessage("------- Demonstrating the TCP 3-way handshake -------\n");

 logMessage("-------- Initial Client TCP header, sending it to server -------\n");
       
    print_write(client_tcp_seg); //prints tcp segment to term

    //write  the tcp segment to the socket
    memcpy(segment_arr, &client_tcp_seg, 532); //Copying 532 bytes

    //writes segment to server
    if( write(sockfd , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("write/send failed");
        return 1;
    }

        read(sockfd, segment_arr,sizeof(segment_arr)); //reads segment from server 
        memcpy(&server_tcp_seg, segment_arr, 532); //Copying the 532 bytes 
        
	logMessage("--------- Response header received from Server --------\n");
        
	print_write(server_tcp_seg);//write to file to send to server_tcp_seg
        
	if(!verifyChecksum(server_tcp_seg)) {
                logMessage("Corrupted data\n");
        }
        else if(server_tcp_seg.ack == (client_tcp_seg.seq + 1)) {
                logMessage("Received ack -------\n");
        }
  
  client_tcp_seg.seq = server_tcp_seg.ack; //setting client seq # to server ackn number 
  client_tcp_seg.ack = server_tcp_seg.seq + 1; //setting akc num to server ack plus one
  client_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b010010; //changed 512 from data seg size
  client_tcp_seg.cksum = computeChecksum(client_tcp_seg);

        logMessage("--------- Modified Client TCP-header, sending it to server -------------\n");
        // print the header
        print_write(client_tcp_seg);
	memcpy(segment_arr, &client_tcp_seg, 532); //copying 152 bytes
    //write  the tcp segment to the socket
    if( write(sockfd , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

 logMessage("----------------- Finished the 3-way handshake ------------\n\n");

logMessage("----------- Demonstrating the closing of TCP connection ------------\n");
 

  client_tcp_seg.seq = 2046;
  client_tcp_seg.ack = 1024;
  client_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b000001; // setting the FIN bit to 1
  client_tcp_seg.cksum = computeChecksum(client_tcp_seg);

        logMessage("-------- Connection closing header, sending it to server --------\n");
        // print the header
        print_write(client_tcp_seg);

    //write  the tcp segment to the socket
    memcpy(segment_arr, &client_tcp_seg, 532); //copying 532 bytes

    if( write(sockfd , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

        read(sockfd, segment_arr,sizeof(segment_arr)); //reads struct from server
        memcpy(&server_tcp_seg, segment_arr, 532); //copying 532 bytes
        
	logMessage("--------- Response header received from Server -----------\n");
        print_write(server_tcp_seg);
        
	if(!verifyChecksum(server_tcp_seg)) {
                logMessage("Corrupted data\n");
        }  else
                logMessage("Checksum verification successfull!\n");

        read(sockfd, segment_arr,sizeof(segment_arr));
        memcpy(&server_tcp_seg, segment_arr, 532); //copying 532 bytes
        
	logMessage("--------- Finish header received from Server -------------\n");
        print_write(server_tcp_seg);
        
	if(!verifyChecksum(server_tcp_seg)) {
                logMessage("Corrupted data\n");
        }  else
                logMessage("Checksum verification successfull!\n");

  client_tcp_seg.seq++;
  client_tcp_seg.ack = server_tcp_seg.seq + 1;
  client_tcp_seg.hdr_flags = ((sizeof(client_tcp_seg)-512)/4 << 12) | 0b010010;
  client_tcp_seg.cksum = computeChecksum(client_tcp_seg);
  logMessage("-------- Final ACK for closing, sending it to server --------\n");
        // print the header
        print_write(client_tcp_seg);

    //write  the tcp segment to the socket
    memcpy(segment_arr, &client_tcp_seg, 532); //Copying 532 bytes

    if( write(sockfd , segment_arr , sizeof(segment_arr)) < 0)
    {
        puts("Write failed");
        return 1;
    }

    logMessage("--------- Finished closing the TCP connection -----------\n\n");

   close(sockfd);

   return 0;

}

//function that computes checksum 
unsigned short int computeChecksum(struct tcp_hdr tcp_seg) {

  unsigned short int cksum_arr[266];
  unsigned int i,sum=0, cksum;
  tcp_seg.cksum = 0; // reset the checksum field to zero

  memcpy(cksum_arr, &tcp_seg, 532); //Copying 152 bytes

  for (i=0;i<76;i++)               // Compute sum
   sum = sum + cksum_arr[i];

  cksum = sum >> 16;              // Fold once
  sum = sum & 0x0000FFFF;
  sum = cksum + sum;

  cksum = sum >> 16;             // Fold once more
  sum = sum & 0x0000FFFF;
  cksum = cksum + sum;


  /* XOR the sum for checksum */
  return (0xFFFF^cksum);
}

//checks the checksum is verified 
int verifyChecksum(struct tcp_hdr tcp_seg) {
        return (computeChecksum(tcp_seg) == tcp_seg.cksum);
}

//prints and writes to terminal
void print_write(struct tcp_hdr tcp_seg) {

  printf("Src: %d\n", tcp_seg.src); // prints source as everything else
  printf("Dest: %d", tcp_seg.des);
  printf("Seq: %d\n", tcp_seg.seq);
  printf("Ack: %d", tcp_seg.ack);
  printf("Header Flags: 0x%04X\n", tcp_seg.hdr_flags);
  printf("Checksum: 0x%04X\n", tcp_seg.cksum);
  
 
  // log the header to a file
  logHeader(tcp_seg);

}

//prints the message to show each segment being sent and tranmitted 
void logMessage(char* message) {
  // print the message to the console
  printf("%s",message);
  FILE *logFile = fopen("client.txt", "a+");
  // Writing all values to a file
  fputs(message, logFile);
  fclose(logFile);

}

//prints log header to client txt 
void logHeader(struct tcp_hdr tcp_seg) {

  FILE *logFile = fopen("client.txt", "a+");
  
  fprintf(logFile,"Src: %d\n", tcp_seg.src);
  fprintf(logFile," Dest: %d", tcp_seg.des);
  fprintf(logFile,"Seq: %d\n", tcp_seg.seq);
  fprintf(logFile," Ack: %d", tcp_seg.ack);
  fprintf(logFile,"Header Flags: 0x%04X\n", tcp_seg.hdr_flags);
  fprintf(logFile,"Checksum: 0x%04X\n", tcp_seg.cksum);
 
  

  fclose(logFile);
}


