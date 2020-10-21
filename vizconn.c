#include <stdio.h>
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
#include <mpi.h>
#include "mpiPi.h"
#include "vizconn.h"

#define PORTNUM 6666

int sockfd, client_sockfd;
socklen_t client_address_len;
struct sockaddr_in server_address, client_address;

//void *initConn(void *); //void initConn();
void handler(int);

char *serverIP = "127.0.0.1";		//10.192.13.35";

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

int myrank;

mpiPi_t mpiPi_local;
static int dumpCount = 0;

static int
mpiPi_local_callsite_stats_pc_hashkey (const void *p)
{
  int res = 0;
  int i;
  callsite_stats_t *csp = (callsite_stats_t *) p;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp);
  for (i = 0; i < MPIP_CALLSITE_STACK_DEPTH; i++)
    {
      res ^= (unsigned) (long) csp->pc[i];
    }
  return 52271 ^ csp->op ^ res ^ csp->rank;
}

static int
mpiPi_local_callsite_stats_pc_comparator (const void *p1, const void *p2)
{
  int i;
  callsite_stats_t *csp_1 = (callsite_stats_t *) p1;
  callsite_stats_t *csp_2 = (callsite_stats_t *) p2;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_1);
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_2);

#define express(f) {if ((csp_1->f) > (csp_2->f)) {return 1;} if ((csp_1->f) < (csp_2->f)) {return -1;}}
  express (op);
  express (rank);

  for (i = 0; i < MPIP_CALLSITE_STACK_DEPTH; i++)
    {
      express (pc[i]);
    }
#undef express

  return 0;
}


typedef struct callsite_cache_entry_t
{
  void *pc;
  char *filename;
  char *functname;
  int line;
}
local_callsite_pc_cache_entry_t;

h_t *local_callsite_pc_cache = NULL;

static int
local_callsite_pc_cache_comparator (const void *p1, const void *p2)
{
  local_callsite_pc_cache_entry_t *cs1 = (local_callsite_pc_cache_entry_t *) p1;
  local_callsite_pc_cache_entry_t *cs2 = (local_callsite_pc_cache_entry_t *) p2;

  if ((long) cs1->pc > (long) cs2->pc)
    {
      return 1;
    }
  if ((long) cs1->pc < (long) cs2->pc)
    {
      return -1;
    }
  return 0;
}

static int
local_callsite_pc_cache_hashkey (const void *p1)
{
  local_callsite_pc_cache_entry_t *cs1 = (local_callsite_pc_cache_entry_t *) p1;
  return 662917 ^ ((long) cs1->pc);
}

static int
mpiPi_local_callsite_stats_src_hashkey (const void *p)
{
  int res = 0;
  callsite_stats_t *csp = (callsite_stats_t *) p;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp);
  return 52271 ^ csp->op ^ res ^ csp->rank ^ csp->csid;
}

static int
mpiPi_local_callsite_stats_src_comparator (const void *p1, const void *p2)
{
  callsite_stats_t *csp_1 = (callsite_stats_t *) p1;
  callsite_stats_t *csp_2 = (callsite_stats_t *) p2;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_1);
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_2);

#define express(f) {if ((csp_1->f) > (csp_2->f)) {return 1;} if ((csp_1->f) < (csp_2->f)) {return -1;}}
  express (op);
  express (csid);
  express (rank);
#undef express

  return 0;
}

static int
mpiPi_local_callsite_stats_src_id_hashkey (const void *p)
{
  int res = 0;
  callsite_stats_t *csp = (callsite_stats_t *) p;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp);
  return 52271 ^ csp->op ^ res ^ csp->csid;
}


static int
mpiPi_local_callsite_stats_src_id_comparator (const void *p1, const void *p2)
{
  callsite_stats_t *csp_1 = (callsite_stats_t *) p1;
  callsite_stats_t *csp_2 = (callsite_stats_t *) p2;
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_1);
  MPIP_CALLSITE_STATS_COOKIE_ASSERT (csp_2);

#define express(f) {if ((csp_1->f) > (csp_2->f)) {return 1;} if ((csp_1->f) < (csp_2->f)) {return -1;}}
  express (op);
  express (csid);
#undef express

  return 0;
}

h_t *local_callsite_src_id_cache = NULL;
int local_callsite_src_id_counter = 1;

static int
local_callsite_src_id_cache_comparator (const void *p1, const void *p2)
{
  int i;
  callsite_src_id_cache_entry_t *csp_1 = (callsite_src_id_cache_entry_t *) p1;
  callsite_src_id_cache_entry_t *csp_2 = (callsite_src_id_cache_entry_t *) p2;

#define express(f) {if ((csp_1->f) > (csp_2->f)) {return 1;} if ((csp_1->f) < (csp_2->f)) {return -1;}}
  if (mpiPi.stackDepth == 0)
    {
      express (id);		/* In cases where the call stack depth is 0, the only unique info may be the id */
      return 0;
    }
  else
    {
      for (i = 0; i < MPIP_CALLSITE_STACK_DEPTH; i++)
	{
	  if (csp_1->filename[i] != NULL && csp_2->filename[i] != NULL)
	    {
	      if (strcmp (csp_1->filename[i], csp_2->filename[i]) > 0)
		{
		  return 1;
		}
	      if (strcmp (csp_1->filename[i], csp_2->filename[i]) < 0)
		{
		  return -1;
		}
	      express (line[i]);
	      if (strcmp (csp_1->functname[i], csp_2->functname[i]) > 0)
		{
		  return 1;
		}
	      if (strcmp (csp_1->functname[i], csp_2->functname[i]) < 0)
		{
		  return -1;
		}
	    }

	  express (pc[i]);
	}
    }
#undef express
  return 0;
}

static int
local_callsite_src_id_cache_hashkey (const void *p1)
{
  int i, j;
  int res = 0;
  callsite_src_id_cache_entry_t *cs1 = (callsite_src_id_cache_entry_t *) p1;
  for (i = 0; i < MPIP_CALLSITE_STACK_DEPTH; i++)
    {
      if (cs1->filename[i] != NULL)
	{
	  for (j = 0; cs1->filename[i][j] != '\0'; j++)
	    {
	      res ^= (unsigned) cs1->filename[i][j];
	    }
	  for (j = 0; cs1->functname[i][j] != '\0'; j++)
	    {
	      res ^= (unsigned) cs1->functname[i][j];
	    }
	}
      res ^= cs1->line[i];
    }
  return 662917 ^ res;
}




void mpiPi_local_collect_basics(int report_style) {

  printf("\n%d: Begin of collect_basics\n", mpiPi_local.rank);

 if (mpiPi_local.rank == mpiPi_local.collectorRank)
    {
      /* In the case where multiple reports are generated per run,
         only allocate memory for global_task_info once */
      if (mpiPi_local.global_task_app_time == NULL)
	{
	  mpiPi_local.global_task_app_time =
	    (double *) calloc (mpiPi_local.size, sizeof (double));

	  if (mpiPi_local.global_task_app_time == NULL)
	    mpiPi_abort
	      ("Failed to allocate memory for global_task_app_time");

	  mpiPi_msg_debug
	    ("MEMORY : Allocated for global_task_app_time :          %13ld\n",
	     mpiPi_local.size * sizeof (double));
	}

      bzero (mpiPi_local.global_task_app_time, mpiPi_local.size * sizeof (double));

      if (mpiPi_local.global_task_mpi_time == NULL)
	{
	  mpiPi_local.global_task_mpi_time =
	    (double *) calloc (mpiPi_local.size, sizeof (double));

	  if (mpiPi_local.global_task_mpi_time == NULL)
	    mpiPi_abort
	      ("Failed to allocate memory for global_task_mpi_time");

	  mpiPi_msg_debug
	    ("MEMORY : Allocated for global_task_mpi_time :          %13ld\n",
	     mpiPi_local.size * sizeof (double));
	}

      bzero (mpiPi_local.global_task_mpi_time, mpiPi_local.size * sizeof (double));

      //  Only allocate hostname storage if we are doing a verbose report
      if (mpiPi_local.global_task_hostnames == NULL
	  && (report_style == mpiPi_style_verbose
	      || report_style == mpiPi_style_both))
	{
	  mpiPi_local.global_task_hostnames =
	    (mpiPi_hostname_t *) calloc (mpiPi.size,
					 sizeof (char) *
					 MPIPI_HOSTNAME_LEN_MAX);

	  if (mpiPi_local.global_task_hostnames == NULL)
	    mpiPi_abort
	      ("Failed to allocate memory for global_task_hostnames");

	  mpiPi_msg_debug
	    ("MEMORY : Allocated for global_task_hostnames :          %13ld\n",
	     mpiPi.size * sizeof (char) * MPIPI_HOSTNAME_LEN_MAX);
	}

      if (mpiPi_local.global_task_hostnames != NULL)
	bzero (mpiPi_local.global_task_hostnames,
	       mpiPi.size * sizeof (char) * MPIPI_HOSTNAME_LEN_MAX);
    }

  PMPI_Gather (&mpiPi_local.cumulativeTime, 1, MPI_DOUBLE,
	       mpiPi.global_task_app_time, 1, MPI_DOUBLE,
	       mpiPi.collectorRank, mpiPi.comm);

  if (report_style == mpiPi_style_verbose || report_style == mpiPi_style_both)
    {
      PMPI_Gather (mpiPi_local.hostname, MPIPI_HOSTNAME_LEN_MAX, MPI_CHAR,
		   mpiPi.global_task_hostnames, MPIPI_HOSTNAME_LEN_MAX,
		   MPI_CHAR, mpiPi.collectorRank, mpiPi.comm);
    }


  printf("\n%d: End of collect_basics\n", mpiPi_local.rank);
}


int
mpiPi_local_mergeResults () {

	int ac;
	callsite_stats_t **av;
	int totalCount = 0;
	int maxCount = 0;
	int retval = 1, sendval;
	
  printf("\n%d Begin mergeResults\n", mpiPi.rank);
	h_gather_data (mpiPi.task_callsite_stats, &ac, (void ***) &av);
  printf("\n%d After mergeResults:h_gather_data\n", mpiPi.rank);

	/* determine size of space necessary on collector */
  	PMPI_Allreduce (&ac, &totalCount, 1, MPI_INT, MPI_SUM, mpiPi.comm);
  printf("\n%d After Allreduce\n", mpiPi.rank);
  	PMPI_Reduce (&ac, &maxCount, 1, MPI_INT, MPI_MAX, mpiPi.collectorRank, mpiPi.comm);	
	
  printf("\n%d After reduce\n", mpiPi.rank);

	/* gather global data at collector */
	if (mpiPi.rank == mpiPi.collectorRank) {
		int i;
		int ndx = 0;

		/* Open call site hash tables.  */
		mpiPi_local.global_callsite_stats = h_open (mpiPi_local.tableSize,
				mpiPi_local_callsite_stats_src_hashkey,
				mpiPi_local_callsite_stats_src_comparator);
		mpiPi_local.global_callsite_stats_agg = h_open (mpiPi_local.tableSize,
				mpiPi_local_callsite_stats_src_id_hashkey,
				mpiPi_local_callsite_stats_src_id_comparator);
		if (local_callsite_pc_cache == NULL)
		{
			local_callsite_pc_cache = h_open (mpiPi_local.tableSize,
					local_callsite_pc_cache_hashkey,
					local_callsite_pc_cache_comparator);
		}
		if (callsite_src_id_cache == NULL)
		{
			callsite_src_id_cache = h_open (mpiPi_local.tableSize,
					local_callsite_src_id_cache_hashkey,
					local_callsite_src_id_cache_comparator);
		}

		/* Try to allocate space for max count of callsite info from all tasks  */
		
		
		mpiPi_local.rawCallsiteData = (callsite_stats_t *) calloc (maxCount, sizeof (callsite_stats_t));
		if (mpiPi_local.rawCallsiteData == NULL)
		{
			mpiPi_msg_warn ("Failed to allocate memory to collect callsite info");
			retval = 0;
		}

		/* Clear global_mpi_time and global_mpi_size before accumulation in mpiPi_insert_callsite_records */
		mpiPi_local.global_mpi_time = 0.0;
		mpiPi_local.global_mpi_size = 0.0;

	}

}


void freeall() {

	//do not deallocate these
//	free(mpiPi_local.global_task_hostnames);	// = NULL;
//	free(mpiPi_local.global_task_app_time);
//	free(mpiPi_local.global_task_mpi_time);

	free(mpiPi_local.global_callsite_stats);
	free(mpiPi_local.global_callsite_stats_agg);	
	free(mpiPi_local.rawCallsiteData);
}

void
mpiPi_local_generateReport() {

/*
	mpiP_TIMER dur;
	mpiPi_GETTIME (&mpiPi_local.endTime);
	dur = (mpiPi_GETTIMEDIFF (&mpiPi_local.endTime, &mpiPi_local.startTime) / 1000000.0);
	double cumulativeTime = dur; //??
	mpiPi_local.cumulativeTime += dur;
	assert (mpiPi_local.cumulativeTime >= 0);
	mpiPi_GETTIME (&mpiPi_local.startTime);
	printf("\nRank %d cumulativeTime %lf\n", mpiPi_local.rank, cumulativeTime);

	PMPI_Barrier(mpiPi_local.comm);
	mpiPi_local_collect_basics (mpiPi_local.report_style);
	mpiPi_local_mergeResults ();
*/
	mpiP_TIMER dur;
	mpiPi_GETTIME (&mpiPi.endTime);
	dur = (mpiPi_GETTIMEDIFF (&mpiPi.endTime, &mpiPi.startTime) / 1000000.0);
	double cumulativeTime = dur; //??
	mpiPi.cumulativeTime += dur;
	assert (mpiPi.cumulativeTime >= 0);
	mpiPi_GETTIME (&mpiPi.startTime);
	printf("\nRank %d cumulativeTime %lf\n", mpiPi.rank, cumulativeTime);

	PMPI_Barrier(mpiPi.comm);

	mpiPi_local_collect_basics (mpiPi.report_style);
	mpiPi_local_mergeResults ();
/*	if (mergeResult == 1 && mpiPi.stackDepth == 0)
		mergeResult = mpiPi_insert_MPI_records ();
	if (mergeResult == 1)
		mergeResult = mpiPi_mergeCollectiveStats ();
	if (mergeResult == 1)
		mergeResult = mpiPi_mergept2ptStats ();
*/
	PMPI_Barrier(mpiPi.comm);

	printf("\nRank %d publishResults\n", mpiPi.rank);
	dumpCount++;
	FILE *fp = NULL;
	char oFile[256];
	snprintf (oFile, 256, "dump.%d.%d.%d.mpiP", mpiPi.size, mpiPi.procID, dumpCount);
	if (mpiPi.collectorRank == mpiPi.rank) 
		fp = fopen(oFile, "w");	

	mpiPi_profile_print (fp, mpiPi.report_style);

	if (mpiPi.collectorRank == mpiPi.rank) 
		fclose(fp);

	//freeall();

}



void copyToLocal() {

	//copy from mpiPi.to mpiPi.enabled
	if (mpiPi.tag == 9821) {
 	  MPI_Comm_rank(mpiPi.comm, &myrank);
	  printf("Rank: %d %d\n", mpiPi.rank, myrank);
	}
	else
	  printf("mpiPi tag != 9821 %d\n", mpiPi.tag);
	  
//	MPI_Comm_dup (mpiPi.comm, &mpiPi_local.comm);


	//mpiPi_local_init();
	mpiPi_local = mpiPi;
//	mpiPi_local.enabled = 1;		//enabled if flipped on and off in the MPI processes
	//if (mpiPi_local.enabled) 
	//    mpiPi_GETTIME (&mpiPi_local.startTime);
	
	 mpiPi_local.task_callsite_stats = h_open (mpiPi.tableSize, mpiPi_local_callsite_stats_pc_hashkey, mpiPi_local_callsite_stats_pc_comparator);

}

void tokenize(char *input, const char *marker, char *returnstr) {

//	if (input == NULL) return "Byebye";

//        char *returnstr = (char *) malloc(msgsize*sizeof(char));

//	const char marker[2] = "\n";
	
	printf("Enter tokenize\n");

	printf ("input: %s marker: %s\n", input, marker);
        char *token = strtok (input, marker);
        while (token != NULL) {
                printf ("token: %s\n", token);
                strcpy(returnstr, token);
                token = strtok (NULL, marker);
        }
//        return returnstr;
}


void sim_msg_handler(int sockfd)
{
       // static char func[] = "sim_msg_handler";
	//printf("\nhandler\n");

//test
/*
        char *buf = "Hey there";
        size_t msgsize = strlen(buf)+1;
        if (send(sockfd, buf, msgsize, 0) < 0)
                perror("Send failed");

        char *buffer = (char *)malloc(msgsize*sizeof(char));
        if(recv(sockfd, buffer, 512, 0) < 0)
                perror("Recv failed");
        printf("%s\n", buffer);
*/

      	int numbytes;
        char *buffer = (char *)malloc(msgsize*sizeof(char));
        char *last = (char *)malloc(msgsize*sizeof(char));
	const char marker[2] = "%";

        while(1) {
		
		int time = mpiPi.rank * 8 + mpiPi.rank;
	//	sleep(time);
                //if(recv(sockfd, buffer, msgsize, 0) < 0)
                while((numbytes = recv(sockfd, buffer, msgsize, 0)) <= 0);
//                        perror("Recv failed");
                printf("\nReceived [%s]\n", buffer);
                tokenize(buffer, marker, last);
		printf("\n%d\n", numbytes);

                if(strncmp(last, "Byebye", 6) == 0) {
	
			printf("\nlast token: %s from %d\n", last, mpiPi.rank);
                        break;
		}
	
        }

	//copyToLocal();
	printf("\nrank %d enabled %d enabledCount %d collector %d tableSize %d tag %d\n", mpiPi.rank, mpiPi.enabled, mpiPi.enabledCount, mpiPi.collectorRank, mpiPi.tableSize, mpiPi.tag);
	//printf("\nrank %d tag %d\n", mpiPi_local.rank, mpiPi_local.tag);

	int i=0;
	for(i=0; i<4; i++) {
		sleep(i*3);
		PMPI_Barrier(mpiPi.comm);		
		mpiPi_local_generateReport();
	}

	printf("\n****\n");

}


//void initConn() {

void * initConn(void *arg) {

	sleep(6);

	//printf("\ninitConn");

//	int * rank = (int *)arg;
//	printf("\ninitConn called %d\n", *rank);

	if ((hostname = gethostbyname(serverIP)) == 0) {
                perror("client: gethostbyname error ");
                exit(1);
        }

	   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("client: socket error ");
                exit(1);
        }

//	printf("\nsockfd %d\n", sockfd);
//	printf("\nrank %d mpiPi.enabled %d\n", mpiPi.rank, mpiPi.enabled);

        // Fill socket structure with host information

        memset(&address, 0, sizeof(address));   //bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = ((struct in_addr *)(hostname->h_addr))->s_addr;

        // Signal handler to signal client when simdaemon sends message
        //signal(SIGIO, sim_msg_handler);

        // Connect to socket 

	printf("\nConnect to socket sockfd %d\n", sockfd);

        if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1) {
                perror("client: connect error ");
                exit(1);
        }

	copyToLocal();
	sim_msg_handler(sockfd);

	close(sockfd);

//	return NULL;
	pthread_exit((void*) 0);
}


void * initSocketConn(void *arg) {

	// Create an endpoint for communication

        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("vizConn: socket error ");
                exit(1);
        }

	fcntl(sockfd, F_SETFL, O_NONBLOCK);  // set to non-blocking
//	fcntl(sockfd, F_SETFL, O_ASYNC);     // set to asynchronous I/O
	
	// Fill socket structure with host information

	printf("\nI am server\n");

	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(PORT);
	server_address.sin_addr.s_addr = INADDR_ANY;

	/*if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
		perror("vizConn: bind error ");
		exit(1);
	}*/

	if (listen(sockfd, 5) == -1) {
		perror("vizConn: listen error ");
		exit(1);
	}
	
	client_address_len = sizeof(client_address);
	if ((client_sockfd = accept(sockfd, (struct sockaddr *) &client_address, &client_address_len)) == -1) {
		perror("vizConn: accept error ");
	}

	return NULL;
}

void handler(int signum){

	printf("\nIn handler: %d\n", signum);
        printf("\nvalue of cruelty %d\n", pmb.info.cruelty);
	
	return ;

//	msgsnd() and msgrcv()

/*
	char msgcontent[10000];
	unsigned int sender;

	int msgsz = mq_receive(rmsgq_id, msgcontent, sizeof(msgcontent), &sender);
        if (msgsz == -1) {
                perror("In mq_receive()");
                exit(1);
        }
*/

//wont work coz 0 process is calling handler only....
//	mpiPi_generateReport (mpiPi.report_style);
//	mpiPi_collect_basics (mpiPi.report_style);
//	PMPI_Gather (&mpiPi.cumulativeTime, 1, MPI_DOUBLE, mpiPi.global_task_app_time, 1, MPI_DOUBLE, mpiPi.collectorRank, mpiPi.comm);

/*
	printf("\nReceived message (%d bytes) from %d: [%s]\n", msgsz, sender, msgcontent);

	if (mpiPi.global_MPI_stats_agg != NULL)
		printf("\nmpiPi.global_MPI_stats_agg->size=%d\n", mpiPi.global_MPI_stats_agg->size);

	if (mq_notify (rmsgq_id, &sigevent) == -1) {
                if (errno == EBUSY)
                        printf ("Another process has registered for notification.\n");
        }   

*/
	
}


void initMQConn() {


	struct sigevent sev;
	sigset_t mask;
	struct sigaction sa;

	printf("Establishing handler for signal %d\n", SIGIO);
	sa.sa_flags = SA_SIGINFO;
//	sa.sa_sigaction = handler;
	sa.sa_handler = handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGIO, &sa, NULL);
	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGIO;

	//signal(SIGIO, handler);
/*        signal(SIGUSR1, handler);
	sigevent.sigev_notify = SIGEV_SIGNAL;
	sigevent.sigev_signo = SIGUSR1;
*/
/*
	if (signal(SIGUSR1, handler) == SIG_ERR)
	  printf("\ncan't catch SIGUSR1\n");
*/
	key_t key = ftok("/home/preeti/keyfile", 'a');
	//if (msqid = msgget(key, 0666 | IPC_CREAT) == -1)
	if ((msqid = msgget(111, IPC_CREAT | 0666)) == -1)
		printf("Error in msgget\n");

//        pmb.info.cruelty = 0;
        printf("\nOld value of cruelty: %d\n", pmb.info.cruelty);
//	msgrcv(msqid, &pmb, sizeof(struct pirate_msgbuf) - sizeof(long), 2, IPC_NOWAIT);
	msgrcv(msqid, &pmb, sizeof(struct pirate_msgbuf) - sizeof(long), 2, 0);
        printf("\nvalue of cruelty %d\n", pmb.info.cruelty);

//	sleep(1);
//	raise(SIGUSR1);
/*
	struct sigaction action;
	action.sa_handler = handler;
	sigemptyset (&action.sa_mask);
        action.sa_flags = 0;
	if (sigaction(SIGIO, &action, (struct sigaction *)NULL) == -1)
		perror("Failed to set new Handle");
*/

/*

	// Send message queue name
        const char *MSGQ_NAME = "/mqueue";

        // open another and nofify
        rmsgq_id = mq_open(MSGQ_NAME, O_RDWR | O_CREAT | O_EXCL, S_IRWXU | S_IRWXG, NULL);

        if (rmsgq_id == -1) {
                printf("error: %s (%d)\n", strerror(errno), errno);
        }
*/

//        signal(SIGINT, handler);
//	signal (SIGUSR1, handler);
//	sigevent.sigev_signo = SIGUSR1;

	//if (mq_notify (rmsgq_id, &sigevent) == -1) {
/*	if (mq_notify (msqid, &sigevent) == -1) {
		if (errno == EBUSY) 
			printf ("Another process has registered for notification.\n");
	}   
*/

}

void finalConn() {

	//msgctl(msqid, IPC_RMID, NULL);
	
}
