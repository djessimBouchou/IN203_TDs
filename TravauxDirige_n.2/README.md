# TP2 de NOM Prénom

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## Mandelbrot 

Nous allons découper l'image de manière équitable et chaque processus aura donc une partie de l'image à traiter (de Hauteur/Nombre de processeurs lignes).

           | Taille image : 800 x 600 | 
-----------+---------------------------
séquentiel |            17.68s
1          |            17.62s
2          |            08.90s  
3          |            06.67s  
4          |            05.20s  
8          |            05.18s  


*Discuter sur ce qu'on observe, la logique qui s'y cache.*
Le temps d'exécution n'est pas divisé exactement par 2 lorsqu'on passe de 2 processus à 4 processus (chaque processus ne met pas le même temps à traiter une image). 

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
