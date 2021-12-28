#include <cstdlib>
#include <random>
#include <iostream>
#include <fstream>
# include <chrono>
#include <mpi.h>
#include<omp.h>
#include "contexte.hpp"
#include "individu.hpp"
#include "graphisme/src/SDL2/sdl2.hpp"

void màjStatistique( std::vector<épidémie::Grille::StatistiqueParCase>& grille_stat, std::vector<épidémie::Individu> const& individus,
                    int largeur)
{
    auto& statistique = grille_stat;
    int taille_grille = (int) statistique.size();

    #pragma omp parallel for schedule(dynamic, 1000)
    for ( int i = 0; i < taille_grille; i++ )
    {
        statistique[i].nombre_contaminant_grippé_et_contaminé_par_agent = 0;
        statistique[i].nombre_contaminant_seulement_contaminé_par_agent = 0;
        statistique[i].nombre_contaminant_seulement_grippé              = 0;
    }

    auto const& personne = individus;
    int taille_ind = (int)individus.size();

    #pragma omp parallel for schedule(dynamic, 1000)
    for ( int k = 0; k < taille_ind; k++ )
    {
        auto pos = personne[k].position();

        std::size_t index = pos.x + pos.y * largeur;
        if (personne[k].aGrippeContagieuse() )
        {
            if (personne[k].aAgentPathogèneContagieux())
            {
                statistique[index].nombre_contaminant_grippé_et_contaminé_par_agent += 1;
            }
            else 
            {
                statistique[index].nombre_contaminant_seulement_grippé += 1;
            }
        }
        else
        {
            if (personne[k].aAgentPathogèneContagieux())
            {
                statistique[index].nombre_contaminant_seulement_contaminé_par_agent += 1;
            }
        }
    }
}

void afficheSimulation(sdl2::window& écran, std::vector<épidémie::Grille::StatistiqueParCase> const& grille_stat, std::size_t jour, 
	int largeur_grille, int hauteur_grille)
{
    auto [largeur_écran,hauteur_écran] = écran.dimensions();
    auto const& statistiques = grille_stat;
    sdl2::font fonte_texte("./graphisme/src/data/Lato-Thin.ttf", 18);
    écran.cls({0x00,0x00,0x00});
    // Affichage de la grille :
    std::uint16_t stepX = largeur_écran/largeur_grille;
    unsigned short stepY = (hauteur_écran-50)/hauteur_grille;
    double factor = 255./15.;

    for ( unsigned short i = 0; i < largeur_grille; ++i )
    {
        for (unsigned short j = 0; j < hauteur_grille; ++j )
        {
            auto const& stat = statistiques[i+j*largeur_grille];
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

    écran << sdl2::texte("Carte population grippée", fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(largeur_écran/2, hauteur_écran-20);
    écran << sdl2::texte(std::string("Jour : ") + std::to_string(jour), fonte_texte, écran, {0xFF,0xFF,0xFF,0xFF}).at(0,hauteur_écran-20);
    écran << sdl2::flush;
}


void simulation(bool affiche)
{
    MPI_Status status;
    MPI_Request request;
    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    int color;
    if(world_rank==0)
    {
        color = 0;
    }
    else
    {
        color = 1;
    }

    MPI_Comm row_comm;
    MPI_Comm_split(MPI_COMM_WORLD, color, world_rank, &row_comm);

    int row_rank, row_size;
    MPI_Comm_rank(row_comm, &row_rank);
    MPI_Comm_size(row_comm, &row_size);

    if (world_rank==0)
    {
        constexpr const unsigned int largeur_écran = 1280, hauteur_écran = 1024;
        sdl2::window écran("Simulation épidémie de grippe", {largeur_écran,hauteur_écran});

        std::ofstream output("Courbe.dat");
        output << "# jours_écoulés \t nombreTotalContaminésGrippe \t nombreTotalContaminésAgentPathogène()" << std::endl;

        épidémie::ContexteGlobal contexte;
        épidémie::Grille grille{contexte.taux_population};
        auto [largeur_grille,hauteur_grille] = grille.dimension();

        bool quitting = false;
        double compt = 0;
        int jours_comptés = 0;
        while(!quitting)
        {
            std::chrono::time_point<std::chrono::system_clock> start, end, end2;
            start = std::chrono::system_clock::now();
                    
            int wait;
            
            std::vector<épidémie::Grille::StatistiqueParCase> statistiques(largeur_grille*hauteur_grille);


            MPI_Isend( &wait, 1, MPI_INT, 1, 102, MPI_COMM_WORLD, &request);
            MPI_Recv(statistiques.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            int jours_écoulés = status.MPI_TAG;

            if (affiche) afficheSimulation(écran, statistiques, jours_écoulés, largeur_grille, hauteur_grille);

            int flag = 0;
            MPI_Iprobe(1, 103, MPI_COMM_WORLD, &flag, &status);
            std::cout<<jours_écoulés<<" / "<<jours_comptés<<std::endl;
            if(flag)
            {
                MPI_Recv(&quitting, 1, MPI_C_BOOL, 1, 103, MPI_COMM_WORLD, &status); 
            }

            auto& buf = grille.getStatistiques();
            buf = statistiques;
            output << jours_écoulés << "\t" << grille.nombreTotalContaminésGrippe() << "\t"
                  << grille.nombreTotalContaminésAgentPathogène() << std::endl;

            end = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end-start;
            compt += elapsed_seconds.count();
            jours_comptés += 1;
            if (quitting)
            {
                std::cout << "Temps moyen affichage d'un pas de temps "<<compt/jours_comptés<< std::endl;
            }
            
        }
        output.close();
    }

    else
    {
        unsigned int graine_aléatoire = 1;
        std::uniform_real_distribution<double> porteur_pathogène(0.,1.);

        épidémie::ContexteGlobal contexte;
        contexte.interactions.β = 60.;
        std::vector<épidémie::Individu> population_loc;
        auto taille_loc = contexte.taux_population/row_size;
        population_loc.reserve(taille_loc);

        épidémie::Grille grille{contexte.taux_population};
        auto [largeur_grille,hauteur_grille] = grille.dimension();

        épidémie::AgentPathogène agent(graine_aléatoire++);

        auto début = row_rank*taille_loc;
        auto fin = (row_rank+1)*taille_loc;
        for (std::size_t i = début; i < fin; ++i )
        {
            std::default_random_engine motor(100*(i+1));
            population_loc.emplace_back(graine_aléatoire+i, contexte.espérance_de_vie, contexte.déplacement_maximal);
            population_loc.back().setPosition(largeur_grille, hauteur_grille);
            if (porteur_pathogène(motor) < 0.2)
            {
                population_loc.back().estContaminé(agent);
            }
        }
        graine_aléatoire+=row_size*taille_loc;
        std::size_t jours_écoulés = 0;
        int         jour_apparition_grippe = 0;
        int         nombre_immunisés_grippe_loc = (taille_loc*23)/100;

        sdl2::event_queue queue;
        bool quitting = false;
        //std::ofstream output("Courbe.dat");
        if(row_rank==0)
        {
            //output << "# jours_écoulés \t nombreTotalContaminésGrippe \t nombreTotalContaminésAgentPathogène()" << std::endl;
            std::cout << "Début boucle épidémie" << std::endl << std::flush;
        }
        
        épidémie::Grippe grippe(0);
        double compt = 0;
 
	    while (!quitting)
	    {
	    	std::chrono::time_point<std::chrono::system_clock> start, end;
    		start = std::chrono::system_clock::now();

	        auto events = queue.pull_events();
	        for ( const auto& e : events)
	        {
	            if (e->kind_of_event() == sdl2::event::quit)
	                quitting = true;
	        }

            if (jours_écoulés%365 == 0)
            {
                grippe = épidémie::Grippe(jours_écoulés/365);
                jour_apparition_grippe = grippe.dateCalculImportationGrippe();
                grippe.calculNouveauTauxTransmission();

                #pragma omp parallel for schedule(dynamic, 1000)
                for ( int ipersonne = 0; ipersonne < nombre_immunisés_grippe_loc; ++ipersonne)
                {
                    population_loc[ipersonne].devientImmuniséGrippe();
                }
                #pragma omp parallel for schedule(dynamic, 1000)
                for ( int ipersonne = nombre_immunisés_grippe_loc; ipersonne < int(taille_loc); ++ipersonne )
                {
                    population_loc[ipersonne].redevientSensibleGrippe();
                }
            }
            if (jours_écoulés%365 == std::size_t(jour_apparition_grippe))
            {
                int fin;
                if (nombre_immunisés_grippe_loc+25 < int(taille_loc))
                {
                    fin = nombre_immunisés_grippe_loc+25;
                }
                else
                {
                    fin = taille_loc;
                }
                for (int ipersonne = nombre_immunisés_grippe_loc; ipersonne < fin; ++ipersonne )
                {
                    population_loc[ipersonne].estContaminé(grippe);
                }
            }

	        màjStatistique(grille.getStatistiques(), population_loc, largeur_grille);
            auto statistiques_loc = grille.getStatistiques();

            int wait;
            std::vector<épidémie::Grille::StatistiqueParCase> statistiques(largeur_grille*hauteur_grille);
            MPI_Allreduce(statistiques_loc.data(), statistiques.data(), 3*largeur_grille*hauteur_grille, MPI_INT, MPI_SUM, row_comm);

            auto& buf = grille.getStatistiques();
            buf = statistiques;

            if(row_rank==0)
            {
                int flag = 0;
                MPI_Iprobe(0, 102, MPI_COMM_WORLD, &flag, &status);
                if(flag)
                {
                    MPI_Irecv(&wait, 1, MPI_INT, 0, 102, MPI_COMM_WORLD, &request);
                    MPI_Send(statistiques.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 0, jours_écoulés, MPI_COMM_WORLD);
                }
            }

	        std::size_t compteur_grippe_loc = 0, compteur_agent_loc = 0, mouru_loc = 0;
	        auto& personne = population_loc;
            int taille_pop = (int)population_loc.size();

            #pragma omp parallel for schedule(dynamic, 1000)
	        for ( int i = 0; i < taille_pop; i++ )
	        {
	            if (personne[i].testContaminationGrippe(grille, contexte.interactions, grippe, agent))
	            {
	                compteur_grippe_loc ++;
	                personne[i].estContaminé(grippe);
	            }
	            if (personne[i].testContaminationAgent(grille, agent))
	            {
	                compteur_agent_loc ++;
	                personne[i].estContaminé(agent);
	            }
	            if (personne[i].doitMourir())
	            {
	                mouru_loc++;
	                unsigned nouvelle_graine = jours_écoulés + personne[i].position().x*personne[i].position().y;
	                personne[i] = épidémie::Individu(nouvelle_graine, contexte.espérance_de_vie, contexte.déplacement_maximal);
	                personne[i].setPosition(largeur_grille, hauteur_grille);
	            }
	            personne[i].veillirDUnJour();
	            personne[i].seDéplace(grille);
	        }

            int compteur_grippe, compteur_agent, mouru;
            MPI_Reduce(&compteur_grippe_loc, &compteur_grippe, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, row_comm);
            MPI_Reduce(&compteur_agent_loc, &compteur_agent, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, row_comm);
            MPI_Reduce(&mouru_loc, &mouru, 1, MPI_UNSIGNED_LONG, MPI_SUM, 0, row_comm);

	        jours_écoulés += 1;

	        if ((row_rank==0) && quitting)
	        {
                int flag = 0;
	        	MPI_Iprobe(0, 102, MPI_COMM_WORLD, &flag, &status);
                std::cout<<"FLAG = "<<flag<<std::endl;
	    		if(flag)
	    		{
	    			MPI_Irecv(&wait, 1, MPI_INT, 0, 102, MPI_COMM_WORLD, &request);
	    			MPI_Send(statistiques.data(), 3*largeur_grille*hauteur_grille, MPI_INT, 0, jours_écoulés, MPI_COMM_WORLD);
	    		}
	        	MPI_Send(&quitting, 1, MPI_C_BOOL, 0, 103, MPI_COMM_WORLD);
	        }

	        end = std::chrono::system_clock::now();
		    std::chrono::duration<double> elapsed_seconds = end-start;
		    compt += elapsed_seconds.count();

            if(quitting)
            {
                std::cout<<"Temps moyen du processus "<<row_rank<<" = "<<compt/jours_écoulés<<std::endl;
            }

	    }// Fin boucle temporelle
	    //std::cout << "Temps moyen simulation d'un pas de temps "<<compt/jours_écoulés<< std::endl;
	}
}

int main(int argc, char* argv[])
{
	MPI_Init(&argc, &argv);
	int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // parse command-line
    bool affiche = true;
    for (int i=0; i<argc; i++) {
      std::cout << i << " " << argv[i] << "\n";
      if (std::string("-nw") == argv[i]) affiche = false;
    }
  
    sdl2::init(argc, argv);
    {
        simulation(affiche);
        std::cout<<"OUT "<< rank <<std::endl;
    }
    sdl2::finalize();
    MPI_Finalize();
    return EXIT_SUCCESS;
}