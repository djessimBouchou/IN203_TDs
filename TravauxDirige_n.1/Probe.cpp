# include <cstdlib>
# include <sstream>
# include <string>
# include <stdlib.h>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <math.h>
# include <mpi.h>
#include <chrono>
#include <thread>

int main( int nargs, char* argv[] )
{
    using namespace std::this_thread; // sleep_for, sleep_until
    using namespace std::chrono; // nanoseconds, system_clock, seconds
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init( &nargs, &argv );
	// Pour des raisons de portabilité qui débordent largement du cadre
	// de ce cours, on préfère toujours cloner le communicateur global
	// MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour connaître le nombre de processus
	// qui ont été lancés par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour connaître l'identifiant qui
	// m'a été attribué ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	unsigned long tmp;
	int tag = 1212;
	MPI_Status status;
    MPI_Request request;

	int flag = 0;
    int value;
    int v[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
    while(1)
    {
    if (rank == 0) 
    {
        sleep_for(seconds(1));
        while(!flag) MPI_Iprobe(1, tag, globComm, &flag, &status);
        MPI_Irecv(&value, 1, MPI_INT, 1, tag, globComm, &request);
        std::cout << value << std::endl;
        sleep_for(seconds(1));
        flag = 0;
        while(!flag) MPI_Iprobe(1, tag, globComm, &flag, &status);
        MPI_Irecv(&value, 1, MPI_INT, 1, tag, globComm, &request);
        std::cout << value << std::endl;
        
        
    }
    else
    {
        int count = 0;
        while(count < 10) {
            MPI_Isend(v + count, 1, MPI_INT, 0, tag, globComm, &request);
            count++;
        }
        
        
    } 
    }
    
    

	MPI_Finalize();
	return EXIT_SUCCESS;
}
