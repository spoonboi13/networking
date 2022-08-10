#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
int main()
{

	int sockfd, userinput; //socket descriptor, userinput for port #
	char recvline[4096]; //input for url 
	struct sockaddr_in servaddr; //server address in struct 
	char buffer[BUFFER_SIZE] = {0}; //making buffer clear/empty
	printf("SMTP Server: ");
	fgets(recvline, 100, stdin); //gets user input of email server 

	recvline[strlen(recvline) -1] = '\0'; 

	printf("Port: ");
	scanf("%d", &userinput); // takes in port number 

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0 ){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	//else 
	//	printf("socket successfully created..\n"); 

	struct hostent *hostname; //get hostname
	if((hostname = gethostbyname(recvline)) == NULL){
		herror("gethostbyname(): ");
		exit(1);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	servaddr.sin_family = AF_INET; //af_inet family 
	servaddr.sin_port = htons(2525); //port number for smtp2go
	memcpy(&servaddr.sin_addr.s_addr,hostname->h_addr, hostname->h_length); // to get address as hostname 

	//connect to server
	if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
		printf("error connecting\n");
		exit(EXIT_FAILURE);

	}
	//else
	//	printf("success connecting!\n");
	
	char servmess[] = "telnet mail.smtp2go.com 2525"; //message sent to server to connect with telnet 
	if(recv(sockfd, servmess,BUFFER_SIZE , 0) < 0)
	{
		puts("send failed");
		exit(1);
	}
	//puts("reply recieved\n");

	memset(&servmess, sizeof(servmess), 0);
	printf("Response: ");
	servmess[strlen(servmess) -1] = '\0'; //clears it out 
	puts(servmess); //prints servmessage to terminal
	bzero(buffer, BUFFER_SIZE);
	

	char hello[] = "EHLO\n"; // string ehlo 
	write(sockfd, hello, strlen(hello)); //writes ehlo to server
	sleep(1); //client sleeps 1 sec
	recv(sockfd, hello, BUFFER_SIZE, 0); //reads response
	puts(hello); // prints response from server	
	bzero(buffer, sizeof(buffer)); 

	char authlogin[] = "AUTH LOGIN\n"; //authenticatin login string
        char authlogin_buffer[20];
	write(sockfd, authlogin, strlen(authlogin)); //writes to server 
	//sleep(1); 	
	printf("Response: ");
	bzero(authlogin_buffer,sizeof(authlogin_buffer));	
	int n = recv(sockfd,authlogin_buffer,sizeof(authlogin_buffer), 0);//replaced srrlen(authlogin) with buffersize
	
	if(n){
		puts(authlogin_buffer);
	}
	bzero(authlogin_buffer, sizeof(authlogin_buffer));

	char username[]= "\n"; // making username empty
	printf("Username: ");
	scanf("%s",username);	
	strcat(username, "\n"); 
	char username_buffer[20]; 	
	bzero(username_buffer, sizeof(username_buffer)); 
        write(sockfd, username, strlen(username)); //sending username to server 
	//sleep(1); 
	recv(sockfd, username_buffer, sizeof(username_buffer), 0);
	puts(username_buffer);	
	bzero(username_buffer,sizeof(username_buffer));
	bzero(buffer,sizeof(buffer));

	char password[] = "\n"; 
	printf("Password: ");
	scanf("%s",password);
	strcat(password,"\n");
	char password_buffer[30];
	bzero(password_buffer,sizeof(password_buffer));
	write(sockfd,password,strlen(password)); //sending password to server
	recv(sockfd,password_buffer, sizeof(password_buffer),0); //reading from server
	puts(password_buffer);
	bzero(password_buffer,sizeof(password_buffer));
	bzero(buffer,sizeof(buffer));
		
	char mailfrom[] = "\n"; // changed from MAIL FROM: \n
	printf("Senders email address: ");	
	scanf("%s",mailfrom);
	strcat(mailfrom,"\n");
	char mailfrom_buffer[30];
	bzero(mailfrom_buffer,sizeof(mailfrom_buffer));
	write(sockfd,"MAIL FROM:< >\n",strlen(mailfrom)); //changed to mailfrom instead of "MAIL FROM:\n"
	//sleep(1);
	printf("Response: ");
	int m = recv(sockfd, mailfrom_buffer, sizeof(mailfrom_buffer), 0);// changed mailfrom_buffer to mailfrom
	if(m){
	
		puts(mailfrom_buffer);
	}
	bzero(mailfrom_buffer,sizeof(mailfrom_buffer));
	bzero(buffer,BUFFER_SIZE);

	//char rcptto[] = "\n";
	char rcptto[70] = {};
	char rcptto_buffer[128];
	printf("Recipient’s email address: ");
	scanf("%s",rcptto);
	strcat(buffer, "RCPT TO: ");
	strcpy(buffer,rcptto);
	strcat(buffer,rcptto);
	bzero(rcptto_buffer,sizeof(rcptto_buffer));
	write(sockfd,buffer,strlen(buffer));// changed from rcptto
	//bzero(rcptto_buffer,sizeof(rcptto_buffer));
	bzero(buffer, sizeof(buffer));
	sleep(1);
	recv(sockfd,rcptto_buffer,sizeof(rcptto_buffer),0);
	// changed sizeofrcptobuffer to strlen
	puts(rcptto_buffer);
	bzero(rcptto_buffer,sizeof(rcptto_buffer));

	//DATA SECTION, couldn't compile

	//char data[] = "DATA\n";
        //char data_buffer[50];
        //write(sockfd, data, strlen(data)); //writes to server 
        //sleep(1);     
        //bzero(data_buffer,sizeof(data_buffer));
        //int d = recv(sockfd,data_buffer,sizeof(data_buffer), 0);

        //if(d){
          //      puts(data_buffer);
        //}
        //bzero(data_buffer, sizeof(data_buffer));
	
	//USERNAME SECTION FOR DATA, couldn't compile past rcpt to
	
	//char username2[]= "\n"; // making username empty
        //printf("Senders Name: ");
        //scanf("%s",username2);
        //strcat(username2, "\n");
        //char username2_buffer[20];
        //bzero(username2_buffer, sizeof(username2_buffer));
        //write(sockfd, "FROM: " "< >\n", strlen(username2)); //sending username to server 
        //sleep(1); 
        //recv(sockfd, username2_buffer, sizeof(username2_buffer), 0);
        //puts(username2_buffer);
        //bzero(username2_buffer,sizeof(username2_buffer));
        //bzero(buffer,sizeof(buffer));

	//RCPTS NAME, coudlnt compile

	////char rcptname[]= "\n"; // making rcptname empty
        //printf("Reciepients Name: ");
        //scanf("%s",rcptname);
        //strcat(rcptname, "\n");
        //char rcptname_buffer[20];
        //bzero(rcptname_buffer, sizeof(rcptname_buffer));
        //write(sockfd, "TO: " "< >\n", strlen(rcptname)); //sending username to server 
        //sleep(1); 
        //recv(sockfd, rcptname_buffer, sizeof(rcptname_buffer), 0);
        //puts(rcptname_buffer);
        //bzero(rcptname_buffer,sizeof(rcptname_buffer));
        //bzero(buffer,sizeof(buffer));

	//SUBJECT SECTION

	//char subject[] = "\n"; // making subject line
        //printf("Subject: ");
        //scanf("%s",subject);
        //strcat(subject,"\n");
        //char subject_buffer[30];
       // bzero(subject_buffer,sizeof(subject_buffer));
        //write(sockfd,"SUBJECT:"%s"\n",strlen(subject)); //changed to mailfrom instead of "MAIL FROM:\n"
        //sleep(1);
       // printf("Response: ");
        //int s = recv(sockfd, subject_buffer, sizeof(subject_buffer), 0);// changed mailfrom_buffer to mailfrom
        //if(s){

               // puts(subject_buffer);
        //}
        //bzero(subject_buffer,sizeof(subject_buffer));
        //bzero(buffer,BUFFER_SIZE);
}
