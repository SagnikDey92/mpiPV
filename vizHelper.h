void tokenize(char *input, const char *marker, char *returnstr) {

//	if (input == NULL) return "Byebye";
//        char *returnstr = (char *) malloc(msgsize*sizeof(char));
//	const char marker[2] = "\n";
//	printf("Enter tokenize\n");
//	printf("input: %s marker: %s\n", input, marker);
        char *token = strtok (input, marker);
        while (token != NULL) {
//            printf ("token: %s\n", token);
            strcpy(returnstr, token);
            token = strtok (NULL, marker);
        }
//        return returnstr;
}


void sim_msg_handler(int sockfd)
{
      	int i=0, numbytes;
        char *buffer = (char *)malloc(msgsize*sizeof(char));
        char *last = (char *)malloc(msgsize*sizeof(char));
	const char marker[2] = "%";

	while (1) {
		buffer[0] = '\0';
		sleep (1);
		//printf("\nDEBUG: %d rank %d tag %d local collector %d\n", i, mpiPi.rank, mpiPi.tag, mpiPi.collectorRank);
    		mpiPi_generateReport (mpiPi.report_style);
		MPI_Pcontrol(2);
                int number = 123;
                if (mpiPi.rank == mpiPi.collectorRank) {
                  if((numbytes = recv(sockfd, buffer, msgsize, 0)) == -1) 
          	    perror("\nRecv failed\n");              
                  MPI_Send(&number, 1, MPI_INT, 1, 0, mpiPi.comm);
                }  
                else
                  MPI_Recv(&number, 1, MPI_INT, 0, 0, mpiPi.comm, MPI_STATUS_IGNORE);
                      
                // if (world_rank == 0) {
                //         number = -1;
                //         MPI_Send(&number, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
                //         } else if (world_rank == 1) {
                //         MPI_Recv(&number, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                //                 MPI_STATUS_IGNORE);
                //         printf("Process 1 received number %d from process 0\n",
                //                 number);
                //         }
                printf("\nDEBUG: %d Received [%s]\n", mpiPi.rank, buffer);
                tokenize(buffer, marker, last);
/*
                printf("\nDEBUG: %d Received [%s] [%s]\n", mpiPi.rank, buffer, last);
		printf("\nDEBUG: %d numbytes = %d\n", mpiPi.rank, numbytes);
*/
	  	if(strncmp(buffer, "Byebye", 6) == 0) 
		  break;
	}
//	close(sockfd);
}

void * initConn(void *arg) {

	int i=0, flags;
        char *buffer = (char *)malloc(msgsize*sizeof(char));
	struct sigaction saio; 

	//sleep(10);
	printf("\ninitConn process %d thread id %d \n", getpid(), pthread_self());

//	int * rank = (int *)arg;
//	printf("\ninitConn called %d\n", *rank);
        if (mpiPi.rank == mpiPi.collectorRank)
        {
	if ((hostname = gethostbyname(serverIP)) == 0) {
                perror("client: gethostbyname error ");
                exit(1);
        }

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
                perror("client: socket error ");
                exit(1);
        }

	/* install the signal handler before making the device asynchronous */
        /*saio.sa_handler = sim_msg_handler;
	sigemptyset(&saio.sa_mask);
        saio.sa_flags = 0;
        saio.sa_restorer = NULL;
        sigaction(SIGIO,&saio,NULL);
*/
        // Fill socket structure with host information

        memset(&address, 0, sizeof(address));   //bzero(&address, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_port = htons(PORT);
        address.sin_addr.s_addr = ((struct in_addr *)(hostname->h_addr))->s_addr;

        // Signal handler to signal client when simdaemon sends message
       // signal(SIGIO, sim_msg_handler);

        // Connect to socket 
	printf("\n%d: Connect to socket sockfd %d\n", mpiPi.rank, sockfd);
        if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) == -1) {
                perror("client: connect error ");
                exit(1);
        }
        }
	//fcntl(sockfd, F_SETOWN, getpid());
	//flags = fcntl(sockfd, F_GETFL);
	//fcntl(sockfd, F_SETFL, flags | O_ASYNC | O_RDWR | O_NONBLOCK); // | FASYNC);

	sim_msg_handler(sockfd);

}

