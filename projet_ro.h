/**
 * @file projet_ro.h
 * @brief En-tête pour le projet de Recherche Opérationnelle (Problème de Transport).
 * Contient les définitions des structures de données et les prototypes des fonctions.
 */

#ifndef PROJET_RO_H
#define PROJET_RO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>

// ==========================================================
// 1. STRUCTURES DE DONNEES
// ==========================================================

/**
 * @brief Structure représentant une instance du problème de transport.
 */
typedef struct {
    int n;       // Nombre de fournisseurs (lignes)
    int m;       // Nombre de clients (colonnes)

    double** A;  // Matrice des coûts unitaires (taille n x m)
    int* P;      // Vecteur des provisions/offres (taille n)
    int* C;      // Vecteur des commandes/demandes (taille m)
    int** B;     // Matrice de la solution (quantités transportées) (taille n x m)
} ProblemeTransport;

/**
 * @brief Noeud pour la file (Queue) utilisée dans le parcours BFS.
 * Sert à détecter les cycles et vérifier la connexité.
 */
typedef struct {
    int index;  // Indice du noeud (0 à n-1 pour fournisseurs, n à n+m-1 pour clients)
    int parent; // Indice du noeud parent dans l'arborescence
} BFSNode;

/**
 * @brief Structure de File (FIFO) simple pour l'algorithme BFS.
 */
typedef struct {
    BFSNode* data; // Tableau dynamique des noeuds
    int front;     // Tête de file
    int rear;      // Queue de file
    int capacity;  // Capacité maximale
} Queue;


// ==========================================================
// 2. GESTION DE LA MÉMOIRE ET INITIALISATION
// ==========================================================

/**
 * @brief Alloue la mémoire pour un nouveau problème de transport.
 * Initialise la matrice de solution B à 0.
 * @param n Nombre de fournisseurs.
 * @param m Nombre de clients.
 * @return Pointeur vers la structure allouée.
 */
ProblemeTransport* creerProbleme(int n, int m);

/**
 * @brief Libère toute la mémoire allouée pour le problème (matrices et vecteurs).
 * @param p Pointeur vers le problème à libérer.
 */
void libererProbleme(ProblemeTransport* p);

/**
 * @brief Lit un fichier de données et construit la structure ProblemeTransport associée.
 * Le fichier doit respecter le format défini par le sujet.
 * @param nomFichier Chemin vers le fichier .txt.
 * @return Pointeur vers le problème chargé, ou NULL en cas d'erreur.
 */
ProblemeTransport* lireDonnees(const char* nomFichier);

/**
 * @brief Réinitialise la matrice de solution B à 0.
 * Utile avant d'exécuter un nouvel algorithme sur les mêmes données.
 * @param p Le problème.
 */
void reinitialiserSolution(ProblemeTransport* p);


// ==========================================================
// 3. AFFICHAGE ET ANALYSE
// ==========================================================

/**
 * @brief Affiche la matrice des coûts unitaires (A) formatée dans la console.
 */
void afficherTableauCouts(ProblemeTransport* p);

/**
 * @brief Affiche la matrice de la solution actuelle (B) formatée dans la console.
 */
void afficherTableauSolution(ProblemeTransport* p);

/**
 * @brief Calcule le coût total de la solution actuelle (Fonction Z).
 * Formule : Z = Somme( A[i][j] * B[i][j] )
 * @return Le coût total (double).
 */
double calculerCoutTotal(ProblemeTransport* p);

/**
 * @brief Affiche la table des potentiels E(s) et E(t).
 */
void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t);

/**
 * @brief Affiche la table des coûts marginaux.
 * Affiche les opportunités d'amélioration pour les cases hors-base.
 */
void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base);


// ==========================================================
// 4. ALGORITHMES DE SOLUTION INITIALE
// ==========================================================

/**
 * @brief Génère une solution initiale via la méthode du Coin Nord-Ouest.
 * Algorithme simple mais ne garantissant pas une solution proche de l'optimal.
 * @param p Le problème à résoudre.
 */
void algoNordOuest(ProblemeTransport* p);

/**
 * @brief Génère une solution initiale via la méthode de Balas-Hammer (approximation de Vogel).
 * Utilise les pénalités pour choisir les cases prioritaires.
 * @param p Le problème à résoudre.
 */
void algoBalasHammer(ProblemeTransport* p);


// ==========================================================
// 5. ALGORITHME DU MARCHE-PIED (OPTIMISATION)
// ==========================================================

/**
 * @brief Algorithme principal du Marche-Pied (Stepping-Stone).
 * Itère pour améliorer la solution initiale jusqu'à l'optimalité.
 * @param p Le problème contenant une solution initiale admissible.
 */
void algoMarchePied(ProblemeTransport* p);

/**
 * @brief Vérifie si la solution est dégénérée (graphe non connexe ou nombre d'arêtes < n+m-1).
 * Ajoute des quantités epsilon (0) artificielles si nécessaire pour rendre la base connexe.
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @return true si la dégénérescence a été traitée, false sinon.
 */
bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base);

/**
 * @brief Calcule les potentiels des lignes (E_s) et des colonnes (E_t) sur la base actuelle.
 * Système d'équations : E_s[i] - E_t[j] = A[i][j] pour toute case (i,j) de la base.
 */
void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t);

/**
 * @brief Calcule les coûts marginaux pour les variables hors-base.
 * Détermine si une amélioration est possible.
 * @param i_ajout, j_ajout Pointeurs pour stocker les indices de la variable entrante (le meilleur gain).
 * @return true si une amélioration est possible (coût marginal négatif trouvé), false si optimal.
 */
bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t, int* i_ajout, int* j_ajout, double** couts_marginaux);

/**
 * @brief Identifie le cycle créé par l'ajout de la variable (i_ajout, j_ajout) et met à jour la solution.
 * Ajuste les quantités le long du cycle (+theta, -theta).
 */
void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout);


// ==========================================================
// 6. OUTILS AUXILIAIRES (Gestion de File / Graphes)
// ==========================================================

Queue* creerQueue(int capacity);
void libererQueue(Queue* q);
void enqueue(Queue* q, int index, int parent);
BFSNode dequeue(Queue* q);
bool isQueueEmpty(Queue* q);

#endif // PROJET_RO_H
