#define FREQ 15
void * initConn(void *arg) {

	//if (mpiPi_local.rank == mpiPi_local.collectorRank)
        int *check = (int*)arg;
        while(!(*check)) {
                if (mpiPi.rank == mpiPi.collectorRank)
                        mpiPi_generateReport (mpiPi.report_style);
                sleep(FREQ);
        }
}

