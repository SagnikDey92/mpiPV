#define FREQ 15
void * initConn(void *arg) {
       // if (mpiPi.rank == mpiPi.collectorRank) {
                while(1) {
			printf("debug1\n\n");
                        mpiPi_generateReport (mpiPi.report_style);
			//MPI_Pcontrol(3);
			printf("debug2\n\n");
                        MPI_Pcontrol(2);
			printf("debug3\n\n");
                        sleep(FREQ);
			printf("debug4\n\n");
                }
        //}
}

