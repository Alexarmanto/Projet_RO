/**
 * @file projet_ro.h
 * @brief En-tête pour le projet de Recherche Opérationnelle (Problème de Transport).
 * Contient les définitions des structures de données et les prototypes des fonctions.
 * Version complète avec améliorations 1 et 2 implémentées.
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
 * @param p Le problème à afficher.
 */
void afficherTableauCouts(ProblemeTransport* p);

/**
 * @brief Affiche la matrice de la solution actuelle (B) formatée dans la console.
 * @param p Le problème à afficher.
 */
void afficherTableauSolution(ProblemeTransport* p);

/**
 * @brief Calcule le coût total de la solution actuelle (Fonction Z).
 * Formule : Z = Somme( A[i][j] * B[i][j] )
 * @param p Le problème.
 * @return Le coût total (double).
 */
double calculerCoutTotal(ProblemeTransport* p);

/**
 * @brief Affiche la table des potentiels E(s) et E(t).
 * @param p Le problème.
 * @param E_s Tableau des potentiels des fournisseurs.
 * @param E_t Tableau des potentiels des clients.
 */
void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t);

/**
 * @brief Affiche la table des coûts marginaux.
 * Affiche les opportunités d'amélioration pour les cases hors-base.
 * @param p Le problème.
 * @param couts_marginaux Matrice des coûts marginaux.
 * @param base Matrice binaire identifiant les variables de base.
 */
void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base);


// ==========================================================
// 4. ALGORITHMES DE SOLUTION INITIALE
// ==========================================================

/**
 * @brief Génère une solution initiale via la méthode du Coin Nord-Ouest.
 * Algorithme simple mais ne garantissant pas une solution proche de l'optimal.
 * Affiche toutes les étapes de construction.
 * @param p Le problème à résoudre.
 */
void algoNordOuest(ProblemeTransport* p);

/**
 * @brief Version silencieuse de Nord-Ouest (sans affichage détaillé).
 * Utilisée dans les pipelines (options 3 et 4) pour éviter la redondance.
 * @param p Le problème à résoudre.
 */
void algoNordOuestSilencieux(ProblemeTransport* p);

/**
 * @brief Génère une solution initiale via la méthode de Balas-Hammer (approximation de Vogel).
 * Utilise les pénalités pour choisir les cases prioritaires.
 * Affiche toutes les itérations avec pénalités et choix de cases.
 * @param p Le problème à résoudre.
 */
void algoBalasHammer(ProblemeTransport* p);

/**
 * @brief Version silencieuse de Balas-Hammer (sans affichage détaillé).
 * Utilisée dans les pipelines (options 3 et 4) pour éviter la redondance.
 * @param p Le problème à résoudre.
 */
void algoBalasHammerSilencieux(ProblemeTransport* p);


// ==========================================================
// 5. ALGORITHME DU MARCHE-PIED (OPTIMISATION)
// ==========================================================

/**
 * @brief Algorithme principal du Marche-Pied (Stepping-Stone).
 * Itère pour améliorer la solution initiale jusqu'à l'optimalité.
 * Affiche toutes les itérations avec détection de cycles, connexité, et coûts.
 * @param p Le problème contenant une solution initiale admissible.
 */
void algoMarchePied(ProblemeTransport* p);

/**
 * @brief Vérifie si la solution est dégénérée et résout les problèmes de cycles/connexité.
 * Implémente l'AMÉLIORATION 1 :
 * - Détection et résolution répétée de TOUS les cycles
 * - Ajout d'arêtes par coûts croissants pour garantir la connexité
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @return true si la dégénérescence a été traitée, false sinon.
 */
bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base);

/**
 * @brief Teste la connexité du graphe et affiche les composantes connexes.
 * Utilise un parcours BFS pour identifier les composantes.
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @return true si connexe, false sinon.
 */
bool testerConnexite(ProblemeTransport* p, int** base);

/**
 * @brief Calcule les potentiels des lignes (E_s) et des colonnes (E_t) sur la base actuelle.
 * Système d'équations : E_s[i] - E_t[j] = A[i][j] pour toute case (i,j) de la base.
 * Utilise un parcours BFS en fixant E_s[0] = 0 comme référence.
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @param E_s Tableau des potentiels des fournisseurs (sortie).
 * @param E_t Tableau des potentiels des clients (sortie).
 */
void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t);

/**
 * @brief Calcule les coûts marginaux pour les variables hors-base.
 * Détermine si une amélioration est possible (Critère de Dantzig).
 * Applique la règle de Bland pour la variable entrante (parcours lexicographique).
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @param E_s Tableau des potentiels des fournisseurs.
 * @param E_t Tableau des potentiels des clients.
 * @param i_ajout Pointeur pour stocker l'indice ligne de la variable entrante.
 * @param j_ajout Pointeur pour stocker l'indice colonne de la variable entrante.
 * @param couts_marginaux Matrice des coûts marginaux (sortie).
 * @return true si une amélioration est possible (coût marginal négatif trouvé), false si optimal.
 */
bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t,
                            int* i_ajout, int* j_ajout, double** couts_marginaux);

/**
 * @brief Identifie le cycle créé par l'ajout de la variable (i_ajout, j_ajout) et met à jour la solution.
 * Affiche le cycle détecté, les conditions de maximisation (theta), et l'arête sortante.
 * Implémente l'AMÉLIORATION 2 : Gestion du cas θ = 0 (dégénérescence cyclique).
 * Applique la règle de Bland pour la variable sortante (indice minimal).
 * @param p Le problème.
 * @param base Matrice binaire identifiant les variables de base.
 * @param i_ajout Indice ligne de la variable entrante.
 * @param j_ajout Indice colonne de la variable entrante.
 */
void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout);


// ==========================================================
// 6. OUTILS AUXILIAIRES (Gestion de File / Graphes)
// ==========================================================

/**
 * @brief Crée une file (queue) de capacité donnée pour les parcours BFS.
 * @param capacity Capacité maximale de la file.
 * @return Pointeur vers la file créée.
 */
Queue* creerQueue(int capacity);

/**
 * @brief Libère la mémoire allouée pour une file.
 * @param q Pointeur vers la file à libérer.
 */
void libererQueue(Queue* q);

/**
 * @brief Ajoute un élément à la file (enqueue).
 * @param q Pointeur vers la file.
 * @param index Indice du noeud à ajouter.
 * @param parent Indice du noeud parent.
 */
void enqueue(Queue* q, int index, int parent);

/**
 * @brief Retire un élément de la file (dequeue).
 * @param q Pointeur vers la file.
 * @return Le noeud retiré (BFSNode).
 */
BFSNode dequeue(Queue* q);

/**
 * @brief Vérifie si la file est vide.
 * @param q Pointeur vers la file.
 * @return true si vide, false sinon.
 */
bool isQueueEmpty(Queue* q);

// ==========================================================
// 7. OUTILS Pour l'étude de la complexité
// ==========================================================

#ifndef ETUDE_COMPLEXITE_H
#define ETUDE_COMPLEXITE_H

/**
 * @brief Lance la batterie de tests aléatoires et génère les CSV/Python.
 */
void lancer_etude_complete();

#endif

/**
 * @brief Versions silencieuses pour l'étude de complexité.
 */
void algoMarchePiedSilencieux(ProblemeTransport* p);

bool trouverEtResoudreCycleSilencieux(ProblemeTransport* p, int** base, int i_ajout, int j_ajout);

bool testerConnexiteSilencieux(ProblemeTransport* p, int** base);
bool testerEtResoudreDegenerescenceSilencieux(ProblemeTransport* p, int** base);

#endif
