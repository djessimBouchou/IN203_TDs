#include<chrono>
#include<random>
#include<iostream>
//Attention,ne marche qu'en C++11 ou supérieur : 
double approximate_pi(unsigned long nbSamples) 
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning=myclock::now();
    myclock::duration d = myclock::now() -beginning;
    unsigned seed= d.count();
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
    //Number of nbDarts throwed in the unit disk 
    double ratio=double(nbDarts)/double(nbSamples);
    return ratio;
}

int main()
{
    
    double pi = approximate_pi(100000);
    std::cout << "La valeur approchée vaut : " << 4*pi << std::endl;
}