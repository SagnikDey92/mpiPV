#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <signal.h>
#include <mqueue.h>
#include <pthread.h>

#define PORTNUM 6666

int sockfd, client_sockfd;
socklen_t client_address_len;
struct sockaddr_in server_address, client_address;

void *initConn(void *); //void initConn();
void handler(int);

char *serverIP = "127.0.0.1";	//10.192.13.35";

struct sigevent sigevent = {0};        // For notification
mqd_t rmsgq_id;

struct pirate_msgbuf {
    long mtype;  /* must be positive */
    struct pirate_info {
        char name[30];
        char ship_type;
        int notoriety;
        int cruelty;
        int booty_value;
    } info;
};

struct pirate_msgbuf pmb;

int msqid;

//Socket connection             [with simd]

int sockfd, address_len, result;
struct sockaddr_in address;
struct hostent *hostname;

const int PORT=6666;
pthread_t thr;
int msgsize = 2048;

mpiPi_t mpiPi_local;
static int dumpCount = 0;
