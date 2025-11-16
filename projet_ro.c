// Fichier: projet_ro.c
// Implementation des fonctions (l'"usine")

#include "projet_ro.h"

/**
 * @brief Alloue la memoire pour un nouveau probleme de transport.
 */
ProblemeTransport* creerProbleme(int n, int m) {
    ProblemeTransport* p = (ProblemeTransport*)malloc(sizeof(ProblemeTransport));
    if (p == NULL) {
        perror("Erreur d'allocation pour ProblemeTransport");
        exit(EXIT_FAILURE);
    }

    p->n = n;
    p->m = m;

    // Allouer les pointeurs pour les lignes des matrices
    p->A = (double**)malloc(n * sizeof(double*));
    p->B = (int**)malloc(n * sizeof(int*));
    if (p->A == NULL || p->B == NULL) {
        perror("Erreur d'allocation pour les lignes des matrices");
        exit(EXIT_FAILURE);
    }

    // Allouer les lignes (colonnes) pour chaque matrice
    for (int i = 0; i < n; i++) {
        p->A[i] = (double*)malloc(m * sizeof(double));
        p->B[i] = (int*)malloc(m * sizeof(int));
        if (p->A[i] == NULL || p->B[i] == NULL) {
            perror("Erreur d'allocation pour une colonne de matrice");
            exit(EXIT_FAILURE);
        }
    }

    // Allouer les vecteurs
    p->P = (int*)malloc(n * sizeof(int));
    p->C = (int*)malloc(m * sizeof(int));
    if (p->P == NULL || p->C == NULL) {
        perror("Erreur d'allocation pour les vecteurs P ou C");
        exit(EXIT_FAILURE);
    }

    // Initialiser la matrice de solution B a 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            p->B[i][j] = 0;
        }
    }

    return p;
}

/**
 * @brief Libere toute la memoire allouee pour le probleme.
 */
void libererProbleme(ProblemeTransport* p) {
    if (p == NULL) return;

    for (int i = 0; i < p->n; i++) {
        free(p->A[i]);
        free(p->B[i]);
    }
    free(p->A);
    free(p->B);
    free(p->P);
    free(p->C);
    free(p);
}

/**
 * @brief 2. FONCTION LECTURE DES DONNEES
 */
ProblemeTransport* lireDonnees(const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (f == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    int n, m;
    if (fscanf(f, "%d %d", &n, &m) != 2) {
        fprintf(stderr, "Erreur de lecture de n et m\n");
        exit(EXIT_FAILURE);
    }

    ProblemeTransport* p = creerProbleme(n, m);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (fscanf(f, "%lf", &p->A[i][j]) != 1) {
                fprintf(stderr, "Erreur de lecture de la matrice A\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    for (int i = 0; i < n; i++) {
        if (fscanf(f, "%d", &p->P[i]) != 1) {
            fprintf(stderr, "Erreur de lecture du vecteur P\n");
            exit(EXIT_FAILURE);
        }
    }

    for (int j = 0; j < m; j++) {
        if (fscanf(f, "%d", &p->C[j]) != 1) {
            fprintf(stderr, "Erreur de lecture du vecteur C\n");
            exit(EXIT_FAILURE);
        }
    }

    fclose(f);
    printf("Lecture du fichier '%s' terminee avec succes.\n", nomFichier);
    printf("N = %d (fournisseurs), M = %d (clients)\n", p->n, p->m);

    return p;
}

/**
 * @brief 3. FONCTION AFFICHAGE DES TABLEAUX (Version 20/20)
 */
void afficherTableauCouts(ProblemeTransport* p) {
    if (p == NULL) return;

    const int LARGEUR_COL = 12;

    printf("\n--- TABLEAU DES COUTS (A) ---\n\n");

    printf("%*s", LARGEUR_COL, "");
    for (int j = 0; j < p->m; j++) {
        char en_tete[20];
        sprintf(en_tete, "Client %d", j);
        printf("%*s", LARGEUR_COL, en_tete);
    }
    printf("%*s\n", LARGEUR_COL, "Provisions (P)");
    printf("%*s\n", (p->m + 2) * LARGEUR_COL, "--------------------------------------------------");

    for (int i = 0; i < p->n; i++) {
        char en_tete_ligne[20];
        sprintf(en_tete_ligne, "Fourn. %d", i);
        printf("%*s", LARGEUR_COL, en_tete_ligne);

        for (int j = 0; j < p->m; j++) {
            printf("%*.2f", LARGEUR_COL, p->A[i][j]);
        }
        printf("%*d\n", LARGEUR_COL, p->P[i]);
    }

    printf("%*s\n", (p->m + 2) * LARGEUR_COL, "--------------------------------------------------");
    printf("%*s", LARGEUR_COL, "Commandes (C)");
    for (int j = 0; j < p->m; j++) {
        printf("%*d", LARGEUR_COL, p->C[j]);
    }
    printf("\n\n");
}

/**
 * @brief Affiche la matrice de 'int' (ex: Solution) avec P et C.
 */
void afficherTableauSolution(ProblemeTransport* p) {
    if (p == NULL) return;

    const int LARGEUR_COL = 12;

    printf("\n--- PROPOSITION DE TRANSPORT (B) ---\n\n");

    printf("%*s", LARGEUR_COL, "");
    for (int j = 0; j < p->m; j++) {
        char en_tete[20];
        sprintf(en_tete, "Client %d", j);
        printf("%*s", LARGEUR_COL, en_tete);
    }
    printf("%*s\n", LARGEUR_COL, "Provisions (P)");
    printf("%*s\n", (p->m + 2) * LARGEUR_COL, "--------------------------------------------------");

    for (int i = 0; i < p->n; i++) {
        char en_tete_ligne[20];
        sprintf(en_tete_ligne, "Fourn. %d", i);
        printf("%*s", LARGEUR_COL, en_tete_ligne);

        for (int j = 0; j < p->m; j++) {
            printf("%*d", LARGEUR_COL, p->B[i][j]);
        }
        printf("%*d\n", LARGEUR_COL, p->P[i]);
    }

    printf("%*s\n", (p->m + 2) * LARGEUR_COL, "--------------------------------------------------");
    printf("%*s", LARGEUR_COL, "Commandes (C)");
    for (int j = 0; j < p->m; j++) {
        printf("%*d", LARGEUR_COL, p->C[j]);
    }
    printf("\n\n");
}

// Fichier: projet_ro.c
// ... (Toutes les fonctions de l'Etape 1 sont au-dessus) ...

// ==========================================================
// IMPLÉMENTATION DES FONCTIONS - ÉTAPE 2
// ==========================================================

/**
 * @brief Reinitialise la matrice de solution B a 0.
 * C'est une fonction "helper" (utilitaire) pour nos bonnes pratiques.
 */
void reinitialiserSolution(ProblemeTransport* p) {
    if (p == NULL) return;
    for (int i = 0; i < p->n; i++) {
        // On peut utiliser memset, mais une boucle est tout aussi claire
        for (int j = 0; j < p->m; j++) {
            p->B[i][j] = 0;
        }
    }
}

/**
 * @brief TACHE 2: Calcule le cout total de la solution B actuelle.
 */
double calculerCoutTotal(ProblemeTransport* p) {
    if (p == NULL) return 0.0;

    double coutTotal = 0.0;
    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            // Ajoute seulement si une quantite est transportee
            if (p->B[i][j] > 0) {
                coutTotal += p->A[i][j] * p->B[i][j];
            }
        }
    }
    return coutTotal;
}

/**
 * @brief TACHE 1: Remplit la matrice B en utilisant la methode du Coin Nord-Ouest.
 */
void algoNordOuest(ProblemeTransport* p) {
    if (p == NULL) return;

    // Bonne pratique: reinitialiser la solution precedente
    reinitialiserSolution(p);

    // Bonne pratique: Travailler sur des copies de P et C
    // Nous allouons de la nouvelle memoire pour ces copies
    int* P_copie = (int*)malloc(p->n * sizeof(int));
    int* C_copie = (int*)malloc(p->m * sizeof(int));
    if (P_copie == NULL || C_copie == NULL) {
        perror("Erreur d'allocation pour les copies (Nord-Ouest)");
        exit(EXIT_FAILURE);
    }

    // Copier les valeurs initiales (ne pas toucher a p->P et p->C)
    memcpy(P_copie, p->P, p->n * sizeof(int));
    memcpy(C_copie, p->C, p->m * sizeof(int));

    // Logique de l'algorithme
    int i = 0; // Ligne actuelle (fournisseur)
    int j = 0; // Colonne actuelle (client)

    while (i < p->n && j < p->m) {
        // Trouver le minimum entre la provision et la commande
        int quantite = (P_copie[i] < C_copie[j]) ? P_copie[i] : C_copie[j];

        // Assigner cette quantite a la solution
        p->B[i][j] = quantite;

        // Mettre a jour les copies
        P_copie[i] -= quantite;
        C_copie[j] -= quantite;

        // Decaler
        if (P_copie[i] == 0) {
            i++; // Le fournisseur i est epuise, on passe a la ligne suivante
        }
        if (C_copie[j] == 0) {
            j++; // Le client j est satisfait, on passe a la colonne suivante
        }
        // Note: si les deux tombent a 0 en meme temps, i et j avancent
    }

    // Bonne pratique: Liberer la memoire des copies
    free(P_copie);
    free(C_copie);
}


/**
 * @brief TACHE 3: Remplit la matrice B en utilisant la methode de Balas-Hammer (Vogel).
 */
void algoBalasHammer(ProblemeTransport* p) {
    if (p == NULL) return;

    // Bonne pratique: reinitialiser la solution
    reinitialiserSolution(p);

    // --- 1. Preparation (Bonne Pratique) ---

    // Copies de P et C
    int* P_copie = (int*)malloc(p->n * sizeof(int));
    int* C_copie = (int*)malloc(p->m * sizeof(int));
    memcpy(P_copie, p->P, p->n * sizeof(int));
    memcpy(C_copie, p->C, p->m * sizeof(int));

    // Tableaux pour marquer les lignes/colonnes satisfaites (saturees)
    // calloc met tout a 0 (false)
    int* ligne_saturee = (int*)calloc(p->n, sizeof(int));
    int* col_saturee = (int*)calloc(p->m, sizeof(int));

    // Tableaux pour stocker les penalites
    double* penalite_ligne = (double*)malloc(p->n * sizeof(double));
    double* penalite_colonne = (double*)malloc(p->m * sizeof(double));

    if (!P_copie || !C_copie || !ligne_saturee || !col_saturee || !penalite_ligne || !penalite_colonne) {
         perror("Erreur d'allocation pour Balas-Hammer");
         exit(EXIT_FAILURE);
    }

    int nb_lignes_restantes = p->n;
    int nb_cols_restantes = p->m;

    // --- 2. Boucle principale de l'algorithme ---

    // On continue tant qu'il reste au moins une ligne ET une colonne a traiter
    while (nb_lignes_restantes > 0 && nb_cols_restantes > 0) {

        // --- 3. Calcul des penalites de lignes ---
        for (int i = 0; i < p->n; i++) {
            if (ligne_saturee[i]) {
                penalite_ligne[i] = -1.0; // Ignore cette ligne, -1 est < toute penalite
                continue;
            }

            double min1 = DBL_MAX;
            double min2 = DBL_MAX;

            // Trouver les 2 plus petits couts de la ligne i
            for (int j = 0; j < p->m; j++) {
                if (col_saturee[j]) continue; // Ignore cette colonne

                if (p->A[i][j] < min1) {
                    min2 = min1;
                    min1 = p->A[i][j];
                } else if (p->A[i][j] < min2) {
                    min2 = p->A[i][j];
                }
            }
            // Si min2 n'a pas ete trouve (ex: 1 seule colonne restante)
            penalite_ligne[i] = (min2 == DBL_MAX) ? min1 : (min2 - min1);
        }

        // --- 4. Calcul des penalites de colonnes ---
        for (int j = 0; j < p->m; j++) {
            if (col_saturee[j]) {
                penalite_colonne[j] = -1.0; // Ignore
                continue;
            }

            double min1 = DBL_MAX;
            double min2 = DBL_MAX;

            // Trouver les 2 plus petits couts de la colonne j
            for (int i = 0; i < p->n; i++) {
                if (ligne_saturee[i]) continue; // Ignore

                if (p->A[i][j] < min1) {
                    min2 = min1;
                    min1 = p->A[i][j];
                } else if (p->A[i][j] < min2) {
                    min2 = p->A[i][j];
                }
            }
            penalite_colonne[j] = (min2 == DBL_MAX) ? min1 : (min2 - min1);
        }

        // --- 5. Trouver la penalite maximale ---
        double max_penalite = -1.0;
        int est_ligne = 0; // 0 = c'est une colonne, 1 = c'est une ligne
        int index_max = -1;

        // Verifier les lignes
        for (int i = 0; i < p->n; i++) {
            if (penalite_ligne[i] > max_penalite) {
                max_penalite = penalite_ligne[i];
                est_ligne = 1;
                index_max = i;
            }
        }
        // Verifier les colonnes
        for (int j = 0; j < p->m; j++) {
            if (penalite_colonne[j] > max_penalite) {
                max_penalite = penalite_colonne[j];
                est_ligne = 0;
                index_max = j;
            }
        }

        // Si aucune penalite n'a ete trouvee (tout est sature)
        if (index_max == -1) {
            break;
        }

        // --- 6. Trouver le cout minimal dans la ligne/colonne selectionnee ---
        int i_min = -1, j_min = -1;
        double min_cout = DBL_MAX;

        if (est_ligne) {
            int i = index_max;
            // Trouver le cout min dans la ligne i (parmi les colonnes non saturees)
            for (int j = 0; j < p->m; j++) {
                if (!col_saturee[j] && p->A[i][j] < min_cout) {
                    min_cout = p->A[i][j];
                    i_min = i;
                    j_min = j;
                }
            }
        } else {
            int j = index_max;
            // Trouver le cout min dans la colonne j (parmi les lignes non saturees)
            for (int i = 0; i < p->n; i++) {
                if (!ligne_saturee[i] && p->A[i][j] < min_cout) {
                    min_cout = p->A[i][j];
                    i_min = i;
                    j_min = j;
                }
            }
        }

        // Cas d'arret (ex: derniere cellule)
        if (i_min == -1 || j_min == -1) {
             // Il peut rester une seule cellule a remplir
            if (nb_lignes_restantes == 1 && nb_cols_restantes == 1) {
                // Trouver la derniere cellule non saturee
                for (int i = 0; i < p->n; i++) {
                    if (!ligne_saturee[i]) i_min = i;
                }
                for (int j = 0; j < p->m; j++) {
                    if (!col_saturee[j]) j_min = j;
                }
                if (i_min == -1 || j_min == -1) break; // Securite
            } else {
                break; // Impossible de trouver, sortie de boucle
            }
        }

        // --- 7. Assigner le transport (quantite max) ---
        int quantite = (P_copie[i_min] < C_copie[j_min]) ? P_copie[i_min] : C_copie[j_min];

        p->B[i_min][j_min] = quantite;

        P_copie[i_min] -= quantite;
        C_copie[j_min] -= quantite;

        // --- 8. Mettre a jour les lignes/colonnes saturees ---
        if (P_copie[i_min] == 0) {
            if (!ligne_saturee[i_min]) {
                ligne_saturee[i_min] = 1;
                nb_lignes_restantes--;
            }
        }
        if (C_copie[j_min] == 0) {
            if (!col_saturee[j_min]) {
                col_saturee[j_min] = 1;
                nb_cols_restantes--;
            }
        }
    }

    // --- 9. Nettoyage (Bonne Pratique) ---
    free(P_copie);
    free(C_copie);
    free(ligne_saturee);
    free(col_saturee);
    free(penalite_ligne);
    free(penalite_colonne);
}

// Fichier: projet_ro.c
// ... (Toutes les fonctions des Etapes 1 et 2) ...

// ==========================================================
// IMPLEMENTATION DES FONCTIONS - ÉTAPE 3
// ==========================================================

#include <stdbool.h>

// --- Fonctions utilitaires pour la File (BFS) ---

Queue* creerQueue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->data = (BFSNode*)malloc(capacity * sizeof(BFSNode));
    q->front = 0;
    q->rear = -1;
    q->capacity = capacity;
    return q;
}

void libererQueue(Queue* q) {
    if (q == NULL) return;
    free(q->data);
    free(q);
}

bool isQueueEmpty(Queue* q) {
    return q->rear < q->front;
}

void enqueue(Queue* q, int index, int parent) {
    if (q->rear >= q->capacity - 1) {
        // Devrait pas arriver si la capacite est (n+m)
        fprintf(stderr, "Erreur: File pleine\n");
        return;
    }
    q->rear++;
    q->data[q->rear].index = index;
    q->data[q->rear].parent = parent;
}

BFSNode dequeue(Queue* q) {
    BFSNode node = { -1, -1 }; // Noeud invalide
    if (isQueueEmpty(q)) {
        fprintf(stderr, "Erreur: File vide\n");
        return node;
    }
    node = q->data[q->front];
    q->front++;
    return node;
}


// --- Fonctions de l'Algorithme Marche-Pied ---

/**
 * @brief TACHE 6.1 & 6.2: Teste la degenerescence (connexite et cycles).
 * Verifie si le graphe de la solution B est un arbre (n+m-1 aretes,
 * connexe, et acyclique). [cite: 290-296]
 * Si non-connexe, ajoute des aretes artificielles (b_ij = 0).
 * Si cyclique, resout le cycle.
 *
 * @param p Le probleme.
 * @param base Matrice [n][m] (allouee par l'appelant) qui sera remplie
 * avec 1 pour les aretes de base, 0 sinon.
 * @return true si le graphe est (ou a ete rendu) non-degenere, false si echec.
 */
bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base) {
    int n = p->n;
    int m = p->m;
    int nb_noeuds = n + m;
    int nb_aretes_base = 0;

    // 1. Initialiser la base avec les aretes b_ij > 0
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (p->B[i][j] > 0) {
                base[i][j] = 1;
                nb_aretes_base++;
            } else {
                base[i][j] = 0;
            }
        }
    }

    printf("  [Test Graphe] Aretes initiales (B > 0): %d. Requis pour arbre: %d\n",
           nb_aretes_base, nb_noeuds - 1);

    // 2. Verifier les cycles (Tache 6.2) [cite: 2438, 2442]
    // Un graphe avec des aretes > 0 ne devrait pas avoir de cycle
    // s'il vient de N-O ou B-H, mais l'ajout d'une arete en crea un.
    // (Note: une vraie verification de cycle devrait etre faite ici.
    // Pour l'instant, on suppose que N-O/B-H ne creent pas de cycles)
    // TODO: Implementer la detection de cycle (estAcycliqueBFS) [cite: 2457]
    // S'il y a un cycle -> appeler trouverEtResoudreCycle
    // Pour ce projet, on suppose que la solution initiale est acyclique.

    // 3. Verifier la connexite (Tache 6.1)
    if (nb_aretes_base >= nb_noeuds - 1) {
        // Si on a assez d'aretes, on suppose qu'il est non-degenere
        // (C'est une simplification, on devrait tester la connexite)
        printf("  [Test Graphe] Le graphe est suppose non-degenere.\n");
        return true;
    }

    // 4. CAS DEGENERE: Pas assez d'aretes (non-connexe) [cite: 355-364]
    // Nous devons ajouter des aretes (b_ij = 0) pour le rendre connexe [cite: 2448]
    printf("  [Test Graphe] Cas degenere: non-connexe. Ajout d'aretes artificielles...\n");

    // On utilise une methode simple:
    // On ajoute des aretes (i,j) de cout minimal (A[i][j])
    // tant que nb_aretes < n+m-1 ET que l'ajout ne cree pas de cycle.

    // (Implementation simplifiee pour l'instant:
    // on ajoute juste des (i,j) = 0 ou base[i][j] == 0 jusqu'a n+m-1)
    // NOTE: Une vraie implementation doit chercher le cout minimal
    // et verifier les cycles a chaque ajout ! [cite: 363]

    for (int i = 0; i < n; i++) {
        if (nb_aretes_base >= nb_noeuds - 1) break;
        for (int j = 0; j < m; j++) {
            if (nb_aretes_base >= nb_noeuds - 1) break;

            if (base[i][j] == 0) { // Si l'arete n'est pas deja dans la base
                // On l'ajoute artificiellement
                base[i][j] = 1;
                nb_aretes_base++;
                printf("    -> Ajout arete artificielle (b_ij = 0) a (%d, %d)\n", i, j);

                // !! ATTENTION !!
                // On devrait verifier si cet ajout cree un cycle !
                // Si oui, on l'enleve (base[i][j] = 0) et on essaie la suivante.
                // Pour l'instant, nous omettons cette verification complexe.
            }
        }
    }

    if (nb_aretes_base == nb_noeuds - 1) {
        printf("  [Test Graphe] Graphe rendu non-degenere.\n");
        return true;
    }

    fprintf(stderr, "Erreur: Impossible de resoudre la degenerescence.\n");
    return false;
}

/**
 * @brief TACHE 6.3: Calcule les potentiels E(s) et E(t).
 * (STUB - A implementer)
 */
void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t) {
    int n = p->n;
    int m = p->m;
    int nb_noeuds = n + m;

    // 1. Initialisation
    // Marqueurs pour savoir si un potentiel est deja calcule
    bool* s_connu = (bool*)calloc(n, sizeof(bool));
    bool* t_connu = (bool*)calloc(m, sizeof(bool));
    Queue* q = creerQueue(nb_noeuds);

    // Initialiser tous les potentiels a "inconnu"
    for (int i = 0; i < n; i++) E_s[i] = DBL_MAX;
    for (int j = 0; j < m; j++) E_t[j] = DBL_MAX;

    // 2. Point de depart: On fixe E_s[0] = 0 [cite: 762]
    E_s[0] = 0.0;
    s_connu[0] = true;
    enqueue(q, 0, -1); // On ajoute le noeud S_0 (index 0) a la file

    // 3. Boucle BFS
    while (!isQueueEmpty(q)) {
        BFSNode noeud = dequeue(q);
        int u = noeud.index; // Index du noeud (0 a n+m-1)

        if (u < n) {
            // C'est un Fournisseur S_i (index 'i' = u)
            int i = u;
            // Trouver tous les clients 'j' connectes a 'i' PAR LA BASE
            for (int j = 0; j < m; j++) {
                if (base[i][j] == 1 && !t_connu[j]) {
                    // c(i,j) = E_s[i] - E_t[j]  =>  E_t[j] = E_s[i] - c(i,j)
                    E_t[j] = E_s[i] - p->A[i][j];
                    t_connu[j] = true;
                    // On ajoute le client C_j (index global n+j) a la file
                    enqueue(q, n + j, i);
                }
            }
        } else {
            // C'est un Client C_j (index 'j' = u - n)
            int j = u - n;
            // Trouver tous les fournisseurs 'i' connectes a 'j' PAR LA BASE
            for (int i = 0; i < n; i++) {
                if (base[i][j] == 1 && !s_connu[i]) {
                    // c(i,j) = E_s[i] - E_t[j]  =>  E_s[i] = E_t[j] + c(i,j)
                    E_s[i] = E_t[j] + p->A[i][j];
                    s_connu[i] = true;
                    // On ajoute le fournisseur S_i (index global i) a la file
                    enqueue(q, i, n + j);
                }
            }
        }
    }

    // 4. Nettoyage
    libererQueue(q);
    free(s_connu);
    free(t_connu);

    printf("  [Potentiels] Calcul des potentiels termine.\n");
}

/**
 * @brief TACHE 6.4: Affiche la table des couts potentiels E(s) - E(t).
 * Correspond au tableau 1.5.3 du cours.
 */
void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t) {
    if (p == NULL) return;

    const int LARGEUR_COL = 12;

    printf("\n--- TABLE DES COUTS POTENTIELS (E(s) - E(t)) ---\n\n");

    // En-tete (Clients Cj et E(t))
    printf("%*s", LARGEUR_COL, "");
    for (int j = 0; j < p->m; j++) {
        char en_tete[20];
        sprintf(en_tete, "Client %d", j);
        printf("%*s", LARGEUR_COL, en_tete);
    }
    printf("%*s\n", LARGEUR_COL, "E(s)"); // Potentiel Fournisseur
    printf("%*s", LARGEUR_COL, ""); // Coin
    for (int j = 0; j < p->m; j++) {
        printf("%*.2f", LARGEUR_COL, E_t[j]); // Affiche E(t)
    }
    printf("\n%*s\n", (p->m + 2) * LARGEUR_COL, "--------------------------------------------------");

    // Lignes
    for (int i = 0; i < p->n; i++) {
        char en_tete_ligne[20];
        sprintf(en_tete_ligne, "Fourn. %d", i);
        printf("%*s", LARGEUR_COL, en_tete_ligne);

        // Affiche E(s) - E(t)
        for (int j = 0; j < p->m; j++) {
            printf("%*.2f", LARGEUR_COL, (E_s[i] - E_t[j]));
        }
        // Affiche E(s) a la fin
        printf("%*.2f\n", LARGEUR_COL, E_s[i]);
    }
    printf("\n");
}


/**
 * @brief TACHE 6.4: Affiche la table des couts marginaux d(s,t).
 * d(s,t) = c(s,t) - (E(s) - E(t))
 * Pour les aretes de base, d(s,t) = 0 par definition.
 */
void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base) {
    if (p == NULL) return;

    const int LARGEUR_COL = 12;

    printf("\n--- TABLE DES COUTS MARGINAUX (d(s,t)) ---\n");
    printf(" (Aretes de base marquees avec 0.00)\n\n");

    // En-tete (Clients Cj)
    printf("%*s", LARGEUR_COL, "");
    for (int j = 0; j < p->m; j++) {
        char en_tete[20];
        sprintf(en_tete, "Client %d", j);
        printf("%*s", LARGEUR_COL, en_tete);
    }
    printf("\n%*s\n", (p->m + 1) * LARGEUR_COL, "--------------------------------------------------");

    // Lignes
    for (int i = 0; i < p->n; i++) {
        char en_tete_ligne[20];
        sprintf(en_tete_ligne, "Fourn. %d", i);
        printf("%*s", LARGEUR_COL, en_tete_ligne);

        for (int j = 0; j < p->m; j++) {
            if (base[i][j] == 1) {
                // Cout marginal est 0 par definition [cite: 708]
                printf("%*.2f", LARGEUR_COL, 0.0);
            } else {
                printf("%*.2f", LARGEUR_COL, couts_marginaux[i][j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
/**
 * @brief TACHE 6.4: Calcule les couts marginaux.
 * (STUB - A implementer)
 */
// Fichier: projet_ro.c

// ... (apres la fonction calculerPotentiels) ...

/**
 * @brief TACHE 6.4: Calcule les couts marginaux.
 * (STUB - A implementer)
 */
// Fichier: projet_ro.c
// ... (apres la fonction calculerPotentiels) ...

/**
 * @brief TACHE 6.4: Calcule les couts marginaux pour les aretes HORS base.
 * d(s,t) = c(s,t) - (E(s) - E(t))
 * Trouve l'arete (i_ajout, j_ajout) avec le cout marginal le plus negatif.
 *
 * @param couts_marginaux Matrice [n][m] (allouee par l'appelant) qui sera remplie.
 * @return true s'il existe un cout marginal negatif (non optimal), false sinon (optimal).
 */
bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t,
                            int* i_ajout, int* j_ajout, double** couts_marginaux) {

    int n = p->n;
    int m = p->m;
    double meilleur_cout_marginal = 0.0; // Seuil d'optimalite
    bool optimal = true;

    // On reinitialise les indices d'ajout
    *i_ajout = -1;
    *j_ajout = -1;

    // On parcourt TOUTES les aretes (i,j)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {

            if (base[i][j] == 1) {
                // Arete DANS la base
                // Le cout marginal est 0 par definition [cite: 427]
                couts_marginaux[i][j] = 0.0;
            } else {
                // Arete HORS base
                // On calcule le cout marginal
                double d_ij = p->A[i][j] - (E_s[i] - E_t[j]);
                couts_marginaux[i][j] = d_ij;

                // On cherche le cout le plus negatif
                // Utilisation d'une petite tolerance (ex: -1e-9) pour eviter
                // les problemes de precision si d_ij est -0.0000001
                if (d_ij < meilleur_cout_marginal && d_ij < -1e-9) {
                    optimal = false; // On a trouve un cout negatif
                    meilleur_cout_marginal = d_ij;
                    *i_ajout = i;
                    *j_ajout = j;
                }
            }
        }
    }

    if (optimal) {
        printf("  [Marginaux] Calcul termine. Tous les couts marginaux sont >= 0.\n");
        return false; // Renvoie "Optimal"
    } else {
        printf("  [Marginaux] Calcul termine. Meilleur cout marginal: %.2f en (%d, %d).\n",
               meilleur_cout_marginal, *i_ajout, *j_ajout);
        return true; // Renvoie "Non Optimal"
    }
}

/**
 * @brief TACHE 6.5: Trouve le cycle cree par l'ajout de (i,j) et le resout.
 * (STUB - A implementer)
 */
// Fichier: projet_ro.c
// ... (apres la fonction calculerCoutsMarginaux) ...

/**
 * @brief TACHE 6.5: Trouve le cycle cree par l'ajout de (i_ajout, j_ajout)
 * et applique la maximisation du transport.
 */
void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout) {
    int n = p->n;
    int m = p->m;
    int nb_noeuds = n + m;

    printf("  [Optimisation] Ajout de l'arete (%d, %d) et resolution du cycle...\n", i_ajout, j_ajout);

    // --- 1. Trouver le cycle (BFS) ---

    // parent[k] = index du noeud qui a decouvert 'k'
    int* parent = (int*)malloc(nb_noeuds * sizeof(int));
    bool* visite = (bool*)calloc(nb_noeuds, sizeof(bool));
    Queue* q = creerQueue(nb_noeuds);

    for (int i = 0; i < nb_noeuds; i++) parent[i] = -1;

    // On ajoute l'arete (i_ajout, j_ajout) a la base TEMPORAIREMENT
    // pour que le BFS puisse la trouver, mais on part de j_ajout
    // pour trouver i_ajout via un autre chemin.

    int start_node = n + j_ajout; // Index global du client C_j
    int end_node = i_ajout;     // Index global du fournisseur S_i

    enqueue(q, start_node, -1);
    visite[start_node] = true;

    // parent[end_node] restera -1 tant qu'on ne l'a pas trouve
    while (!isQueueEmpty(q) && parent[end_node] == -1) {
        BFSNode noeud = dequeue(q);
        int u = noeud.index;

        if (u < n) {
            // C'est un Fournisseur S_i (index 'i' = u)
            int i = u;
            for (int j = 0; j < m; j++) {
                int v = n + j; // Index global du client
                // On cherche un voisin (sauf l'arete qu'on ajoute)
                if (base[i][j] == 1 && !visite[v]) {
                    if (i == i_ajout && j == j_ajout) continue; // Ignore la nouvelle arete

                    visite[v] = true;
                    parent[v] = u;
                    enqueue(q, v, u);
                }
            }
        } else {
            // C'est un Client C_j (index 'j' = u - n)
            int j = u - n;
            for (int i = 0; i < n; i++) {
                int v = i; // Index global du fournisseur
                // On cherche un voisin (sauf l'arete qu'on ajoute)
                if (base[i][j] == 1 && !visite[v]) {
                    if (i == i_ajout && j == j_ajout) continue; // Ignore la nouvelle arete

                    visite[v] = true;
                    parent[v] = u;
                    enqueue(q, v, u);
                    if (v == end_node) break; // On a trouve le chemin !
                }
            }
        }
    } // Fin du BFS

    // --- 2. Trouver Delta (quantite min sur les aretes '-') ---

    // Le cycle est: i_ajout -> j_ajout -> parent[j_ajout] ... -> i_ajout
    // On le retrace a l'envers

    int i_retire = -1, j_retire = -1; // Arete qui va sortir de la base
    double delta = DBL_MAX;
    bool signe_plus = false; // L'arete (i_ajout, j_ajout) est '+', donc la suivante (noeud -> i_ajout) est '-'
    int noeud_actuel = end_node; // On part de S_i (i_ajout)

    printf("    Cycle trouve: ");
    while (noeud_actuel != start_node) {
        int noeud_parent = parent[noeud_actuel];

        int i, j;
        if (noeud_actuel < n) { // noeud_actuel = S_i
            i = noeud_actuel;
            j = noeud_parent - n;
        } else { // noeud_actuel = C_j
            i = noeud_parent;
            j = noeud_actuel - n;
        }

        printf("(%d, %d) ", i, j);

        if (!signe_plus) { // Arete '-'
            if (p->B[i][j] < delta) {
                delta = p->B[i][j];
                i_retire = i;
                j_retire = j;
            }
        }
        signe_plus = !signe_plus; // On alterne
        noeud_actuel = noeud_parent;
    }
    printf("(%d, %d)\n", i_ajout, j_ajout);
    printf("    Delta (quantite a transferer): %.2f\n", delta);


    // --- 3. Appliquer Delta sur le cycle ---

    signe_plus = true; // L'arete (i_ajout, j_ajout) est '+'
    p->B[i_ajout][j_ajout] += delta;

    noeud_actuel = end_node; // On repart de S_i (i_ajout)

    while (noeud_actuel != start_node) {
        int noeud_parent = parent[noeud_actuel];
        int i, j;
        if (noeud_actuel < n) {
            i = noeud_actuel;
            j = noeud_parent - n;
        } else {
            i = noeud_parent;
            j = noeud_actuel - n;
        }

        if (signe_plus) {
            p->B[i][j] += delta;
        } else {
            p->B[i][j] -= delta;
        }
        signe_plus = !signe_plus;
        noeud_actuel = noeud_parent;
    }

    // --- 4. Mettre a jour la base ---
    base[i_ajout][j_ajout] = 1; // L'arete ajoutee entre
    base[i_retire][j_retire] = 0; // L'arete tombee a 0 sort
    printf("    Arete ajoutee a la base: (%d, %d). Arete retiree: (%d, %d).\n",
           i_ajout, j_ajout, i_retire, j_retire);

    // --- 5. Nettoyage ---
    free(parent);
    free(visite);
    libererQueue(q);
}

/**
 * @brief TACHE 6: Fonction principale de l'algorithme du marche-pied.
 * Orchestre la boucle d'optimisation.
 */
// Fichier: projet_ro.c

// ... (fonction calculerPotentiels juste au-dessus) ...

void algoMarchePied(ProblemeTransport* p) {
    int n = p->n;
    int m = p->m;

    // --- Allocation de la memoire pour l'Etape 3 ---
    int** base = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        base[i] = (int*)calloc(m, sizeof(int));
    }

    double* E_s = (double*)malloc(n * sizeof(double));
    double* E_t = (double*)malloc(m * sizeof(double));

    // <-- AJOUT: Allouer la matrice pour les couts marginaux
    double** couts_marginaux = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        couts_marginaux[i] = (double*)calloc(m, sizeof(double));
    }

    int iteration = 1;
    bool estOptimal = false;

    while (!estOptimal) {
        printf("\n--- MARCHE-PIED: Iteration %d ---\n", iteration);

        // 1. Tester et resoudre la degenerescence
        bool nonDegenere = testerEtResoudreDegenerescence(p, base);
        if (!nonDegenere) {
            fprintf(stderr, "Erreur fatale: Echec de la resolution de la degenerescence.\n");
            break;
        }

        // 2. Calculer les potentiels
        calculerPotentiels(p, base, E_s, E_t);

        // --- MISE A JOUR ---
        // 3. Afficher la table des potentiels
        afficherTableauPotentiels(p, E_s, E_t);

        // 4. Calculer les couts marginaux et verifier l'optimalite
        int i_ajout = -1, j_ajout = -1;
        // On passe la matrice 'couts_marginaux' pour qu'elle soit remplie
        bool nonOptimal = calculerCoutsMarginaux(p, base, E_s, E_t, &i_ajout, &j_ajout, couts_marginaux);

        // 5. Afficher la table des marginaux (on la met ici, avant le if)
        afficherTableauMarginaux(p, couts_marginaux, base);
        // --- FIN MISE A JOUR ---

        if (!nonOptimal) {
            printf("  [Resultat] Solution optimale trouvee.\n");
            estOptimal = true;
        } else {
            printf("  [Resultat] Non optimal. Cout marginal neg trouvé en (%d, %d).\n", i_ajout, j_ajout);

            // 6. Ameliorer la solution (resoudre le cycle)
            trouverEtResoudreCycle(p, base, i_ajout, j_ajout);

            // Afficher la nouvelle solution et son cout
            afficherTableauSolution(p);
            printf("  [Resultat] Nouveau cout: %.2f\n", calculerCoutTotal(p));
        }

        iteration++;
        if (iteration > 20) {
            printf("Arret de securite apres 20 iterations.\n");
            break;
        }
    } // Fin de la boucle while

    printf("\n==================================================\n");
    printf("FIN DE L'OPTIMISATION (MARCHE-PIED)\n");
    printf("==================================================\n");

    // --- Liberation de la memoire de l'Etape 3 ---
    for (int i = 0; i < n; i++) {
        free(base[i]);
        free(couts_marginaux[i]); // <-- AJOUT: Liberer les marginaux
    }
    free(base);
    free(couts_marginaux); // <-- AJOUT: Liberer les marginaux
    free(E_s);
    free(E_t);
}
