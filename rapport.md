# PCO25 LAB05

## Autheur : Mauros Santos, Gabriel Bader

## Overview

Ce projet consiste à modéliser et implémenter un système de gestion de vélos en libre-service dans une ville, en utilisant des moniteurs de Mesa
## Choix de conceptions

### Moniteur de Mesa

Dans le contexte de ce laboratoire, il est essentiel de gérer correctement l’accès concurrent aux bornes de vélo : les habitants, tout comme le van chargé du restockage, doivent accéder simultanément aux mêmes ressources (les vélos et les places disponibles).
Pour ceci, on a mis en place cette classe :

``` c++
class BikeStation
{
public:
    /**
     * @brief Mutex that secure the critical section
     */
    PcoMutex mutex;

    /**
     * @brief condition of hasVTT - notify when there is a VTT available
     */
    PcoConditionVariable hasVTT;

    /**
     * @brief condition of hasRoad - notify when there is a Road available
     */
    PcoConditionVariable hasRoad;
    /**
    * @brief condition of hasRoad - notify when there is a hasGravel available
    */
    PcoConditionVariable hasGravel;
    
    // ........
    
private:
    /**
    * @brief when emergency stop, check this condition to return the right way
    */
    bool ended = false;


```

Avec les attributs mutex, hasVTT, hasROad, hasGravel et ended qu'on va justifier ci-dessous:

#### Mutex

Notre mutex est notre protection qui permetre de gérér l'accès concurant. Plus précisement on va protéger l'accès de :

- std::vector<Bike*> bikes;
- const size_t capacity;

`bikes` : est un vecteur de bike, qui va stocker les vélos actuellement à la bike station. On va donc devoir protéger tout les accès en lecture/écriture afin de garantir l'intégrité de ce tableau. Par exemple, lorsqu'on veut prendre un vélo, il ne faut pas que ce vélo soit déja pris par un autre thread (people).

#### PcoConditionVariable

Nous avons décider de faire 3 variables différentes qui représente chaque type de vélo. L'objectif étant de garantir une liste "FIFO" par type de vélo. Cela nous permet que lorsque une personne arrive en premier, mais que sont type de vélo n'est pas disponible, une deuxième personne peut tout de meme prendre sont vélo sans que l'ensembe du système soit bloqué. Cela nous évite aussi de devoir reveiller tous les threads et qu'ils se rendorme si y'a pas leur type de vélo.

Donc on a :

- PcoConditionVariable hasVTT
- PcoConditionVariable hasRoad;
- PcoConditionVariable hasGravel;

Qui définissent chaque catégorie de vélo.


#### bool ended

Cette variable est un flag qui nous permet de savoir lorsque nous voulons stoper la simulation de prévenir tout les threads et de les arreters correctement.