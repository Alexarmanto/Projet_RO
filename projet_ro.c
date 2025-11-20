/**
 * @file projet_ro.c
 * @brief Implémentation des algorithmes de Recherche Opérationnelle.
 * @details Ce programme implémente:
 * - Heuristique de Balas-Hammer (Vogel) pour la solution initiale (Complexité quasi-linéaire).
 * - Méthode du Marche-Pied avec Potentiels pour l'optimisation (Algorithme du Simplexe sur graphe).
 * - Gestion stricte de la dégénérescence et règle de Bland pour garantir la terminaison.
 * - Convention de signe: Cij = Ui - Vj (Potentiels).
 */

#include "projet_ro.h"
#include <math.h>

// Seuil de tolérance pour les comparaisons flottantes
#define EPSILON 1e-9

// ==========================================================
// 0. OUTILS AUXILIAIRES (FILE / QUEUE POUR BFS)
// ==========================================================

Queue* creerQueue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) { perror("Erreur alloc queue"); exit(EXIT_FAILURE); }
    q->data = (BFSNode*)malloc(capacity * sizeof(BFSNode));
    q->front = 0;
    q->rear = -1;
    q->capacity = capacity;
    return q;
}

void libererQueue(Queue* q) {
    if (q) { free(q->data); free(q); }
}

bool isQueueEmpty(Queue* q) {
    return q->rear < q->front;
}

void enqueue(Queue* q, int index, int parent) {
    if (q->rear < q->capacity - 1) {
        q->rear++;
        q->data[q->rear].index = index;
        q->data[q->rear].parent = parent;
    }
}

BFSNode dequeue(Queue* q) {
    BFSNode node = { -1, -1 };
    if (!isQueueEmpty(q)) {
        node = q->data[q->front];
        q->front++;
    }
    return node;
}

/**
 * @brief Vérifie l'existence d'un chemin dans le graphe partiel de la base.
 * Utilisé pour garantir que l'ajout d'une variable artificielle ne crée pas de cycle.
 */
static bool cheminExiste(int n, int m, int** base, int start, int target) {
    Queue* q = creerQueue(n + m);
    bool* visite = (bool*)calloc(n + m, sizeof(bool));
    bool found = false;

    enqueue(q, start, -1);
    visite[start] = true;

    while (!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u == target) { found = true; break; }

        if (u < n) { // Source
            for (int j = 0; j < m; j++) {
                if (base[u][j] && !visite[n + j]) {
                    visite[n + j] = true; enqueue(q, n + j, u);
                }
            }
        } else { // Client
            for (int i = 0; i < n; i++) {
                if (base[i][u - n] && !visite[i]) {
                    visite[i] = true; enqueue(q, i, u);
                }
            }
        }
    }
    free(visite); libererQueue(q);
    return found;
}

// ==========================================================
// 1. GESTION MÉMOIRE ET LECTURE
// ==========================================================

ProblemeTransport* creerProbleme(int n, int m) {
    ProblemeTransport* p = (ProblemeTransport*)malloc(sizeof(ProblemeTransport));
    if (!p) exit(EXIT_FAILURE);
    p->n = n; p->m = m;
    p->A = (double**)malloc(n * sizeof(double*));
    p->B = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        p->A[i] = (double*)malloc(m * sizeof(double));
        p->B[i] = (int*)calloc(m, sizeof(int));
    }
    p->P = (int*)malloc(n * sizeof(int));
    p->C = (int*)malloc(m * sizeof(int));
    return p;
}

void libererProbleme(ProblemeTransport* p) {
    if (!p) return;
    for (int i = 0; i < p->n; i++) { free(p->A[i]); free(p->B[i]); }
    free(p->A); free(p->B); free(p->P); free(p->C);
    free(p);
}

ProblemeTransport* lireDonnees(const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (!f) { perror("Erreur ouverture fichier"); return NULL; }
    int n, m;
    if (fscanf(f, "%d %d", &n, &m) != 2) return NULL;
    ProblemeTransport* p = creerProbleme(n, m);
    for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) fscanf(f, "%lf", &p->A[i][j]);
    for (int i = 0; i < n; i++) fscanf(f, "%d", &p->P[i]);
    for (int j = 0; j < m; j++) fscanf(f, "%d", &p->C[j]);
    fclose(f);
    return p;
}

void reinitialiserSolution(ProblemeTransport* p) {
    if(!p) return;
    for (int i = 0; i < p->n; i++) for (int j = 0; j < p->m; j++) p->B[i][j] = 0;
}

// ==========================================================
// 2. AFFICHAGE
// ==========================================================

void afficherTableauCouts(ProblemeTransport* p) {
    if (!p) return;
    printf("\n--- MATRICE DES COUTS (A) ---\n       ");
    for (int j = 0; j < p->m; j++) printf(" C%-4d ", j);
    printf(" | PROV\n");
    for (int i = 0; i < p->n; i++) {
        printf(" F%-4d ", i);
        for (int j = 0; j < p->m; j++) printf("%6.2f ", p->A[i][j]);
        printf(" | %d\n", p->P[i]);
    }
    printf(" COMM  ");
    for (int j = 0; j < p->m; j++) printf("%6d ", p->C[j]);
    printf("\n");
}

void afficherTableauSolution(ProblemeTransport* p) {
    if (!p) return;
    printf("\n--- PROPOSITION DE TRANSPORT (B) ---\n       ");
    for (int j = 0; j < p->m; j++) printf(" C%-4d ", j);
    printf(" | PROV\n");
    for (int i = 0; i < p->n; i++) {
        printf(" F%-4d ", i);
        for (int j = 0; j < p->m; j++) printf("%6d ", p->B[i][j]);
        printf(" | %d\n", p->P[i]);
    }
    printf(" COMM  ");
    for (int j = 0; j < p->m; j++) printf("%6d ", p->C[j]);
    printf("\n");
}

double calculerCoutTotal(ProblemeTransport* p) {
    double total = 0.0;
    for (int i = 0; i < p->n; i++)
        for (int j = 0; j < p->m; j++) total += p->A[i][j] * p->B[i][j];
    return total;
}

void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t) {
    printf("\n--- POTENTIELS (Convention: Cij = Es - Et) ---\n       ");
    for (int j = 0; j < p->m; j++) printf(" Et%-3d ", j);
    printf("\n       ");
    for (int j = 0; j < p->m; j++) printf("%6.2f ", E_t[j]);
    printf("\n");
    for (int i = 0; i < p->n; i++) {
        printf(" Es%-3d ", i);
        for (int j = 0; j < p->m; j++) printf("   .   ");
        printf("| %6.2f\n", E_s[i]);
    }
}

void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base) {
    printf("\n--- COUTS MARGINAUX (Delta = Cij - (Es - Et)) ---\n");
    for (int i = 0; i < p->n; i++) {
        printf(" F%-4d ", i);
        for (int j = 0; j < p->m; j++) {
            if (base[i][j]) printf("   B   ");
            else printf("%6.2f ", couts_marginaux[i][j]);
        }
        printf("\n");
    }
}

// ==========================================================
// 3. ALGORITHMES INITIAUX
// ==========================================================

void algoNordOuest(ProblemeTransport* p) {
    reinitialiserSolution(p);
    int* P = (int*)malloc(p->n * sizeof(int));
    int* C = (int*)malloc(p->m * sizeof(int));
    memcpy(P, p->P, p->n * sizeof(int));
    memcpy(C, p->C, p->m * sizeof(int));

    int i = 0, j = 0;
    while (i < p->n && j < p->m) {
        int q = (P[i] < C[j]) ? P[i] : C[j];
        p->B[i][j] = q;
        P[i] -= q; C[j] -= q;

        // Gestion de la dégénérescence initiale:
        // Si P[i] et C[j] s'annulent simultanément, on n'incrémente qu'un seul indice
        // pour laisser une variable de base à 0 (epsilon) à l'itération suivante.
        if (P[i] == 0 && C[j] == 0) {
            if (i < p->n - 1) i++;
            else j++;
        }
        else if (P[i] == 0) i++;
        else j++;
    }
    free(P); free(C);
}

// Méthode de Balas-Hammer (VAM - Vogel Approximation Method)
void algoBalasHammer(ProblemeTransport* p) {
    reinitialiserSolution(p);
    int* P = (int*)malloc(p->n * sizeof(int));
    int* C = (int*)malloc(p->m * sizeof(int));
    memcpy(P, p->P, p->n * sizeof(int));
    memcpy(C, p->C, p->m * sizeof(int));

    int* row_sat = (int*)calloc(p->n, sizeof(int));
    int* col_sat = (int*)calloc(p->m, sizeof(int));
    int n_rows = p->n, n_cols = p->m;

    while (n_rows > 0 && n_cols > 0) {
        double max_pen = -1.0;
        int idx_best = -1;
        bool is_row = true;

        // Calcul Pénalités Lignes (Différence entre les 2 plus petits coûts)
        for (int i = 0; i < p->n; i++) {
            if (row_sat[i]) continue;
            double m1 = DBL_MAX, m2 = DBL_MAX;
            for (int j = 0; j < p->m; j++) {
                if (!col_sat[j]) {
                    if (p->A[i][j] < m1) { m2 = m1; m1 = p->A[i][j]; }
                    else if (p->A[i][j] < m2) { m2 = p->A[i][j]; }
                }
            }
            double pen = (m2 == DBL_MAX) ? m1 : (m2 - m1);
            if (pen > max_pen) { max_pen = pen; idx_best = i; is_row = true; }
        }

        // Calcul Pénalités Colonnes
        for (int j = 0; j < p->m; j++) {
            if (col_sat[j]) continue;
            double m1 = DBL_MAX, m2 = DBL_MAX;
            for (int i = 0; i < p->n; i++) {
                if (!row_sat[i]) {
                    if (p->A[i][j] < m1) { m2 = m1; m1 = p->A[i][j]; }
                    else if (p->A[i][j] < m2) { m2 = p->A[i][j]; }
                }
            }
            double pen = (m2 == DBL_MAX) ? m1 : (m2 - m1);
            // En cas d'égalité, on priorise arbitrairement les colonnes (ou indices faibles)
            if (pen > max_pen) { max_pen = pen; idx_best = j; is_row = false; }
        }

        // Sélection de la case min-coût dans la ligne/colonne choisie
        int r = -1, c = -1;
        double min_cost = DBL_MAX;

        if (is_row) {
            r = idx_best;
            for (int j = 0; j < p->m; j++) {
                if (!col_sat[j] && p->A[r][j] < min_cost) { min_cost = p->A[r][j]; c = j; }
            }
        } else {
            c = idx_best;
            for (int i = 0; i < p->n; i++) {
                if (!row_sat[i] && p->A[i][c] < min_cost) { min_cost = p->A[i][c]; r = i; }
            }
        }

        if (r == -1 || c == -1) break;

        int q = (P[r] < C[c]) ? P[r] : C[c];
        p->B[r][c] = q;
        P[r] -= q; C[c] -= q;

        // Saturation intelligente: Ne jamais saturer Ligne ET Colonne simultanément
        // sauf à la toute fin, pour préserver la connexité (base de taille n+m-1).
        if (P[r] == 0 && C[c] == 0) {
            if (n_rows > 1 || n_cols > 1) {
                row_sat[r] = 1; n_rows--;
            } else {
                row_sat[r] = 1; col_sat[c] = 1;
                n_rows--; n_cols--;
            }
        }
        else if (P[r] == 0) { row_sat[r] = 1; n_rows--; }
        else if (C[c] == 0) { col_sat[c] = 1; n_cols--; }
    }
    free(P); free(C); free(row_sat); free(col_sat);
}

// ==========================================================
// 4. MARCHE-PIED (OPTIMISATION)
// ==========================================================

// Complétion de la base pour obtenir un arbre (Graphe connexe sans cycle)
// Théorème: Une solution de base doit avoir n+m-1 arêtes.
bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int nb_aretes = 0;
    for(int i=0; i<n; i++) for(int j=0; j<m; j++) if(base[i][j]) nb_aretes++;

    int requis = n + m - 1;
    if (nb_aretes >= requis) return false;

    // Ajout de variables artificielles (epsilon) pour restaurer la connexité
    for(int i=0; i<n && nb_aretes < requis; i++) {
        for(int j=0; j<m && nb_aretes < requis; j++) {
            if (!base[i][j]) {
                // On n'ajoute l'arête que si elle ne crée pas de cycle
                if (!cheminExiste(n, m, base, i, n + j)) {
                    base[i][j] = 1; // Variable de base (quantité 0)
                    nb_aretes++;
                }
            }
        }
    }
    return true;
}

void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t) {
    for(int i=0; i<p->n; i++) E_s[i] = DBL_MAX;
    for(int j=0; j<p->m; j++) E_t[j] = DBL_MAX;

    Queue* q = creerQueue(p->n + p->m);

    // Fixation du potentiel de référence (Liberté de jauge)
    E_s[0] = 0.0;
    enqueue(q, 0, -1);

    while(!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u < p->n) { // Source i
            for (int j = 0; j < p->m; j++) {
                if (base[u][j] && E_t[j] == DBL_MAX) {
                    // Cours: Cij = Es[i] - Et[j]  => Et[j] = Es[i] - Cij
                    E_t[j] = E_s[u] - p->A[u][j];
                    enqueue(q, p->n + j, u);
                }
            }
        } else { // Client j
            int j = u - p->n;
            for (int i = 0; i < p->n; i++) {
                if (base[i][j] && E_s[i] == DBL_MAX) {
                    // Cours: Cij = Es[i] - Et[j] => Es[i] = Cij + Et[j]
                    E_s[i] = p->A[i][j] + E_t[j];
                    enqueue(q, i, u);
                }
            }
        }
    }
    libererQueue(q);
}

// Critère de Dantzig avec règle de Bland implicite (premier trouvé)
bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t,
                            int* i_ajout, int* j_ajout, double** couts_marginaux) {
    double meilleur_gain = -EPSILON;
    *i_ajout = -1; *j_ajout = -1;
    bool amelioration = false;

    // Parcours lexicographique (i, puis j)
    // Si plusieurs gains sont égaux, on prend le premier rencontré (Règle de Bland pour variable entrante)
    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            if (base[i][j]) {
                couts_marginaux[i][j] = 0.0;
            } else {
                // Delta = Cij - (Es - Et)
                double delta = p->A[i][j] - (E_s[i] - E_t[j]);
                couts_marginaux[i][j] = delta;

                if (delta < meilleur_gain) {
                    meilleur_gain = delta;
                    *i_ajout = i;
                    *j_ajout = j;
                    amelioration = true;
                }
            }
        }
    }
    return amelioration;
}

// Fonction critique: Recherche du cycle et mise à jour des flux
// IMPLEMENTATION DE LA REGLE DE BLAND POUR LA VARIABLE SORTANTE
void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout) {
    int n = p->n, m = p->m;
    Queue* q = creerQueue(n + m);
    int* parent = (int*)malloc((n + m) * sizeof(int));
    bool* visite = (bool*)calloc(n + m, sizeof(bool));

    int start = n + j_ajout; // Client
    int target = i_ajout;    // Source

    enqueue(q, start, -1);
    visite[start] = true;
    bool found = false;

    // 1. BFS pour trouver le cycle
    while(!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u == target) { found = true; break; }
        if (u < n) {
            for (int j = 0; j < m; j++) if (base[u][j] && !visite[n + j]) {
                visite[n + j] = true; parent[n + j] = u; enqueue(q, n + j, u);
            }
        } else {
            for (int i = 0; i < n; i++) if (base[i][u - n] && !visite[i]) {
                visite[i] = true; parent[i] = u; enqueue(q, i, u);
            }
        }
    }

    if (found) {
        // Le cycle alterne (+, -, +, -). L'arête entrante est (+).
        // On cherche theta = min(Flux) sur les arêtes (-).
        double theta = DBL_MAX;
        int i_out = -1, j_out = -1;

        int curr = target;
        bool is_neg = true;

        // 2. Premier parcours pour trouver Theta et la variable sortante (Règle de Bland)
        while(curr != start) {
            int par = parent[curr];
            int r, c;
            if (curr < n) { r = curr; c = par - n; } else { r = par; c = curr - n; }

            if (is_neg) {
                int val = p->B[r][c];
                if (val < theta) {
                    theta = val;
                    i_out = r; j_out = c;
                }
                // REGLE DE BLAND (Anti-cyclage):
                // En cas d'égalité de theta (flux limitant), on sort la variable avec le plus petit indice.
                // Indice composite = r * m + c (ordre lexicographique strict)
                else if (val == theta) {
                    if (i_out != -1) {
                        int current_id = r * m + c;
                        int best_id = i_out * m + j_out;
                        if (current_id < best_id) {
                            i_out = r; j_out = c;
                        }
                    }
                }
            }
            is_neg = !is_neg;
            curr = par;
        }

        // 3. Mise à jour des flux
        p->B[i_ajout][j_ajout] += (int)theta;
        base[i_ajout][j_ajout] = 1;

        curr = target;
        is_neg = true;
        while(curr != start) {
            int par = parent[curr];
            int r, c;
            if (curr < n) { r = curr; c = par - n; } else { r = par; c = curr - n; }

            if (is_neg) p->B[r][c] -= (int)theta;
            else p->B[r][c] += (int)theta;
            is_neg = !is_neg;
            curr = par;
        }

        // 4. Sortie de base
        if (i_out != -1) base[i_out][j_out] = 0;
    }
    free(parent); free(visite); libererQueue(q);
}

void algoMarchePied(ProblemeTransport* p) {
    int n = p->n, m = p->m;
    int** base = (int**)malloc(n * sizeof(int*));
    double** couts_marginaux = (double**)malloc(n * sizeof(double*));
    for(int i=0; i<n; i++) {
        base[i] = (int*)calloc(m, sizeof(int));
        couts_marginaux[i] = (double*)calloc(m, sizeof(double));
    }
    double* E_s = (double*)malloc(n * sizeof(double));
    double* E_t = (double*)malloc(m * sizeof(double));

    // Initialisation de la base
    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
            if (p->B[i][j] > 0) base[i][j] = 1;

    int iter = 0;
    bool optimal = false;

    // Boucle du Simplexe
    while (!optimal && iter < 1000) {
        iter++;
        testerEtResoudreDegenerescence(p, base);
        calculerPotentiels(p, base, E_s, E_t);
        afficherTableauPotentiels(p, E_s, E_t);

        int i_in, j_in;
        bool possible = calculerCoutsMarginaux(p, base, E_s, E_t, &i_in, &j_in, couts_marginaux);
        afficherTableauMarginaux(p, couts_marginaux, base);

        if (!possible) {
            optimal = true;
            printf("\n>>> Solution Optimale Trouvee (Iteration %d).\n", iter);
        } else {
            printf("\n[Iter %d] Amelioration via (%d, %d) (Gain marginal: %.2f)\n",
                   iter, i_in, j_in, couts_marginaux[i_in][j_in]);
            trouverEtResoudreCycle(p, base, i_in, j_in);
            afficherTableauSolution(p); // Afficher l'évolution
        }
    }

    for(int i=0; i<n; i++) { free(base[i]); free(couts_marginaux[i]); }
    free(base); free(couts_marginaux); free(E_s); free(E_t);
}
