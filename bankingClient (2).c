#include <netdb.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h> 
#include <sys/time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "bankingClient.h"
#define MAX 255 
//#define PORT 8080
#define SA struct sockaddr 
#define INTERVAL 1000

int signa;

int sockfd2;

void signal_handler(int sig){
	printf("\nClient interrupt detected. Client will now disconnect.\n");
	char* buff3 = (char*)malloc(sizeof(char)*100);
	buff3 = "quit";
	write(sockfd2, buff3, strlen(buff3));
	char buff2[MAX];
	read(sockfd2, buff2, sizeof(buff2)); 
	close(sockfd2);
	exit(1);
 
}

void timer_f(){
	
	/*int keepcnt = 5;
int keepidle = 30;
int keepintvl = 120;

setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(int));
setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(int));
setsockopt(sockfd, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(int));*/
	int error = 0;
		socklen_t len = sizeof(error);
		int retval = getsockopt(sockfd2,SOL_SOCKET,SO_ERROR,&error,&len);


	if(sockfd2 == -1){
		return;
	}
		//printf("apple: %d\n",error);
		if(error !=0){		
			printf("\nServer shut down. This client will now exit.\n");
			close(sockfd2);
			exit(1);
		}
		char* buff3 = (char*)malloc(sizeof(char)*100);
		buff3 = "-1";
		write(sockfd2, buff3, strlen(buff3)); 
		char buff2[MAX];
		read(sockfd2, buff2, sizeof(buff2)); 
		
	
}


void func(int sockfd){ 

     sockfd2 = sockfd;
     struct itimerval it_val;
     it_val.it_value.tv_sec = INTERVAL/1000;
    it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;   
    it_val.it_interval = it_val.it_value;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }	
     signal(SIGALRM, (void (*)(int)) timer_f);
    char buff2[MAX];
	char buff[MAX];
	 
    int n; 
    for (;;) { 
        bzero(buff, sizeof(buff)); 
	 bzero(buff2, sizeof(buff2)); 
	if(signa == 0){
		printf("To Server : "); 
		n = 0; 
		while ((buff[n++] = getchar()) != '\n') 
		    ; 
		
		write(sockfd, buff, sizeof(buff)); 
		bzero(buff, sizeof(buff)); 
	}
        read(sockfd, buff, sizeof(buff)); 
	
        printf("From Server : %s", buff,sockfd); 
	
	if(strcmp(buff,"Disconnected from server.\n") == 0){
		printf("Exiting now..\n");
		break;
	} 
		sleep(2);
       
	
	//if(strcmp(buff,"Disconnected from server.\n") == 0){
	//	printf("Exiting now..\n");
	//	break;
	//} 
    } 
} 


int main(int argc, char *argv[]) 
{ 
	if(argc != 3){
	printf("Invalid parameters\n");
	return 0;
	}	
     int PORT = atoi(argv[2]);
    sockfd2 = -1;
	char* ADDRESS =argv[1];
	//printf("addresS |%s|\n",ADDRESS);
	//strcpy(ADDRESS,argv[1]);
   if(PORT < 8000){
	printf("Invalid port number. Please choose one over 8000\n");
	return 0;
     }
    int sockfd, connfd; 
	signa = 0;
    struct sockaddr_in servaddr, cli; 

    ///char* ip = inet_addr("localhost");
	

    // socket create and varification 
    sockfd = socket(AF_INET, SOCK_STREAM, 0); 
    if (sockfd == -1) { 
        printf("socket creation failed...\n"); 
        exit(0); 
    } 
    \
    bzero(&servaddr, sizeof(servaddr)); 
      
     
   
    // assign IP, PORT 
    servaddr.sin_family = AF_INET; 
   //in_addr_t a = inet_addr(ADDRESS);
	//if(a == -1){
	//	printf("Invalid machine name. Please choose another one and try again\n");
	//	return 0;
	//}
	inet_pton(AF_INET,ADDRESS,&(servaddr.sin_addr.s_addr));
   // servaddr.sin_addr.s_addr = inet_addr("128.6.13.188"); 
    servaddr.sin_port = htons(PORT); 
     
    
    // connect the client socket to server socket 
    while (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) { 
        printf("Connection to server failed. Trying again in 3 seconds...\n"); 
        sleep(3);
    } 
    signal(SIGINT,signal_handler);
      printf("Connected to the server!\n"); 
  	
    // function for chat 
    func(sockfd); 
  
    // close the socket 
    close(sockfd); 
} 
