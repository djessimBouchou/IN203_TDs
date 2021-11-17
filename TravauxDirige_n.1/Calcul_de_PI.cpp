#include<chrono>
#include<random>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <cstdlib>
# include <iomanip>
# include <mpi.h>

//Attention,ne marche qu'en C++11 ou supérieur : 
double approximate_pi(unsigned long nbSamples, int rank) 
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning=myclock::now();
    myclock::duration d = myclock::now() -beginning;
    unsigned seed= d.count() + rank;
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<double> distribution(-1.0,1.0);
    unsigned long nbDarts= 0;
    
    //Throw nbSamples darts in the unit square [-1:1]x[-1:1]
    
    for(unsigned sample= 0;sample<nbSamples; ++sample) 
    {
        double x=distribution(generator);
        double y=distribution(generator);
        //Test if the dart is in the unit disk
        if(x*x+y*y<=1 ) nbDarts++;
    }
    double ratio = (double) nbDarts / nbSamples ;
    return 4*ratio;
}

int main( int nargs, char* argv[] )
{
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
	
    MPI_Status status;
    int tag = 1425;
    double ratio;

    unsigned long nbSamples = atoi(argv[1]);
    if(rank== 0)
    {
        double approx = 0;
        for(int i=1; i<nbp; i++)
        {
            MPI_Recv(&ratio, 1, MPI_DOUBLE, i, tag, globComm, &status);
            approx += ratio;
        }
        std::cout << "L'approcimation de PI est : " << (double) approx/(nbp-1) << std::endl;
    }else
    {
        ratio = approximate_pi(nbSamples,rank);
        MPI_Send(&ratio, 1, MPI_DOUBLE, 0, tag, globComm);
    }
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}