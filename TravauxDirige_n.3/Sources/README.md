

# TP2 de NOM Prénom

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`





## lscpu

Architecture:                    x86_64
CPU op-mode(s):                  32-bit, 64-bit
Byte Order:                      Little Endian
Address sizes:                   39 bits physical, 48 bits virtual
CPU(s):                          8
On-line CPU(s) list:             0-7
Thread(s) per core:              2
Core(s) per socket:              4
Socket(s):                       1
Vendor ID:                       GenuineIntel
CPU family:                      6
Model:                           165
Model name:                      Intel(R) Core(TM) i5-10300H CPU @ 2.50GHz
Stepping:                        2
CPU MHz:                         2496.007
BogoMIPS:                        4992.01
Virtualization:                  VT-x
Hypervisor vendor:               Microsoft
Virtualization type:             full
L1d cache:                       128 KiB
L1i cache:                       128 KiB
L2 cache:                        1 MiB
L3 cache:                        8 MiB
Vulnerability Itlb multihit:     KVM: Mitigation: Split huge pages
Vulnerability L1tf:              Not affected
Vulnerability Mds:               Not affected
Vulnerability Meltdown:          Not affected
Vulnerability Spec store bypass: Mitigation; Speculative Store Bypass disabled via prctl and seccomp
Vulnerability Spectre v1:        Mitigation; usercopy/swapgs barriers and __user pointer sanitization
Vulnerability Spectre v2:        Mitigation; Enhanced IBRS, IBPB conditional, RSB filling
Vulnerability Srbds:             Not affected
Vulnerability Tsx async abort:   Not affected
Flags:                           fpu vme de pse tsc msr pae mce cx8 apic sep mtrr pge mca cmov pat pse36 clflush mmx fxsr sse sse2 ss ht syscall nx pd
                                 pe1gb rdtscp lm constant_tsc rep_good nopl xtopology cpuid pni pclmulqdq vmx ssse3 fma cx16 pcid sse4_1 sse4_2 x2apic
                                  movbe popcnt aes xsave avx f16c rdrand hypervisor lahf_lm abm 3dnowprefetch invpcid_single ssbd ibrs ibpb stibp ibrs
                                 _enhanced tpr_shadow vnmi ept vpid ept_ad fsgsbase bmi1 avx2 smep bmi2 erms invpcid rdseed adx smap clflushopt xsaveo
                                 pt xsavec xgetbv1 xsaves flush_l1d arch_capabilities

*Des infos utiles s'y trouvent : nb core, taille de cache*



## Produit scalaire 

*Expliquer les paramètres, les fichiers, l'optimisation de compil, NbSamples, ...*

OMP_NUM    | samples=1024 | assemblage |    dot    |  speedup  |  efficacité
-----------|--------------|------------|-----------|-----------|--------------
séquentiel |                                            
1          |                  0.83s        0.29s         1
2          |                  0.77s        0.13s        1.55
3          |                  0.82s        0.14s        1.55
4          |                  0.75s        0.10s        1.66             
8          |                  0.88s        0.13s        1.55 


*Discuter sur ce qu'on observe, la logique qui s'y cache.*
On est dans un cas de memory down car on est limité par la vitesse d'accès en mémoire des processeurs. A partir de 2/4 processeurs, on ira pas plus vite.



## Produit matrice-matrice
1. Le produit matrice-matrice naif donne
  dimension  |   temps 
-------------|-----------
    1023     |   1.47s
    1024     |   3.27s
    1025     |   1.46s


### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

`make TestProduct.exe && ./TestProduct.exe 1024`


  ordre           | time    | MFlops  | MFlops(n=2048) 
------------------|---------|---------|----------------
i,j,k (origine)   | 3.99163 | 537.996 |     166.41          
j,i,k             | 4.12565 | 520.52  |    
i,k,j             | 24.4884 |  87.694 |    
k,i,j             | 25.3043 |  84.8662|    
j,k,i             | 0.859461| 2498.64 |    
k,j,i             | 0.966147| 2222.73 |    


*Discussion des résultats*



### OMP sur la meilleure boucle 

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

  OMP_NUM         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
1                 | 3110.54 |    2942.09     |     3483.13    |    2836.29                            
2                 | 5294.58 |    5113.79     |     4683       |    4586.66                          
3                 | 7763.27 |    6358.81     |     10799.2    |    5678.53                        
4                 | 8705.58 |    7730.14     |     9872.87    |    6044.99                         
5                 | 10508.3 |    8978.15     |     12543.1    |    7067.19                                      
6                 | 11608.2 |    9741.84     |     12814.8    |    8415.41                                     
7                 | 12365.3 |    10739.7     |     13112.8    |    9747.24                                     
8                 | 12931.9 |    11739       |     13374      |    7867.05                                 




### Produit par blocs

`make TestProduct.exe && ./TestProduct.exe 1024`

  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
origine (=max)    |  |
32                |  |
64                |  |
128               |  |
256               |  |
512               |  | 
1024              |  |




### Bloc + OMP



  szBlock      | OMP_NUM | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
---------------|---------|---------|------------------------------------------------
A.nbCols       |  1      |         | 
512            |  8      |         | 







# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
