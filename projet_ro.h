// Fichier: projet_ro.h
// "Sommaire" des fonctions

#ifndef PROJET_RO_H
#define PROJET_RO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Pour memcpy
#include <float.h>  // Pour DBL_MAX
#include <stdbool.h> // <-- ✅ AJOUTE CELLE-CI

// 1. STRUCTURE DE DONNEES
// ... (Structure ProblemeTransport) ...

// ... (le reste de ton fichier .h) ...

// 1. STRUCTURE DE DONNEES
typedef struct {
    int n; // Nb. fournisseurs
    int m; // Nb. clients

    double** A; // Matrice des couts (n x m)
    int* P;     // Vecteur des provisions (taille n)
    int* C;     // Vecteur des commandes (taille m)
    int** B;     // Matrice de la proposition de transport (solution) (n x m)

} ProblemeTransport;


// 2. PROTOTYPES DES FONCTIONS (Etape 1)

/**
 * @brief Alloue la memoire pour un nouveau probleme de transport.
 * Initialise la matrice de solution B a 0.
 */
ProblemeTransport* creerProbleme(int n, int m);

/**
 * @brief Libere toute la memoire allouee pour le probleme.
 */
void libererProbleme(ProblemeTransport* p);

/**
 * @brief Lit le fichier .txt et remplit la structure ProblemeTransport.
 */
ProblemeTransport* lireDonnees(const char* nomFichier);

/**
 * @brief Affiche la matrice des couts (A) de maniere alignee.
 */
void afficherTableauCouts(ProblemeTransport* p);

/**
 * @brief Affiche la matrice de solution (B) de maniere alignee.
 */
void afficherTableauSolution(ProblemeTransport* p);


// 3. PROTOTYPES DES FONCTIONS (Etape 2)  // <-- AJOUTER TOUTE CETTE SECTION

/**
 * @brief Reinitialise la matrice de solution B a 0.
 * Bonne pratique avant de lancer un nouvel algorithme.
 */
void reinitialiserSolution(ProblemeTransport* p);

/**
 * @brief TACHE 2: Calcule le cout total de la solution B actuelle.
 * Z = Somme( A[i][j] * B[i][j] )
 */
double calculerCoutTotal(ProblemeTransport* p);

/**
 * @brief TACHE 1: Remplit la matrice B en utilisant la methode du Coin Nord-Ouest.
 */
void algoNordOuest(ProblemeTransport* p);

/**
 * @brief TACHE 3: Remplit la matrice B en utilisant la methode de Balas-Hammer (Vogel).
 */
void algoBalasHammer(ProblemeTransport* p);


// ==========================================================
// 4. STRUCTURES ET PROTOTYPES (ETAPE 3)
// ==========================================================

// --- Structures pour le Graphe (BFS) ---

/**
 * @brief Noeud pour la file (Queue) du BFS.
 * Contient l'index du noeud et son parent (pour trouver les cycles).
 */
typedef struct {
    int index;  // 0 a n-1 (Fourn.), n a n+m-1 (Clients)
    int parent; // Index du noeud parent dans le parcours
} BFSNode;

/**
 * @brief Structure de File (FIFO) simple pour le BFS.
 */
typedef struct {
    BFSNode* data;
    int front;
    int rear;
    int capacity;
} Queue;

// --- Prototypes des Outils Graphe (BFS) ---

Queue* creerQueue(int capacity);
void libererQueue(Queue* q);
void enqueue(Queue* q, int index, int parent);
BFSNode dequeue(Queue* q);
bool isQueueEmpty(Queue* q);

// --- Prototypes des Fonctions Logiques (Marche-Pied) ---

/**
 * @brief TACHE 6: Fonction principale de l'algorithme du marche-pied.
 * Orchestre la boucle d'optimisation.
 * @param p Le probleme.
 * @param solutionInitiale La solution de B (obtenue par N-O ou B-H).
 */
void algoMarchePied(ProblemeTransport* p);

/**
 * @brief TACHE 6.1 & 6.2: Teste la degenerescence (connexite et cycles).
 * Utilise le BFS. [cite: 2438, 2446, 2457, 2459]
 * @param p Le probleme.
 * @param base Matrice temporaire [n][m] pour marquer les aretes de la base.
 * @return true si le graphe est non-degenere (un arbre), false sinon.
 */
bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base);

/**
 * @brief TACHE 6.3: Calcule les potentiels E(s) et E(t).
 * (Sera implementee plus tard)
 */
// Fichier: projet_ro.h
// ... (Toutes les declarations precedentes) ...

// --- Prototypes des Fonctions Logiques (Marche-Pied) ---
// ... (algoMarchePied, testerEtResoudreDegenerescence) ...
void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t);
bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t, int* i_ajout, int* j_ajout, double** couts_marginaux);void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout);

// --- NOUVEAUX PROTOTYPES (AFFICHAGE ETAPE 3) ---

/**
 * @brief TACHE 6.4: Affiche la table des couts potentiels E(s) - E(t).
 * Correspond au tableau 1.5.3 du cours.
 */
void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t);

/**
 * @brief TACHE 6.4: Affiche la table des couts marginaux d(s,t).
 * Correspond au tableau 1.5.4 du cours.
 */
void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base);


#endif //PROJET_RO_H