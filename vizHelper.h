#define FREQ 15
void * initConn(void *arg) {
       while(1) {
		MPI_Pcontrol(3);
		MPI_Pcontrol(2);
		sleep(FREQ);
       }
}

