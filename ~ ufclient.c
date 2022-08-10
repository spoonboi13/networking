/*
    Simple udp client
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/udp.h>	// for the header
#include<netinet/ip.h>	//for the ip header
 
#define SERVER "129.120.151.96" // cse03
#define BUFLEN 512  //Max length of buffer
#define PORT 1235   //The port on which to send data
 
//used https://stackoverflow.com/questions/31995486/use-of-memcpy-to-store-data-from-buffer-into-struct to help find buffer with struct 
//
//struct to hold the headers. found online github to help with headers and how to send datagram through sockets 
//using pseudo_header
struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t udp_length;
};
//udp struct of segment 
struct udp_segment {
  unsigned short source_port;
  unsigned short dest_port;
  unsigned short segment_length;
  unsigned short checksum;
  unsigned char data[513];
};
//checksum declaration  
unsigned short Checksum(unsigned short *ptr,int nbytes);


void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_other;
    int sockfd, i=0, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    FILE* fp;

        if ( (sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
   
    {
        die("socket");
    }
 
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET; 
    si_other.sin_port = htons(1235); //uses port 1235
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }


    // data that will be the packet
    char datagram[4096] , source_ip[32] , *data , *pseudogram;
	
    
    memset (datagram, 0, 4096); //zeros out the packet buffer 
	
    // ip header declared and setting it to the datagram 
    struct iphdr *iph = (struct iphdr *) datagram;
	
    // udp header and setting it to the size of the segment 
    struct udphdr *udphdr = (struct udphdr *) (datagram + sizeof (struct ip));
    struct udp_segment *udph = (struct udp_segment *) (datagram + sizeof (struct ip));	
    struct sockaddr_in sin;
    struct pseudo_header psh;
	
    // setting the datagram to be the size of each header  
    data = datagram + sizeof(struct iphdr) + sizeof(struct udphdr);
   

    // used https://stackoverflow.com/questions/22059189/read-a-file-as-byte-array to help put file in array 
    if (fp = fopen("input.txt", "r")){
     char* buffer = malloc(512);
     while(fread(buffer, 512, 1, fp) == 1){
       strcpy(udph->data, buffer);	
          //do whatever
       //printf("Read 512 segment from file\n");
     }
     fclose (fp);
     free(buffer);
    }
    // help with address 
    strcpy(source_ip , "129.120.151.96"); //127.0.0.1
	
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr.s_addr = inet_addr ("129.120.151.96");


    // fill in the ip headers 
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = sizeof (struct iphdr) + sizeof (struct udp_segment);
    iph->id = htonl (54321);	//Id of this packet
    iph->frag_off = 0;
    iph->ttl = 255;
    iph->protocol = IPPROTO_UDP;
    iph->check = 0;		//Set to 0 before calculating checksum
    iph->saddr = inet_addr ( source_ip );	//Spoof the source ip address
    iph->daddr = si_other.sin_addr.s_addr;
	
    // Ip checksum
    iph->check = Checksum ((unsigned short *) datagram, iph->tot_len);
	
    udph->source_port = htons (3232);
    udph->dest_port = htons (1235);
    udph->segment_length = htons(sizeof(struct udp_segment));	//tcp header size
    udph->checksum = 0;	//leave checksum 0 now, filled later by pseudo header
	

    // Now the UDP checksum using the pseudo header
    psh.source_address = inet_addr( source_ip );
    psh.dest_address = si_other.sin_addr.s_addr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = htons(sizeof(struct udp_segment));
	
    int psize = sizeof(struct pseudo_header) + sizeof(struct udp_segment);
    pseudogram = malloc(psize);
	
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , udph , sizeof(struct udp_segment));
	
    udph->checksum = Checksum( (unsigned short*) pseudogram , psize);

    if (sendto (sockfd, datagram, iph->tot_len ,0, (struct sockaddr *) &si_other, sizeof (sin)) < 0) {
   	
	 perror("sendto failed");
    }
     // send the data
    else {
	//printf ("Packet Send. Length : %d \n" , iph->tot_len);
        printf("Port: %d\n", udphdr->dest);
        printf("Filename: input.txt\n");
        printf("Source Port: %d\n", udphdr->source);
        printf("Length: %d\n", udphdr->len);
        printf("Checksum: %d (%x)\n", udphdr->check, udphdr->check);
        printf("Payload: %d\n", strlen(data));
        printf("Client.log written\n");

    }
	// then open a file log and write to that 
        if (fp = fopen("Client.log", "w")) {
           fprintf(fp, "Port: %d\n", PORT);
           fprintf(fp, "Filename: input.txt\n");
           fprintf(fp, "Source Port: %d\n", ntohs(udphdr->source));
           fprintf(fp, "Destination Port: %d\n", ntohs(udphdr->dest));
           fprintf(fp, "Length: %d\n", ntohs(udphdr->len));
           fprintf(fp, "Checksum: %d (%x)\n", udphdr->check, udphdr->check);
           fprintf(fp, "Payload: %d bytes\n", strlen(data));
           fprintf(fp, "Client.log written\n");
           fclose (fp);
        }

    close(sockfd);
    return 0;
}

//checksum function that kinda works
unsigned short Checksum(unsigned short *ptr,int nbytes) 
{
	register long sum;
	unsigned short oddbyte;
	register short answer;

	sum=0;
	while(nbytes>1) {
		sum+=*ptr++;
		nbytes-=2;
	}
	if(nbytes==1) {
		oddbyte=0;
		*((u_char*)&oddbyte)=*(u_char*)ptr;
		sum+=oddbyte;
	}

	sum = (sum>>16)+(sum & 0xffff);
	sum = sum + (sum>>16);
	answer=(short)~sum;
	
	return(answer);
}
