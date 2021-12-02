# TP2 de BOUCHOU Djessim

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## Mandelbrot 

Nous allons découper l'image de manière équitable et chaque processus aura donc une partie de l'image à traiter (de Hauteur/Nombre de processeurs lignes).

           | Taille image : 800 x 600 |   Speed Up
-----------+--------------------------+----------------
séquentiel |            17.68s        |       1
1          |            17.62s        |       1
2          |            08.90s        |      1.98  
3          |            06.67s        |      2.64     
4          |            05.20s        |      3.39  
8          |            05.18s        |      3.40     


*Discuter sur ce qu'on observe, la logique qui s'y cache.*
Le temps d'exécution n'est pas divisé exactement par 2 lorsqu'on passe de 2 processus à 4 processus (chaque processus ne met pas le même temps à traiter une image). 
Le temps est intéressant avec 2 processeurs, mais pour 3 et 4 on est limité par le temps d'accès à la mémoire.
Le temps pour 4 procs et 8 procs est le même car nous sommes dans un cas de memory bound.


*Expliquer votre stratégie pour faire une partition dynamique des lignes de l'image entre chaque processus*

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |              
1          |              
2          |              
3          |              
4          |              
8          |              



## Produit matrice-vecteur



*Expliquer la façon dont vous avez calculé la dimension locale sur chaque processus, en particulier quand le nombre de processus ne divise pas la dimension de la matrice.*

Dans le cas où la dimension de l'espace n'est pas divisible par le nombre de processus, il suffit de donner au dernier processus le reste de la division. 
