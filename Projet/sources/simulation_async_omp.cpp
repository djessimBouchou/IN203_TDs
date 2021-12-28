#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
#include <chrono>
#include "contexte.hpp"
#include "individu.hpp"
#include <mpi.h>
#include <omp.h>
#include "graphisme/src/SDL2/sdl2.hpp"

void màjStatistique( épidémie::Grille& grille, std::vector<épidémie::Individu> const& individus )
{
    auto& statistique = grille.getStatistiques();

    #pragma omp parallel for schedule(dynamic, 10000)
    for ( int i = 0 ; i < (int) statistique.size() ; i++ )
    {
        statistique[i].nombre_contaminant_grippé_et_contaminé_par_agent = 0;
        statistique[i].nombre_contaminant_seulement_contaminé_par_agent = 0;
        statistique[i].nombre_contaminant_seulement_grippé              = 0;
    }
    auto [largeur,hauteur] = grille.dimension();
    auto& statistiques = grille.getStatistiques();

    #pragma omp parallel for schedule(dynamic, 10000)
    for ( int k = 0; k < (int) individus.size() ; k++ )
    {
        auto pos = individus[k].position();

        std::size_t index = pos.x + pos.y * largeur;
        if (individus[k].aGrippeContagieuse() )
        {
            if (individus[k].aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_grippé_et_contaminé_par_agent += 1;
            }
            else 
            {
                statistiques[index].nombre_contaminant_seulement_grippé += 1;
            }
        }
        else
        {
            if (individus[k].aAgentPathogèneContagieux())
            {
                statistiques[index].nombre_contaminant_seulement_contaminé_par_agent += 1;
            }
        }
    }
}

void afficheSimulation(sdl2::window& écran, std::vector<épidémie::Grille::StatistiqueParCase>& stat_grille, std::size_t jour, int largeur_grille, int hauteur_grille)
{
    auto [largeur_écran,hauteur_écran] = écran.dimensions();
    //auto [largeur_grille,hauteur_grille] = grille.dimension();
    //auto const& statistiques = grille.getStatistiques();
    sdl2::font fonte_texte("./graphisme/src/data/Lato-Bold.ttf", 18);
    écran.cls({0x00,0x00,0x00});
    // Affichage de la grille :
    std::uint16_t stepX = largeur_écran/largeur_grille;
    unsigned short stepY = (hauteur_écran-50)/hauteur_grille;
    double factor = 255./15.;
    for ( unsigned short i = 0; i < largeur_grille; ++i )
    {
        for (unsigned short j = 0; j < hauteur_grille; ++j )
        {
            auto const& stat = stat_grille[i+j*largeur_grille];
            int valueGrippe = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_grippé;
            int valueAgent  = stat.nombre_contaminant_grippé_et_contaminé_par_agent+stat.nombre_contaminant_seulement_contaminé_par_agent;
            std::uint16_t origx = i*stepX;
            std::uint16_t origy = j*stepY;
            std::uint8_t red = valueGrippe > 0 ? 127+std::uint8_t(std::min(128., 0.5*factor*valueGrippe)) : 0;
            std::uint8_t green = std::uint8_t(std::min(255., factor*valueAgent));
            std::uint8_t blue= std::uint8_t(std::min(255., factor*valueAgent ));
            écran << sdl2::rectangle({origx,origy}, {stepX,stepY}, {red, green,blue}, true);
        }
    }
    //#pragma omp barrier
    sdl2::color white = {255,255,255,255};
    écran << sdl2::texte("Carte population grippée", fonte_texte, écran, white).at(largeur_écran/2, hauteur_écran-20);
    écran << sdl2::texte(std::string("Jour : ") + std::to_string(jour), fonte_texte, écran, white).at(0,hauteur_écran-20);
    écran << sdl2::flush;
}

void simulation(bool affiche, MPI_Comm globComm, MPI_Status status, int tag, int rank, MPI_Request request)
{
    
    sdl2::window* écran;
    if(rank==0)
    {
        constexpr const unsigned int largeur_écran = 1280, hauteur_écran = 1024;
        écran = new sdl2::window("Simulation épidémie de grippe", {largeur_écran,hauteur_écran});
    }

    unsigned int graine_aléatoire = 1;
    std::uniform_real_distribution<double> porteur_pathogène(0.,1.);

    épidémie::ContexteGlobal contexte;
    // contexte.déplacement_maximal = 1; <= Si on veut moins de brassage
    contexte.taux_population = 100'000;
    //contexte.taux_population = 1'000;
    contexte.interactions.β = 60.;
    std::vector<épidémie::Individu> population;
    population.reserve(contexte.taux_population);
    épidémie::Grille grille{contexte.taux_population};

    auto [largeur_grille,hauteur_grille] = grille.dimension();
    // L'agent pathogène n'évolue pas et reste donc constant...
    épidémie::AgentPathogène agent(graine_aléatoire++);
    // Initialisation de la population initiale :
    for (std::size_t i = 0; i < contexte.taux_population; ++i )
    {
        std::default_random_engine motor(100*(i+1));
        population.emplace_back(graine_aléatoire++, contexte.espérance_de_vie, contexte.déplacement_maximal);
        population.back().setPosition(largeur_grille, hauteur_grille);
        if (porteur_pathogène(motor) < 0.2)
        {
            population.back().estContaminé(agent);   
        }
    }

    std::size_t jours_écoulés = 0;
    int         jour_apparition_grippe = 0;
    int         nombre_immunisés_grippe= (contexte.taux_population*23)/100;
    sdl2::event_queue queue;

  

    std::ofstream output;

    if(rank == 1)
    {
        output.open("Courbe.dat");
        output << "# jours_écoulés \t nombreTotalContaminésGrippe \t nombreTotalContaminésAgentPathogène()" << std::endl; 
        std::cout << "Début boucle épidémie" << std::endl << std::flush;
    }
        

    épidémie::Grippe grippe(0);

    auto [dim_x, dim_y] = grille.dimension();


    
    double total = 0;
    std::chrono::time_point < std::chrono::system_clock > start, end;

    bool quitting = false;

    while (!quitting)
    {
        start = std::chrono::system_clock::now();
        auto events = queue.pull_events();
        for ( const auto& e : events)
        {
            if (e->kind_of_event() == sdl2::event::quit)
                quitting = true;
        }

        if(rank==1) 
        {
            if (jours_écoulés%365 == 0)// Si le premier Octobre (début de l'année pour l'épidémie ;-) )
            {
                grippe = épidémie::Grippe(jours_écoulés/365);
                jour_apparition_grippe = grippe.dateCalculImportationGrippe();
                grippe.calculNouveauTauxTransmission();
                // 23% des gens sont immunisés. On prend les 23% premiers
                #pragma omp parallel for schedule(dynamic, 1000)
                for ( int ipersonne = 0; ipersonne < nombre_immunisés_grippe; ++ipersonne)
                {
                    population[ipersonne].devientImmuniséGrippe();
                }
                #pragma omp parallel for schedule(dynamic, 1000)
                for ( int ipersonne = nombre_immunisés_grippe; ipersonne < int(contexte.taux_population); ++ipersonne )
                {
                    population[ipersonne].redevientSensibleGrippe();
                }
            }
            if (jours_écoulés%365 == std::size_t(jour_apparition_grippe))
            {
                #pragma omp parallel for schedule(dynamic, 3)
                for (int ipersonne = nombre_immunisés_grippe; ipersonne < nombre_immunisés_grippe + 25; ++ipersonne )
                {
                    population[ipersonne].estContaminé(grippe);
                }
            }
            // Mise à jour des statistiques pour les cases de la grille :
            màjStatistique(grille, population);
            // On parcout la population pour voir qui est contaminé et qui ne l'est pas, d'abord pour la grippe puis pour l'agent pathogène
            std::size_t compteur_grippe = 0, compteur_agent = 0, mouru = 0;

            #pragma omp parallel for schedule(dynamic, 1000)
            for ( int i = 0 ; i < (int) population.size() ; i++ )
            {
                if (population[i].testContaminationGrippe(grille, contexte.interactions, grippe, agent))
                {
                    compteur_grippe ++;
                    population[i].estContaminé(grippe);
                }
                if (population[i].testContaminationAgent(grille, agent))
                {
                    compteur_agent ++;
                    population[i].estContaminé(agent);
                }
                // On vérifie si il n'y a pas de personne qui veillissent de veillesse et on génère une nouvelle personne si c'est le cas.
                if (population[i].doitMourir())
                {
                    mouru++;
                    unsigned nouvelle_graine = jours_écoulés + population[i].position().x*population[i].position().y;
                    population[i] = épidémie::Individu(nouvelle_graine, contexte.espérance_de_vie, contexte.déplacement_maximal);
                    population[i].setPosition(largeur_grille, hauteur_grille);
                }
                population[i].veillirDUnJour();
                population[i].seDéplace(grille);
            }
            
            auto& statistiques = grille.getStatistiques();
            
            int flag = 0;
            MPI_Iprobe(0, tag, globComm, &flag, &status);
            if(flag)
            {
                int buffer;
                MPI_Irecv(&buffer, 1, MPI_INT, 0, tag, globComm, &request);
                MPI_Isend(statistiques.data(), dim_x * dim_y * 3, MPI_INT, 0, tag, globComm, &request);
                MPI_Isend(&jours_écoulés, 1, MPI_INT, 0, tag, globComm, &request);
            }
            
        }

        
        //#############################################################################################################
        //##    Affichage des résultats pour le temps  actuel
        //#############################################################################################################
        else if (rank==0)
        {
                int envoie = 1;
                MPI_Isend(&envoie, 1, MPI_INT, 1, tag, globComm, &request);

                std::vector<épidémie::Grille::StatistiqueParCase> buffer(dim_x * dim_y);
                MPI_Recv(buffer.data(), dim_x * dim_y * 3, MPI_INT, 1, tag, globComm, &status);
                MPI_Recv(&jours_écoulés, 1, MPI_INT, 1, tag, globComm, &status);
                if (affiche) afficheSimulation(*écran, buffer, jours_écoulés, dim_x, dim_y);

                
        }

        /*std::cout << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
                  << grille.nombreTotalContaminésAgentPathogène() << std::endl;*/
        end = std::chrono::system_clock::now();
        if(rank == 1)
        {
            std::chrono::duration < double >elapsed_seconds = end - start;
            total += elapsed_seconds.count();
            std::cout << "Temps écoulé lors du pas de temps " << jours_écoulés << " : " << elapsed_seconds.count() << "\n"; 
            if(jours_écoulés == 200) std::cout << "En moyenne : " << total/200 << std::endl;
            output << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
               << grille.nombreTotalContaminésAgentPathogène() << std::endl;

            if(jours_écoulés >= 200)
            {
                quitting = true;
            } 
            jours_écoulés += 1;

        }        
        
    }// Fin boucle temporelle
    if(rank == 1) output.close();
    
}

int main(int argc, char* argv[])
{
    // On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init( &argc, &argv );
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

	int tag = 1212;
	MPI_Status status;
    MPI_Request request;
    

    // parse command-line
    bool affiche = true;
    for (int i=0; i<argc; i++) {
      std::cout << i << " " << argv[i] << "\n";
      if (std::string("-nw") == argv[i]) affiche = false;
    }
  
    sdl2::init(argc, argv);
    {
        simulation(affiche, globComm, status, tag, rank, request);
    }
    sdl2::finalize();
    MPI_Finalize();
    return EXIT_SUCCESS;
}
