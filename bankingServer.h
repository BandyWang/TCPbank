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



typedef struct account{
	char* name;
	float balance;
	int session;
}account;
typedef struct printOut{
	int* array;
	int index;

}printOut;
typedef struct session{
	char* name;
	int isLogged;

}session;
typedef struct s_t{
	int fd;
	int sig;

}s_t;
typedef struct t_t{
	int sockfd;
	int client;
}t_t;

session* check(char* string,int sockfd,session* ses,int client);
session* serve(char* string, int sockfd, session* ses);
void server_output();
void *func(void* p);
void deposit(session* ses, float amount, int sockfd);
void query(session* ses, int sockfd);
void withdrawl(session* ses, float amount, int sockfd);
void create(char* string,int sockfd,session* ses);
void signal_handler(int sig);
void setBank();
void setPrintOuts();
void freeBank();
void freePrintOuts();
void printEveryone();
char* floatToString(float f);
int isFloat(char *str);
unsigned long hash(unsigned char *str);
