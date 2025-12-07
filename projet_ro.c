/**
 * @file projet_ro.c
 * @brief Implémentation des algorithmes de Recherche Opérationnelle.
 * Version complète avec tous les affichages demandés par le sujet.
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

        if (u < n) {
            for (int j = 0; j < m; j++) {
                if (base[u][j] && !visite[n + j]) {
                    visite[n + j] = true; enqueue(q, n + j, u);
                }
            }
        } else {
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

    printf("\n=== ALGORITHME NORD-OUEST ===\n");

    int i = 0, j = 0;
    int etape = 1;

    while (i < p->n && j < p->m) {
        int q = (P[i] < C[j]) ? P[i] : C[j];
        p->B[i][j] = q;
        P[i] -= q; C[j] -= q;

        printf("Etape %d: Case (%d,%d) = %d (Prov_rest=%d, Comm_rest=%d)\n",
               etape++, i, j, q, P[i], C[j]);

        if (P[i] == 0 && C[j] == 0) {
            if (i < p->n - 1) i++;
            else j++;
        }
        else if (P[i] == 0) i++;
        else j++;
    }

    printf("=== FIN NORD-OUEST ===\n");
    free(P); free(C);
}

// Version silencieuse pour les pipelines (options 3 et 4)
void algoNordOuestSilencieux(ProblemeTransport* p) {
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

        if (P[i] == 0 && C[j] == 0) {
            if (i < p->n - 1) i++;
            else j++;
        }
        else if (P[i] == 0) i++;
        else j++;
    }

    free(P); free(C);
}

void algoBalasHammer(ProblemeTransport* p) {
    reinitialiserSolution(p);
    int* P = (int*)malloc(p->n * sizeof(int));
    int* C = (int*)malloc(p->m * sizeof(int));
    memcpy(P, p->P, p->n * sizeof(int));
    memcpy(C, p->C, p->m * sizeof(int));

    int* row_sat = (int*)calloc(p->n, sizeof(int));
    int* col_sat = (int*)calloc(p->m, sizeof(int));
    int n_rows = p->n, n_cols = p->m;

    int iteration = 0;

    printf("\n=== ALGORITHME BALAS-HAMMER (VOGEL) ===\n");

    while (n_rows > 0 && n_cols > 0) {
        iteration++;
        printf("\n--- ITERATION %d ---\n", iteration);

        double max_pen = -1.0;
        int idx_best = -1;
        bool is_row = true;

        printf("\nPenalites LIGNES :\n");

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
            printf("  F%-2d : %.2f", i, pen);

            if (pen > max_pen) {
                max_pen = pen;
                idx_best = i;
                is_row = true;
                printf(" <- MAX");
            }
            printf("\n");
        }

        printf("\nPenalites COLONNES :\n");

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
            printf("  C%-2d : %.2f", j, pen);

            if (pen > max_pen) {
                max_pen = pen;
                idx_best = j;
                is_row = false;
                printf(" <- MAX");
            }
            printf("\n");
        }

        if (is_row) {
            printf("\n>>> Penalite MAX : Ligne F%d (%.2f)\n", idx_best, max_pen);
        } else {
            printf("\n>>> Penalite MAX : Colonne C%d (%.2f)\n", idx_best, max_pen);
        }

        int r = -1, c = -1;
        double min_cost = DBL_MAX;

        if (is_row) {
            r = idx_best;
            for (int j = 0; j < p->m; j++) {
                if (!col_sat[j] && p->A[r][j] < min_cost) {
                    min_cost = p->A[r][j];
                    c = j;
                }
            }
        } else {
            c = idx_best;
            for (int i = 0; i < p->n; i++) {
                if (!row_sat[i] && p->A[i][c] < min_cost) {
                    min_cost = p->A[i][c];
                    r = i;
                }
            }
        }

        if (r == -1 || c == -1) break;

        printf(">>> Case choisie : (%d, %d) [Cout=%.2f]\n", r, c, min_cost);

        int q = (P[r] < C[c]) ? P[r] : C[c];
        p->B[r][c] = q;
        P[r] -= q; C[c] -= q;

        printf(">>> Quantite affectee : %d\n", q);

        if (P[r] == 0 && C[c] == 0) {
            if (n_rows > 1 || n_cols > 1) {
                row_sat[r] = 1; n_rows--;
                printf(">>> Ligne F%d saturee.\n", r);
            } else {
                row_sat[r] = 1; col_sat[c] = 1;
                n_rows--; n_cols--;
                printf(">>> Ligne F%d et Colonne C%d saturees (derniere iteration).\n", r, c);
            }
        }
        else if (P[r] == 0) {
            row_sat[r] = 1; n_rows--;
            printf(">>> Ligne F%d saturee.\n", r);
        }
        else if (C[c] == 0) {
            col_sat[c] = 1; n_cols--;
            printf(">>> Colonne C%d saturee.\n", c);
        }
    }

    printf("\n=== FIN BALAS-HAMMER ===\n");
    free(P); free(C); free(row_sat); free(col_sat);
}

// Version silencieuse pour les pipelines (options 3 et 4)
void algoBalasHammerSilencieux(ProblemeTransport* p) {
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
            if (pen > max_pen) { max_pen = pen; idx_best = j; is_row = false; }
        }

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

bool testerConnexite(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int total = n + m;

    bool* visite = (bool*)calloc(total, sizeof(bool));
    Queue* q = creerQueue(total);

    enqueue(q, 0, -1);
    visite[0] = true;
    int nb_visites = 1;

    while(!isQueueEmpty(q)) {
        int u = dequeue(q).index;

        if (u < n) {
            for (int j = 0; j < m; j++) {
                if (base[u][j] && !visite[n + j]) {
                    visite[n + j] = true;
                    nb_visites++;
                    enqueue(q, n + j, u);
                }
            }
        } else {
            for (int i = 0; i < n; i++) {
                if (base[i][u - n] && !visite[i]) {
                    visite[i] = true;
                    nb_visites++;
                    enqueue(q, i, u);
                }
            }
        }
    }

    bool connexe = (nb_visites == total);

    if (!connexe) {
        printf("\n[GRAPHE NON CONNEXE]\n");
        printf("  Composante principale : { ");
        for(int i = 0; i < n; i++) {
            if(visite[i]) printf("F%d ", i);
        }
        for(int j = 0; j < m; j++) {
            if(visite[n + j]) printf("C%d ", j);
        }
        printf("} (%d sommets)\n", nb_visites);

        if (nb_visites < total) {
            printf("  Sommets isoles : { ");
            for(int i = 0; i < n; i++) {
                if(!visite[i]) printf("F%d ", i);
            }
            for(int j = 0; j < m; j++) {
                if(!visite[n + j]) printf("C%d ", j);
            }
            printf("} (%d sommets)\n", total - nb_visites);
        }
    }

    free(visite);
    libererQueue(q);
    return connexe;
}

bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int nb_aretes = 0;
    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
            if(base[i][j]) nb_aretes++;

    int requis = n + m - 1;

    printf("\n[TEST DEGENERESCENCE] Aretes de base : %d/%d\n", nb_aretes, requis);

    if (nb_aretes >= requis) {
        bool connexe = testerConnexite(p, base);
        if (connexe) {
            printf(">>> Base complete et connexe.\n");
        }
        return false;
    }

    printf(">>> Base incomplete. Ajout de variables artificielles...\n");

    testerConnexite(p, base);

    int ajouts = 0;
    for(int i=0; i<n && nb_aretes < requis; i++) {
        for(int j=0; j<m && nb_aretes < requis; j++) {
            if (!base[i][j]) {
                if (!cheminExiste(n, m, base, i, n + j)) {
                    base[i][j] = 1;
                    nb_aretes++;
                    ajouts++;
                    printf("  + Arete artificielle : (%d, %d)\n", i, j);
                }
            }
        }
    }

    printf(">>> %d arete(s) ajoutee(s).\n", ajouts);

    return true;
}

void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t) {
    for(int i=0; i<p->n; i++) E_s[i] = DBL_MAX;
    for(int j=0; j<p->m; j++) E_t[j] = DBL_MAX;

    Queue* q = creerQueue(p->n + p->m);

    E_s[0] = 0.0;
    enqueue(q, 0, -1);

    while(!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u < p->n) {
            for (int j = 0; j < p->m; j++) {
                if (base[u][j] && E_t[j] == DBL_MAX) {
                    E_t[j] = E_s[u] - p->A[u][j];
                    enqueue(q, p->n + j, u);
                }
            }
        } else {
            int j = u - p->n;
            for (int i = 0; i < p->n; i++) {
                if (base[i][j] && E_s[i] == DBL_MAX) {
                    E_s[i] = p->A[i][j] + E_t[j];
                    enqueue(q, i, u);
                }
            }
        }
    }
    libererQueue(q);
}

bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t,
                            int* i_ajout, int* j_ajout, double** couts_marginaux) {
    double meilleur_gain = -EPSILON;
    *i_ajout = -1; *j_ajout = -1;
    bool amelioration = false;

    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            if (base[i][j]) {
                couts_marginaux[i][j] = 0.0;
            } else {
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

void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout) {
    int n = p->n, m = p->m;
    Queue* q = creerQueue(n + m);
    int* parent = (int*)malloc((n + m) * sizeof(int));
    bool* visite = (bool*)calloc(n + m, sizeof(bool));

    int start = n + j_ajout;
    int target = i_ajout;

    enqueue(q, start, -1);
    visite[start] = true;
    bool found = false;

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
        // AFFICHAGE DU CYCLE
        int* cycle_sommets = (int*)malloc((n + m) * sizeof(int));
        int cycle_len = 0;

        int curr = target;
        while(curr != start) {
            cycle_sommets[cycle_len++] = curr;
            curr = parent[curr];
        }
        cycle_sommets[cycle_len++] = start;

        printf("\n[CYCLE DETECTE] : ");
        for(int k = cycle_len - 1; k >= 0; k--) {
            int s = cycle_sommets[k];
            if (s < n) printf("F%d", s);
            else printf("C%d", s - n);
            if (k > 0) printf(" -> ");
        }
        printf("\n");

        free(cycle_sommets);

        // CALCUL THETA
        double theta = DBL_MAX;
        int i_out = -1, j_out = -1;

        curr = target;
        bool is_neg = true;

        printf("\n[MAXIMISATION] Conditions :\n");

        while(curr != start) {
            int par = parent[curr];
            int r, c;
            if (curr < n) { r = curr; c = par - n; }
            else { r = par; c = curr - n; }

            if (is_neg) {
                int val = p->B[r][c];
                printf("  Case (%d,%d) : Flux=%d", r, c, val);

                if (val < theta) {
                    theta = val;
                    i_out = r; j_out = c;
                    printf(" <- LIMITANT");
                } else if (fabs(val - theta) < EPSILON && i_out != -1) {
                    int current_id = r * m + c;
                    int best_id = i_out * m + j_out;
                    if (current_id < best_id) {
                        i_out = r; j_out = c;
                        printf(" <- LIMITANT (Bland)");
                    }
                }
                printf("\n");
            }
            is_neg = !is_neg;
            curr = par;
        }

        printf("\n>>> Theta (transfert) = %.0f\n", theta);

        // === AMÉLIORATION 2 : GESTION θ = 0 ===
        if (fabs(theta) < EPSILON) {
            printf("\n[ALERTE] Theta = 0 (Degenerescence cyclique)\n");
            printf(">>> Conservation de l'arete ameliorante (%d, %d)\n", i_ajout, j_ajout);
            printf(">>> Suppression des aretes artificielles (flux nul)...\n");

            // Marquer l'arête améliorante
            base[i_ajout][j_ajout] = 1;

            // Supprimer TOUTES les arêtes artificielles (B[i][j] == 0)
            int nb_supprimees = 0;
            for(int i=0; i<n; i++) {
                for(int j=0; j<m; j++) {
                    if (base[i][j] && p->B[i][j] == 0 && !(i == i_ajout && j == j_ajout)) {
                        base[i][j] = 0;
                        nb_supprimees++;
                        printf("  - Arete artificielle supprimee : (%d, %d)\n", i, j);
                    }
                }
            }

            printf(">>> %d arete(s) artificielle(s) supprimee(s).\n", nb_supprimees);
            printf(">>> Le prochain test de connexite proposera de nouvelles aretes.\n");

            free(parent); free(visite); libererQueue(q);
            return;
        }

        // MISE A JOUR NORMALE (θ > 0)
        p->B[i_ajout][j_ajout] += (int)theta;
        base[i_ajout][j_ajout] = 1;

        curr = target;
        is_neg = true;
        while(curr != start) {
            int par = parent[curr];
            int r, c;
            if (curr < n) { r = curr; c = par - n; }
            else { r = par; c = curr - n; }

            if (is_neg) p->B[r][c] -= (int)theta;
            else p->B[r][c] += (int)theta;
            is_neg = !is_neg;
            curr = par;
        }

        if (i_out != -1) {
            printf("\n[SORTIE DE BASE] Arete supprimee : (%d, %d)\n", i_out, j_out);
            base[i_out][j_out] = 0;
        }
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

    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
            if (p->B[i][j] > 0) base[i][j] = 1;

    int iter = 0;
    bool optimal = false;

    printf("\n\n=== METHODE DU MARCHE-PIED ===\n");

    while (!optimal && iter < 1000) {
        iter++;
        printf("\n========== ITERATION %d ==========\n", iter);

        testerEtResoudreDegenerescence(p, base);
        calculerPotentiels(p, base, E_s, E_t);
        afficherTableauPotentiels(p, E_s, E_t);

        int i_in, j_in;
        bool possible = calculerCoutsMarginaux(p, base, E_s, E_t, &i_in, &j_in, couts_marginaux);
        afficherTableauMarginaux(p, couts_marginaux, base);

        if (!possible) {
            optimal = true;
            printf("\n>>> SOLUTION OPTIMALE (Iteration %d)\n", iter);
        } else {
            printf("\n[AMELIORATION] Arete entrante : (%d, %d) [Gain=%.2f]\n",
                   i_in, j_in, couts_marginaux[i_in][j_in]);
            trouverEtResoudreCycle(p, base, i_in, j_in);
            afficherTableauSolution(p);
            printf("\n>>> Cout apres iteration %d : %.2f\n", iter, calculerCoutTotal(p));
        }
    }

    printf("\n=== FIN MARCHE-PIED ===\n\n");

    for(int i=0; i<n; i++) { free(base[i]); free(couts_marginaux[i]); }
    free(base); free(couts_marginaux); free(E_s); free(E_t);
}



/**
 * @brief Version du Marche-Pied pour l'étude de complexité.
 * Cette fonction est nécessaire pour que etude_complexite.c puisse compiler.
 */
// =========================================================================
// VERSIONS SILENCIEUSES POUR L'ÉTUDE DE COMPLEXITE (SANS PRINTF)
// =========================================================================

// 1. Connexité Silencieuse
bool testerConnexiteSilencieux(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int total = n + m;
    bool* visite = (bool*)calloc(total, sizeof(bool));
    Queue* q = creerQueue(total);

    enqueue(q, 0, -1);
    visite[0] = true;
    int nb_visites = 1;

    while(!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u < n) {
            for (int j = 0; j < m; j++) {
                if (base[u][j] && !visite[n + j]) {
                    visite[n + j] = true;
                    nb_visites++;
                    enqueue(q, n + j, u);
                }
            }
        } else {
            for (int i = 0; i < n; i++) {
                if (base[i][u - n] && !visite[i]) {
                    visite[i] = true;
                    nb_visites++;
                    enqueue(q, i, u);
                }
            }
        }
    }
    free(visite);
    libererQueue(q);
    return (nb_visites == total);
}

// 2. Dégénérescence Silencieuse
bool testerEtResoudreDegenerescenceSilencieux(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int nb_aretes = 0;
    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
            if(base[i][j]) nb_aretes++;

    int requis = n + m - 1;

    if (nb_aretes >= requis) {
        // Juste vérifier la connexité sans rien afficher
        if (testerConnexiteSilencieux(p, base)) return false;
    }

    // Réparation silencieuse
    testerConnexiteSilencieux(p, base);
    for(int i=0; i<n && nb_aretes < requis; i++) {
        for(int j=0; j<m && nb_aretes < requis; j++) {
            if (!base[i][j]) {
                if (!cheminExiste(n, m, base, i, n + j)) {
                    base[i][j] = 1;
                    nb_aretes++;
                }
            }
        }
    }
    return true;
}

// 3. Cycle Silencieux (avec gestion du Theta=0)
// Retourne TRUE si une modification a été faite, FALSE si Theta=0 (stalling)
bool trouverEtResoudreCycleSilencieux(ProblemeTransport* p, int** base, int i_ajout, int j_ajout) {
    int n = p->n, m = p->m;
    Queue* q = creerQueue(n + m);
    int* parent = (int*)malloc((n + m) * sizeof(int));
    bool* visite = (bool*)calloc(n + m, sizeof(bool));

    int start = n + j_ajout;
    int target = i_ajout;

    enqueue(q, start, -1);
    visite[start] = true;
    bool found = false;

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
        double theta = DBL_MAX;
        int i_out = -1, j_out = -1;
        int curr = target;
        bool is_neg = true;

        // Calcul Theta
        while(curr != start) {
            int par = parent[curr];
            int r, c;
            if (curr < n) { r = curr; c = par - n; } else { r = par; c = curr - n; }

            if (is_neg) {
                int val = p->B[r][c];
                if (val < theta) {
                    theta = val;
                    i_out = r; j_out = c;
                } else if (val == theta && i_out != -1) {
                    // Bland
                    if (r * m + c < i_out * m + j_out) { i_out = r; j_out = c; }
                }
            }
            is_neg = !is_neg;
            curr = par;
        }

        // Cas Theta = 0
        if (theta < EPSILON) {
            base[i_ajout][j_ajout] = 1;
            // Suppression des arêtes artificielles sauf celle qu'on vient d'ajouter
            for(int i=0; i<n; i++) {
                for(int j=0; j<m; j++) {
                    if (base[i][j] && p->B[i][j] == 0 && !(i == i_ajout && j == j_ajout)) {
                        base[i][j] = 0;
                    }
                }
            }
            free(parent); free(visite); libererQueue(q);
            return false; // Indique un pas dégénéré
        }

        // Mise à jour normale
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

        if (i_out != -1) base[i_out][j_out] = 0;
    }

    free(parent); free(visite); libererQueue(q);
    return true; // Progression réelle
}

// 4. L'algorithme Principal SILENCIEUX
void algoMarchePiedSilencieux(ProblemeTransport* p) {
    int n = p->n, m = p->m;
    int** base = (int**)malloc(n * sizeof(int*));
    double** couts_marginaux = (double**)malloc(n * sizeof(double*));
    for(int i=0; i<n; i++) {
        base[i] = (int*)calloc(m, sizeof(int));
        couts_marginaux[i] = (double*)calloc(m, sizeof(double));
    }
    double* E_s = (double*)malloc(n * sizeof(double));
    double* E_t = (double*)malloc(m * sizeof(double));

    for(int i=0; i<n; i++)
        for(int j=0; j<m; j++)
            if (p->B[i][j] > 0) base[i][j] = 1;

    int iter = 0;
    int stalling = 0;
    bool optimal = false;

    // Limite de sécurité (5000 itérations)
    while (!optimal && iter < 5000) {
        iter++;

        testerEtResoudreDegenerescenceSilencieux(p, base);
        calculerPotentiels(p, base, E_s, E_t); // Celui-ci n'affiche rien de base, c'est bon

        int i_in, j_in;
        // Celui-ci n'affiche rien si on ne lui demande pas d'afficher la table
        bool possible = calculerCoutsMarginaux(p, base, E_s, E_t, &i_in, &j_in, couts_marginaux);

        if (!possible) {
            optimal = true;
        } else {
            // Appel de la version silencieuse qui retourne false si on piétine (Theta=0)
            bool progression = trouverEtResoudreCycleSilencieux(p, base, i_in, j_in);

            if (!progression) stalling++;
            else stalling = 0;

            // Sécurité anti-boucle infinie sur les 0
            if (stalling > 50) {
                optimal = true; // On force l'arrêt
            }
        }
    }

    for(int i=0; i<n; i++) { free(base[i]); free(couts_marginaux[i]); }
    free(base); free(couts_marginaux); free(E_s); free(E_t);
}
