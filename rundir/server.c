#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

FILE *dbg;
int sockfd, visItsockfd[1024], address_len;
socklen_t visIt_address_len;
struct sockaddr_in address, visIt_address;
char buf[512], visbuf[512];
char *buffer;
ssize_t readlen;
//static const int signum=SIGRTMIN+1; 
struct sigevent sigevent;   
int iter;

void handler(int signum)
{
	printf("Hi I am handler\n");

	if((readlen=(recv(visItsockfd[iter], buffer, 512, 0)))> 0) {
//		buf[readlen]='\0';
		printf("%s\n", buffer);
	}

}

/*
 * Main program
 * 
 * No Arguments
 * 
 * // * Visualization site address 
 *
 */
int main(int argc, char **argv)
{
	// ***************** Variable Declaration *****
	int flag=1, result;

	if(argc < 3)
	{
		printf("Missing argument \nUsage: ./server <number of clients> <report frequency>");
		exit(1);
	}

	int numps = atoi(argv[1]);
	int wait = atoi(argv[2]);

	// Create an endpoint for communication

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("server: socket error ");
		exit(1);
	}

	flag = 1;
  	result = setsockopt(sockfd,            /* socket affected */
                          IPPROTO_TCP,     /* set option at TCP level */
                          TCP_NODELAY,     /* name of option */
                          (char *) &flag,  /* the cast is historical 
                                                  cruft */
                          sizeof(int));    /* length of option value */

	//if (result < 0)
     	//... handle the error ...


	// Fill socket structure with host information

	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = htons(6666);		//6633 if hop
	address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &address, sizeof(address)) == -1) {
		perror("server: bind error ");
		exit(1);
	}

	if (listen(sockfd, 5) == -1) {
		perror("server: listen error ");
		exit(1);
	}
	
//	signal(SIGIO, handler);

	visIt_address_len = sizeof(visIt_address);
	buffer = (char *)malloc(512*sizeof(char));
	

	for (iter=0;iter<numps;iter++) {
	
		printf("waiting %d\n", iter);

		if ((visItsockfd[iter] = accept(sockfd, (struct sockaddr *) &visIt_address, &visIt_address_len)) == -1) {
			perror("server: accept error ");
			exit(1);
		}

		printf("client connected %d\n", visItsockfd[iter]);

	}

/*		if((readlen=(recv(visItsockfd[iter], buffer, 512, 0)))> 0) 
			printf("%s\n", buffer);
		if (readlen < 0)
			perror("Server recv error");
*/
		//sleep(180);

	const char *string[] = {"Hello", "I will", "send", "now", "Byebye"};
	int i=-1, numbytes;

	while(1) {
	 
	  i++;

	  for (iter=0;iter<numps;iter++) {

		sleep(5);
		printf("\nSending %s to %d\n", string[i], visItsockfd[iter]);
		strcpy(buffer, string[i]);
		int len = strlen(buffer);
		buffer[len]='%';	//\n';
		//if((numbytes = write(visItsockfd[iter], buffer, len+1)) <= 0)
		if((numbytes = send(visItsockfd[iter], buffer, len+1, 0)) <= 0)
			perror("Server send error");
		
		printf("\nSent %d %d bytes\n", len, numbytes);

	  }

	  sleep(wait);

	  if(strncmp(buffer, "Byebye", 6) == 0) 
		  break;
	}

	// Close connection
	close(sockfd);
	for (iter=0;iter<numps;iter++) 
		close(visItsockfd[iter]);

	return 0;

}
