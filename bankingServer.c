#include <netdb.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h> 
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include "bankingServer.h"

#define MAX 255 
#define PORT 8080
#define SA struct sockaddr 
#define size 80000
#define INTERVAL 15000
#define DEBUG 1

pthread_mutex_t mutex;
sem_t sem;




account ** bank; 
printOut * allAccounts;
int signa;
int sock1;
int sock2;


void signal_handler(int sig){
	printf("\nCtrl+C detected. Server will now free everything and shut down.\n");
	freeBank();
	freePrintOuts();
	close(sock1);
	close(sock2);
	exit(1);
	
}
unsigned long hash(unsigned char *str){
        unsigned long hash = 5381;
        int c;

        while (c = *str++){
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
	}
	hash = hash%size;
        return hash;
} 
void create(char* string,int sockfd,session* ses){
	int i = (int) hash(string);
	char* buff = (char*)malloc(sizeof(char)*255);
	sem_wait(&sem);
	if(ses->isLogged == 1){
		buff = "User still in session. Please log out first to create a new account.\n";
		write(sockfd,buff,strlen(buff));
		sem_post(&sem);
		return ;
	}
	if(bank[i]->name != NULL && strcmp(bank[i]->name, string) == 0){
		//account exists
		strcpy(buff, "Account ");
		strcat(buff,string);
		strcat(buff," already exists. Please choose another\n");

		write(sockfd, buff, strlen(buff)); 
		sem_post(&sem);
	
	}else{
		
		//account does not exist, create a new one
		strcpy(bank[i]->name, string);
		strcpy(buff, "Account ");
		strcat(buff,string);
		strcat(buff," successfully created!\n");

		write(sockfd,buff,strlen(buff));
		allAccounts->array[allAccounts->index] =  i;
		allAccounts->index++;
		sem_post(&sem);
				
	}
	//sem_post(&sem);

}
session* serve(char* string, int sockfd, session* ses){
	char* buff = (char*)malloc(sizeof(char)*255);
	sem_wait(&sem);
	if(ses->isLogged == 1){
		buff = "User still in session. Please log out first.\n";
		write(sockfd,buff,strlen(buff));
		sem_post(&sem);
		return ses;
	}	
	int i = (int) hash(string);
	if(bank[i]->session == 1){
		//account is already logged in
		strcpy(buff, "Account ");
		strcat(buff,string);
		strcat(buff," is already logged in elsewhere. Please log out first.\n");
		write(sockfd, buff, strlen(buff));
		sem_post(&sem); 
		return ses;
	}
	
	if(strcmp(bank[i]->name, string) == 0){
		//account exists
		strcpy(ses->name, string);
		ses->isLogged = 1;
		bank[i]->session  = 1;
		strcpy(buff, "Successfully log into account ");
		strcat(buff, string);
		strcat(buff,".\n");
		write(sockfd,buff,strlen(buff));
		sem_post(&sem);
		return ses;
	}else{
		//acount does not exists
		strcpy(buff , "Account ");
		strcat(buff, string);
		strcat(buff," does not exist.\n");
		write(sockfd,buff,strlen(buff));
		sem_post(&sem);
		return ses;
	}
	sem_post(&sem);

}

void deposit(session* ses, float amount, int sockfd){
	sem_wait(&sem); 	
	char* string = ses->name;	
	int i = (int)hash(string);
	bank[i]->balance += amount;
	char* buff = (char*)malloc(sizeof(char)*255);
	strcpy(buff,"Account ");
	strcat(buff,string);
	strcat(buff,"|| Deposited: ");
	strcat(buff,floatToString(amount));
	strcat(buff,"|| New Balance: ");
	strcat(buff,floatToString(bank[i]->balance));
	strcat(buff,"\n");
	write(sockfd,buff,strlen(buff));
	sem_post(&sem);

}

void withdrawl(session* ses, float amount, int sockfd){
	sem_wait(&sem);	
	char* string = ses->name;	
	int i = (int)hash(ses->name);
	float bal = bank[i]->balance;
	char* buff = (char*)malloc(sizeof(char)*255);
	if(bal < amount){
		strcpy(buff,"Account ");
		strcat(buff,string);	
		strcat(buff,"||Insufficent funds||Attempted Withdrawl: ");
		strcat(buff,floatToString(amount));
		strcat(buff,"||Balance: ");
		strcat(buff,floatToString(bal));
		strcat(buff,"\n\0");
		write(sockfd,buff,strlen(buff));
	}else{
		strcpy(buff,"Account ");
		strcat(buff,string);
		strcat(buff,"|| Old Balance: ");
		strcat(buff,floatToString(bal));
		strcat(buff,"|| New Balance: ");
		bank[i] -> balance = bank[i]->balance - amount;
		strcat(buff,floatToString(bank[i]->balance));
		strcat(buff,"\n\0");
		write(sockfd,buff,strlen(buff));
		
	}
	sem_post(&sem);
}

void query(session* ses, int sockfd){
	sem_wait(&sem);	
	int i = (int)hash(ses->name);
	char* string = bank[i]->name;
	float bal = bank[i]->balance;	
	char* buff = (char*)malloc(sizeof(char)*255);
	strcpy(buff,"Account ");
	strcat(buff,string);
	strcat(buff,"|| Current Balance: ");
	strcat(buff,floatToString(bal));
	strcat(buff,"\n");
	write(sockfd,buff,strlen(buff));
	sem_post(&sem);
}


session* check(char* string,int sockfd,session* ses,int client){
	char* tok  = strtok(string, " \n");
	char* buff;
	if(tok == NULL){
		char buff2[38]="Incorrect command, please try again\n";
		write(sockfd,buff2,sizeof(buff2));
		bzero(buff2,38);
		return ses;

	}
	if(signa == 1){
		printf("is pressed\n");
		char* buff2 = (char*)malloc(sizeof(char)*100);
		buff2="Disconnected from server.\n\0";
		write(sockfd,buff2,strlen(buff2));
		return;
	} 	
	if(strcmp(tok, "create") == 0){
		if(ses->isLogged == 1){
			buff = "Still in service session. Please log out first through \"end\"\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}
		tok = strtok(NULL,"\n");
		
		if(tok ==NULL){
			buff="Incorrect create command, please try again\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}
		char* p = (char*)malloc(sizeof(char)*255);
		strcpy(p,tok);
		//printf("case create with name |%s|\n",p);
		//goes to create function
		create(p,sockfd,ses);
		return ses;
	}else if(strcmp(tok, "serve") == 0){
		tok = strtok(NULL,"\n");
		if(tok ==NULL){
			buff="Incorrect serve command, please try again\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}
		return serve(tok, sockfd, ses);
		
	}else if(strcmp(tok, "deposit") == 0){
		if(ses->isLogged == 0){
			buff = "Invalid deposit command. Not in service sesson\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}		
		tok = strtok(NULL,"");
		if(isFloat(tok) == 0){
			buff = "Invaild deposit value\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}else{
			//goes to deposit function
			float f = atof(tok);
			deposit(ses,f, sockfd);
			return ses;

		}
		
	}else if(strcmp(tok, "withdraw") == 0){
		if(ses->isLogged == 0){
			buff = "Invalid withdraw operation. Not in service sesson\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}
		tok = strtok(NULL,"");
		if(isFloat(tok) == 0){
			buff = "Invaild withdraw value\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}else{
			//goes to withdraw function
			float f = atof(tok);
			withdrawl(ses, f,  sockfd);
			return ses;
		}
		
	}else if(strcmp(tok, "query") == 0){
		if(ses->isLogged == 0){
			buff = "Can not query. Not logged in.\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}		

		query(ses,sockfd);
		return ses;
	}else if(strcmp(tok, "end") == 0){
		tok = strtok(NULL,"");
		if(tok != NULL){
			buff = "Invaild end command.\n";
			write(sockfd,buff,strlen(buff));
			return ses;

		}
		if(ses->isLogged ==0){
			buff = "Invaild end command. Not in service session. Please enter just \"end\"\n";
			write(sockfd,buff,strlen(buff));
			return ses;
		}
		if(ses->isLogged == 1){
			int i = (int)hash(ses->name);
			bank[i]->session = 0;
		}		
		ses->isLogged = 0;
		buff = "Service session ended.\n\0";
		write(sockfd,buff,strlen(buff));
		return ses;
	}else if(strcmp(tok, "quit") == 0 ){
		tok = strtok(NULL,"");
		if(tok != NULL){
			buff = "Invaild quit command. Please enter just \"quit\"\n";
			write(sockfd,buff,strlen(buff));
			return ses;

		}
		if(ses->isLogged = 1){
			int i = (int)hash(ses->name);
			bank[i]->session = 0;
		}
				
		ses->isLogged = -1;
		printf("Client %d has disconnected.\n",client);
		char buff2[26] = "Disconnected from server.\n";
		write(sockfd,buff2,sizeof(buff2));
		bzero(buff2,26);
		return ses;
	}else{
		char buff2[38]="Incorrect command, please try again\n";
		write(sockfd,buff2,sizeof(buff2));
		bzero(buff2,38);
		return ses;
	}

}



void *func(void* p){ 
	

    t_t* temp = (t_t*)p;
    session* ses = (session*)malloc(sizeof(session));
    ses->name = (char*)malloc(sizeof(char)*255);
    ses->isLogged = 0;
    int sockfd = temp->sockfd;
    int client = temp->client;
    char buff[MAX]; 
    int n; 
	
    // infinite loop for chat 
    for (;;) { 
        bzero(buff, MAX); 
  	//printf("singla2: %d\n",signa);
	
        read(sockfd, buff, sizeof(buff)); 
        // print buffer which contains the client contents 
        //printf("From client: %s\t ", buff);
	
	ses = check(buff,sockfd,ses,client);
        bzero(buff, MAX); 
        n = 0; 
 
	if(ses->isLogged == -1){
		
		return;
	}
    
        //if (strncmp("exit", buff, 4) == 0) { 
          //  printf("Server Exit...\n"); 
            //break; 
        //} 
    } 
} 


void server_output() {
    sem_wait(&sem);
    printEveryone();
    sem_post(&sem);
}


int main(int argc, char *argv[])
{
    sem_init(&sem, 0 ,1);
     struct itimerval it_val;   

     signal(SIGINT,signal_handler);
    signa = 0;
   
    setBank();
    setPrintOuts();
    int sockfd, newsockfd, portno,n;
    int *newsocket;
    socklen_t clilen;     
    char line[256];
    char *data;
    char operation;
    struct sockaddr_in serv_addr, cli_addr;
    pthread_mutex_init(&mutex,NULL);
    FILE *fp=NULL;
    int i=0,clientcount=0;

    // argument count check
    //if (argc < 2) {
      // printf("Error: Less no. of arguments");
       //exit(1);
    //}


      //Server first creates socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    //Check if error in socket creation
    if (sockfd < 0) 
        printf("Error:Creating socket at server");

    bzero((char *) &serv_addr, sizeof(serv_addr));
    //Host to network byte updater
    sock1 = sockfd;
    portno = atoi(argv[1]);
    //portno = 8080;
    if(portno == 0 || portno < 8000){
	printf("Invalide port number. Please choose one over 8000\n");
	return;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    //Bind the socket to port
    if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        printf("Error:in binding socket\n");

	  if (signal(SIGALRM, (void (*)(int)) server_output) == SIG_ERR) {
        perror("Unable to catch SIGALRM");
        exit(1);
    }
    it_val.it_value.tv_sec = INTERVAL/1000;
    it_val.it_value.tv_usec = (INTERVAL*1000) % 1000000;   
    it_val.it_interval = it_val.it_value;
    if (setitimer(ITIMER_REAL, &it_val, NULL) == -1) {
        perror("error calling setitimer()");
        exit(1);
    }
     printf("Server Successfully Started!\n");

    listen(sockfd,5);

  
    clilen = sizeof(cli_addr);
 
    while(newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen))
    {
	
        clientcount++;
            printf("Client %d has connected.\n",clientcount);
        if (newsockfd < 0) 
            printf("Error: in accept");
        
        newsocket = (int *)malloc(1); 
	sock2 = newsockfd; 
	t_t* p = (t_t*)malloc(sizeof(t_t));
	p->client = clientcount;
	p->sockfd = newsockfd;
        *newsocket = newsockfd;
        pthread_t pid;
        if(pthread_create(&pid,NULL,func,(void*)p) < 0)   
        {
            printf("Error in creating new thread");
            return ;
        }  
	
	
	
        
    }
    close(newsockfd);
    close(sockfd);
	printf("closing down server.\n");
    return 0; 
}

void setBank(){
	bank = (account**)malloc(sizeof(account*)*size);
	int i;
	for(i =0; i< size; i++){
		bank[i] = (account*)malloc(sizeof(account));
		bank[i]->name = (char*)malloc(sizeof(char)*255);
		bank[i]->balance = 0;
		bank[i]->session = 0;
	}
}

void freeBank(){
	int i;
	for(i = 0; i < size; i++){
		free(bank[i]->name);
		free(bank[i]);
	}
	free(bank);
}
void setPrintOuts(){
	allAccounts = (printOut*)malloc(sizeof(printOut));
	allAccounts-> array = (int*)malloc(sizeof(int)*size);
	allAccounts-> index = 0;
	
}
void freePrintOuts(){
	free(allAccounts -> array);
	free(allAccounts);
}
char* floatToString(float f){
	char* re = (char*)malloc(sizeof(char)*100);
	snprintf(re, sizeof(re), "%.2f", f);
	return re;

}

int isFloat(char *str) {
	int len;
	float ignore;
	int ret = sscanf(str, "%f %n", &ignore, &len);
	return ret && len==strlen(str);
}
void printEveryone(){
	int i;
	printf("\n========DIAGNOSTIC START========\n");
	for(i = 0; i < allAccounts->index; i++){
		int j = allAccounts->array[i];
		printf("account: |%s|\t balance: %.2f\t ",bank[j]->name,bank[j]->balance);
		if(bank[j]->session == 1){
			printf("IN SERVICE\n");
		}else{
			printf("\n");
		}
	}
	printf("========DIAGNOSTIC END==========\n\n");

}
