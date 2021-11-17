Diffusion_hypercube_dim_1.cpp                                                                       0000644 0001750 0001750 00000004475 14144022517 016517  0                                                                                                    ustar   djessim                         djessim                                                                                                                                                                                                                # include <cstdlib>
# include <sstream>
# include <string>
# include <stdlib.h>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>

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
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	unsigned long tmp;
	int tag = 1212;
	MPI_Status status;

	if(nargs != 2)
	{
		std::cout << "Error. Wrong number of arguments." << std::endl;
		return(-1);
	}
	unsigned long val_prog = atoi(argv[1]);

	if(rank==0)
	{
		MPI_Send(&val_prog, 1, MPI_UNSIGNED_LONG, 1, tag, globComm);
		output << "Valeur transmise : " << val_prog << std::endl;
	}
	else
	{
		MPI_Recv(&tmp, 1, MPI_UNSIGNED_LONG, 0,tag, globComm, &status);
		output << "Valeur transmise : " << tmp << std::endl;
	}

	
	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
                                                                                                                                                                                                   Diffusion_hypercube_dim_d.cpp                                                                       0000644 0001750 0001750 00000005041 14144022510 016561  0                                                                                                    ustar   djessim                         djessim                                                                                                                                                                                                                # include <cstdlib>
# include <sstream>
# include <string>
# include <stdlib.h>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <math.h>
# include <mpi.h>

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
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	unsigned long tmp;
	int tag = 1212;
	MPI_Status status;

	if(nargs != 2)
	{
		std::cout << "Error. Wrong number of arguments." << std::endl;
		return(-1);
	}
	unsigned long val_prog = atoi(argv[1]); // Valeur à diffuser

	int d = 4 ; // Dimension de l'hypercube

	for(int i=1; i<pow(2, d-1); i=i*2)
	{
		if(rank==0)
		{
			tmp = val_prog;
			MPI_Send(&tmp, 1, MPI_UNSIGNED_LONG, i, tag, globComm);
			
		}
		else if(rank>0 && rank<i)
		{
			MPI_Send(&tmp, 1, MPI_UNSIGNED_LONG, i+rank, tag, globComm);
			
		}
		else if(2*i > rank && rank >= i)
		{
			MPI_Recv(&tmp, 1, MPI_UNSIGNED_LONG, rank-i,tag, globComm, &status);
		}
	}
	
	
	output << "Valeur transmise : " << tmp << std::endl;
	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               HelloWorld.cpp                                                                                      0000644 0001750 0001750 00000003656 14144022452 013523  0                                                                                                    ustar   djessim                         djessim                                                                                                                                                                                                                # include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>

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
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	output << "Bonjour, je suis la tâche n° " << rank << " sur " << nbp << " tâches." << std::endl;

	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
                                                                                  Jeton.cpp                                                                                           0000644 0001750 0001750 00000004117 14144022463 012522  0                                                                                                    ustar   djessim                         djessim                                                                                                                                                                                                                # include <iostream>
# include <cstdlib>
# include <mpi.h>

int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...

	MPI_Init( &nargs, &argv );
	// Pour des raison préfère toujours cloner le communicateur global
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

	int tag = 4242;
	int msg_to_recv;
	MPI_Status status;
	if(rank==0)
	{
		int buffer = 42;

		MPI_Send(&buffer, 1, MPI_INT, 1, tag, globComm);
		MPI_Recv(&msg_to_recv, 1, MPI_INT, 
                 nbp-1, tag, globComm, &status);
		msg_to_recv++;
	}
	else
	{
		MPI_Recv(&msg_to_recv, 1, MPI_INT, 
                 rank-1, tag, globComm, &status);
		msg_to_recv++;
		MPI_Send(&msg_to_recv, 1, MPI_INT, (rank+1)%nbp, tag, globComm);
	}
	

	// On peut maintenant commencer à écrire notre programme parallèle en utilisant les
	// services offerts par MPI.
	std::cout << "Je suis la tâche " << rank << " avec la valeur de " << msg_to_recv << ".\n";

	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                 JetonRandom.cpp                                                                                     0000644 0001750 0001750 00000004352 14144022477 013671  0                                                                                                    ustar   djessim                         djessim                                                                                                                                                                                                                # include <iostream>
# include <cstdlib>
# include <mpi.h>
#include<chrono>
#include<random>

int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...

	MPI_Init( &nargs, &argv );
	// Pour des raison préfère toujours cloner le communicateur global
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

	int tag = 4242;
	int msg_to_recv;
	MPI_Status status;

	typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning=myclock::now();
    myclock::duration d = myclock::now() -beginning;
    unsigned seed= d.count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(-50000,50000);
	int buffer = distribution(generator);
	
	MPI_Send(&buffer, 1, MPI_INT, (rank+1)%nbp, tag, globComm);
	MPI_Recv(&msg_to_recv, 1, MPI_INT, 
				(rank-1)%nbp, tag, globComm, &status);
	msg_to_recv++;


	// On peut maintenant commencer à écrire notre programme parallèle en utilisant les
	// services offerts par MPI.
	std::cout << "Je suis la tâche " << rank << " avec la valeur de " << msg_to_recv << ".\n";

	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}
                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      