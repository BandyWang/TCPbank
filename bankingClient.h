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

#define MAX 255 
//#define PORT 8080
#define SA struct sockaddr 
#define INTERVAL 1000

void timer_f();
void func(int sockfd);
