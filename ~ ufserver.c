/*
    Simple udp server
*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include<netinet/ip.h>  //for the ip header
#include<netinet/udp.h> //udp header

 
#define BUFLEN 512  //Max length of buffer
#define PORT 1235  //The port on which to listen for incoming data

//struct for the headers to be sent over/computed seperately 
struct pseudo_header
{
        u_int32_t source_address;
        u_int32_t dest_address;
        u_int8_t placeholder;
        u_int8_t protocol;
        u_int16_t udp_length;
};

//udp segment to send over. all the data that it will take 
struct udp_segment {
  unsigned short source_port;
  unsigned short dest_port;
  unsigned short segment_length;
  unsigned short checksum;
  unsigned char data[513];
};
unsigned short Checksum(unsigned short *ptr,int nbytes);

struct pseudo_header psh; // struct for pseudo header portoin 
 
void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int sockfd, i, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
    FILE *fp;

    //create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    //((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if(bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
        //printf("Waiting for data...\n");
		fflush(stdout);
        bzero (buf, BUFLEN);        
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(sockfd, (char *)buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
         
        buf[recv_len]= '\0';
       //header for ip
       struct iphdr *iph = (struct iphdr *) buf;
       
       //header for udp 
       struct udp_segment *udph = (struct udp_segment *) (buf + sizeof (struct ip)); 
       struct udphdr* udphdr = (struct udphdr *) (buf + sizeof (struct ip)); 
       char* data = buf + sizeof(struct iphdr) + sizeof(struct udphdr); 

int psize = sizeof(struct pseudo_header) + sizeof(struct udp_segment);
char* pseudogram;
// check the checksum using the pseudo header for udp 
    
    psh.source_address = iph->daddr;
    psh.dest_address = iph->saddr;
    psh.placeholder = 0;
    psh.protocol = IPPROTO_UDP;
    psh.udp_length = sizeof(struct udp_segment);
    pseudogram = malloc(psize);
    memcpy(pseudogram , (char*) &psh , sizeof (struct pseudo_header));
    memcpy(pseudogram + sizeof(struct pseudo_header) , (char*)udph , sizeof(struct udp_segment));

       unsigned short received_csum = udph->checksum;
       
 
        //print details of the client/peer and the data received
        printf("Port: %d\n", udphdr->dest);
        printf("Source Port: %d\n", udphdr->source);
        printf("Length: %d\n", udphdr->len); 
        printf("Checksum: %d (%x)\n", udphdr->check, udphdr->check); 
        printf("Payload: %d\n", strlen(data)); 
        printf("Computed Checksum: %d (%x)\n",received_csum, received_csum); 
        printf("Output.txt written\n");
        printf("Server.log written\n");
        
	//printing and writing to the log files from the server side 		
	if (fp = fopen("Server.log", "w")) {
           fprintf(fp, "Port: %d\n", udphdr->dest);
           fprintf(fp, "Source Port: %d\n", udphdr->source);
           fprintf(fp, "Length: %d\n", udphdr->len);
           fprintf(fp, "Checksum: %d (%x)\n", udphdr->check, udphdr->check);
           fprintf(fp, "Payload: %d\n", strlen(data));
           fprintf(fp, "Computed Checksum: %d (%x)\n",received_csum, received_csum);
           if (received_csum != udphdr->check)
              fprintf(fp, "Checksum mismatch!\n");
           else
              fprintf(fp, "Checksum Matched!\n");
           fprintf(fp, "Output.txt written\n");
           fprintf(fp, "Server.log written\n");
           fclose (fp);
        }



	bzero(buf, sizeof(buf));
    }
 
    close(sockfd);
    return 0;
}

//checksum calculation. got help from stackoverflow to help find checksum
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
