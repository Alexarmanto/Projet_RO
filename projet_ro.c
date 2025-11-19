/**
 * @file projet_ro.c
 * @brief Implémentation des algorithmes de Recherche Opérationnelle.
 * Contient la logique du Coin Nord-Ouest, Balas-Hammer et Marche-Pied.
 */

#include "projet_ro.h"

// ==========================================================
// 0. OUTILS AUXILIAIRES (FILE / QUEUE POUR BFS)
// ==========================================================
// Placés en haut pour être accessibles par toutes les fonctions.

Queue* creerQueue(int capacity) {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (!q) exit(EXIT_FAILURE);
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
    if (q->rear >= q->capacity - 1) return; // Sécurité
    q->rear++;
    q->data[q->rear].index = index;
    q->data[q->rear].parent = parent;
}

BFSNode dequeue(Queue* q) {
    BFSNode node = { -1, -1 };
    if (isQueueEmpty(q)) return node;
    node = q->data[q->front];
    q->front++;
    return node;
}

/**
 * @brief Helper local pour trouver un chemin BFS sans passer par une arête exclue.
 * Utile pour la détection de cycles.
 */
static bool trouverCheminBFS(ProblemeTransport* p, int** base, int start_node, int end_node, int i_exclu, int j_exclu) {
    int n = p->n;
    int m = p->m;
    int nb_noeuds = n + m;

    Queue* q = creerQueue(nb_noeuds);
    bool* visite = (bool*)calloc(nb_noeuds, sizeof(bool));

    enqueue(q, start_node, -1);
    visite[start_node] = true;

    bool trouve = false;

    while (!isQueueEmpty(q)) {
        BFSNode noeud = dequeue(q);
        int u = noeud.index;

        if (u == end_node) {
            trouve = true;
            break;
        }

        if (u < n) { // C'est un Fournisseur S_i
            int i = u;
            for (int j = 0; j < m; j++) {
                int v = n + j;
                if (i == i_exclu && j == j_exclu) continue; // Arête interdite
                if (base[i][j] == 1 && !visite[v]) {
                    visite[v] = true;
                    enqueue(q, v, u);
                }
            }
        } else { // C'est un Client C_j
            int j = u - n;
            for (int i = 0; i < n; i++) {
                int v = i;
                if (i == i_exclu && j == j_exclu) continue; // Arête interdite
                if (base[i][j] == 1 && !visite[v]) {
                    visite[v] = true;
                    enqueue(q, v, u);
                }
            }
        }
    }

    libererQueue(q);
    free(visite);
    return trouve;
}


// ==========================================================
// 1. GESTION MÉMOIRE ET LECTURE
// ==========================================================

ProblemeTransport* creerProbleme(int n, int m) {
    ProblemeTransport* p = (ProblemeTransport*)malloc(sizeof(ProblemeTransport));
    if (!p) { perror("Alloc P"); exit(EXIT_FAILURE); }

    p->n = n;
    p->m = m;

    // Allocation des matrices
    p->A = (double**)malloc(n * sizeof(double*));
    p->B = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        p->A[i] = (double*)malloc(m * sizeof(double));
        p->B[i] = (int*)malloc(m * sizeof(int));
        // Initialisation B à 0
        for(int j=0; j<m; j++) p->B[i][j] = 0;
    }

    // Allocation des vecteurs
    p->P = (int*)malloc(n * sizeof(int));
    p->C = (int*)malloc(m * sizeof(int));

    return p;
}

void libererProbleme(ProblemeTransport* p) {
    if (!p) return;
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

ProblemeTransport* lireDonnees(const char* nomFichier) {
    FILE* f = fopen(nomFichier, "r");
    if (!f) { perror("Erreur ouverture fichier"); exit(EXIT_FAILURE); }

    int n, m;
    if (fscanf(f, "%d %d", &n, &m) != 2) exit(EXIT_FAILURE);

    ProblemeTransport* p = creerProbleme(n, m);

    // Lecture Matrice Coûts A
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            fscanf(f, "%lf", &p->A[i][j]);
        }
    }
    // Lecture Provisions P
    for (int i = 0; i < n; i++) fscanf(f, "%d", &p->P[i]);
    // Lecture Commandes C
    for (int j = 0; j < m; j++) fscanf(f, "%d", &p->C[j]);

    fclose(f);
    printf("Info: Données chargées (n=%d, m=%d) depuis '%s'.\n", n, m, nomFichier);
    return p;
}

void reinitialiserSolution(ProblemeTransport* p) {
    if (!p) return;
    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            p->B[i][j] = 0;
        }
    }
}


// ==========================================================
// 2. AFFICHAGE ET CALCULS BASIQUES
// ==========================================================

void afficherTableauCouts(ProblemeTransport* p) {
    if (!p) return;
    const int W = 12; // Largeur colonne

    printf("\n--- MATRICE DES COUTS (A) ---\n\n%*s", W, "");
    for (int j = 0; j < p->m; j++) {
        printf("Client %-5d", j);
    }
    printf("%*s\n", W, "Prov.(P)");

    for (int i = 0; i < p->n; i++) {
        printf("Fourn. %-5d", i);
        for (int j = 0; j < p->m; j++) printf("%*.2f", W, p->A[i][j]);
        printf("%*d\n", W, p->P[i]);
    }
    printf("%*s", W, "Comm.(C)");
    for (int j = 0; j < p->m; j++) printf("%*d", W, p->C[j]);
    printf("\n\n");
}

void afficherTableauSolution(ProblemeTransport* p) {
    if (!p) return;
    const int W = 12;
    printf("\n--- PROPOSITION DE TRANSPORT (B) ---\n\n%*s", W, "");
    for (int j = 0; j < p->m; j++) printf("Client %-5d", j);
    printf("%*s\n", W, "Prov.(P)");

    for (int i = 0; i < p->n; i++) {
        printf("Fourn. %-5d", i);
        for (int j = 0; j < p->m; j++) printf("%*d", W, p->B[i][j]);
        printf("%*d\n", W, p->P[i]);
    }
    printf("%*s", W, "Comm.(C)");
    for (int j = 0; j < p->m; j++) printf("%*d", W, p->C[j]);
    printf("\n\n");
}

double calculerCoutTotal(ProblemeTransport* p) {
    double total = 0.0;
    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            total += p->A[i][j] * p->B[i][j];
        }
    }
    return total;
}

void afficherTableauPotentiels(ProblemeTransport* p, double* E_s, double* E_t) {
    const int W = 12;
    printf("\n--- POTENTIELS (E_s - E_t) ---\n\n%*s", W, "");
    for (int j = 0; j < p->m; j++) printf("Cli %d", j);
    printf("%*s\n", W, "E(s)");

    printf("%*s", W, "");
    for (int j = 0; j < p->m; j++) printf("%*.2f", W, E_t[j]); // Affiche E(t) en haut
    printf("\n");

    for (int i = 0; i < p->n; i++) {
        printf("Fourn. %-5d", i);
        for (int j = 0; j < p->m; j++) {
            printf("%*.2f", W, E_s[i] - E_t[j]);
        }
        printf("%*.2f\n", W, E_s[i]); // Affiche E(s) à droite
    }
}

void afficherTableauMarginaux(ProblemeTransport* p, double** couts_marginaux, int** base) {
    const int W = 12;
    printf("\n--- COUTS MARGINAUX ---\n(0.00 = variable en base)\n\n%*s", W, "");
    for (int j = 0; j < p->m; j++) printf("Cli %d", j);
    printf("\n");

    for (int i = 0; i < p->n; i++) {
        printf("Fourn. %-5d", i);
        for (int j = 0; j < p->m; j++) {
            if (base[i][j]) printf("%*.2f", W, 0.0);
            else printf("%*.2f", W, couts_marginaux[i][j]);
        }
        printf("\n");
    }
}


// ==========================================================
// 3. ALGORITHMES INITIAUX (Nord-Ouest & Balas-Hammer)
// ==========================================================

void algoNordOuest(ProblemeTransport* p) {
    if (!p) return;
    reinitialiserSolution(p);

    int* P_tmp = (int*)malloc(p->n * sizeof(int));
    int* C_tmp = (int*)malloc(p->m * sizeof(int));
    memcpy(P_tmp, p->P, p->n * sizeof(int));
    memcpy(C_tmp, p->C, p->m * sizeof(int));

    int i = 0, j = 0;
    while (i < p->n && j < p->m) {
        int qte = (P_tmp[i] < C_tmp[j]) ? P_tmp[i] : C_tmp[j];
        p->B[i][j] = qte;
        P_tmp[i] -= qte;
        C_tmp[j] -= qte;

        if (P_tmp[i] == 0) i++;
        if (C_tmp[j] == 0) j++;
    }

    free(P_tmp);
    free(C_tmp);
}

void algoBalasHammer(ProblemeTransport* p) {
    if (!p) return;
    reinitialiserSolution(p);

    // Copies de travail
    int* P_tmp = (int*)malloc(p->n * sizeof(int));
    int* C_tmp = (int*)malloc(p->m * sizeof(int));
    memcpy(P_tmp, p->P, p->n * sizeof(int));
    memcpy(C_tmp, p->C, p->m * sizeof(int));

    // Marqueurs de saturation (0=non, 1=oui)
    int* row_sat = (int*)calloc(p->n, sizeof(int));
    int* col_sat = (int*)calloc(p->m, sizeof(int));

    // Tableaux pénalités
    double* pen_row = (double*)malloc(p->n * sizeof(double));
    double* pen_col = (double*)malloc(p->m * sizeof(double));

    int nb_rows = p->n, nb_cols = p->m;

    while (nb_rows > 0 && nb_cols > 0) {
        // 1. Calcul pénalités Lignes
        for (int i = 0; i < p->n; i++) {
            if (row_sat[i]) { pen_row[i] = -1.0; continue; }
            double m1 = DBL_MAX, m2 = DBL_MAX;
            for (int j = 0; j < p->m; j++) {
                if (col_sat[j]) continue;
                if (p->A[i][j] < m1) { m2 = m1; m1 = p->A[i][j]; }
                else if (p->A[i][j] < m2) { m2 = p->A[i][j]; }
            }
            pen_row[i] = (m2 == DBL_MAX) ? m1 : (m2 - m1);
        }

        // 2. Calcul pénalités Colonnes
        for (int j = 0; j < p->m; j++) {
            if (col_sat[j]) { pen_col[j] = -1.0; continue; }
            double m1 = DBL_MAX, m2 = DBL_MAX;
            for (int i = 0; i < p->n; i++) {
                if (row_sat[i]) continue;
                if (p->A[i][j] < m1) { m2 = m1; m1 = p->A[i][j]; }
                else if (p->A[i][j] < m2) { m2 = p->A[i][j]; }
            }
            pen_col[j] = (m2 == DBL_MAX) ? m1 : (m2 - m1);
        }

        // 3. Trouver max pénalité
        double max_p = -1.0;
        int idx_max = -1, is_row = 0;

        for (int i = 0; i < p->n; i++) {
            if (pen_row[i] > max_p) { max_p = pen_row[i]; is_row = 1; idx_max = i; }
        }
        for (int j = 0; j < p->m; j++) {
            if (pen_col[j] > max_p) { max_p = pen_col[j]; is_row = 0; idx_max = j; }
        }

        if (idx_max == -1) break; // Tout est saturé

        // 4. Sélectionner case min coût dans la ligne/col choisie
        int i_sel = -1, j_sel = -1;
        double min_c = DBL_MAX;

        if (is_row) {
            i_sel = idx_max;
            for (int j = 0; j < p->m; j++) {
                if (!col_sat[j] && p->A[i_sel][j] < min_c) {
                    min_c = p->A[i_sel][j]; j_sel = j;
                }
            }
        } else {
            j_sel = idx_max;
            for (int i = 0; i < p->n; i++) {
                if (!row_sat[i] && p->A[i][j_sel] < min_c) {
                    min_c = p->A[i][j_sel]; i_sel = i;
                }
            }
        }

        // Cas particulier fin de boucle
        if (i_sel == -1 || j_sel == -1) {
             for(int i=0;i<p->n;i++) if(!row_sat[i]) i_sel=i;
             for(int j=0;j<p->m;j++) if(!col_sat[j]) j_sel=j;
             if(i_sel == -1 || j_sel == -1) break;
        }

        // 5. Affectation
        int qte = (P_tmp[i_sel] < C_tmp[j_sel]) ? P_tmp[i_sel] : C_tmp[j_sel];
        p->B[i_sel][j_sel] = qte;
        P_tmp[i_sel] -= qte;
        C_tmp[j_sel] -= qte;

        if (P_tmp[i_sel] == 0 && !row_sat[i_sel]) { row_sat[i_sel] = 1; nb_rows--; }
        if (C_tmp[j_sel] == 0 && !col_sat[j_sel]) { col_sat[j_sel] = 1; nb_cols--; }
    }

    free(P_tmp); free(C_tmp);
    free(row_sat); free(col_sat);
    free(pen_row); free(pen_col);
}


// ==========================================================
// 4. OPTIMISATION (ALGO MARCHE-PIED / STEPPING-STONE)
// ==========================================================

bool testerEtResoudreDegenerescence(ProblemeTransport* p, int** base) {
    int n = p->n, m = p->m;
    int nb_noeuds = n + m;
    int nb_aretes = 0;

    // 1. Construction de la base initiale (B > 0)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            if (p->B[i][j] > 0) { base[i][j] = 1; nb_aretes++; }
            else base[i][j] = 0;
        }
    }

    printf("  [Graphe] Arêtes: %d (Requis: %d)\n", nb_aretes, nb_noeuds - 1);

    // 2. Résolution des CYCLES
    bool cycle_resolu = true;
    while (cycle_resolu) {
        cycle_resolu = false;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < m; j++) {
                if (base[i][j] == 1) {
                    // Si on trouve un chemin sans passer par (i,j), c'est qu'il y a un cycle
                    if (trouverCheminBFS(p, base, i, n + j, i, j)) {
                        printf("    -> Cycle détecté en (%d, %d). Résolution...\n", i, j);
                        trouverEtResoudreCycle(p, base, i, j);
                        cycle_resolu = true;
                        break;
                    }
                }
            }
            if(cycle_resolu) break;
        }
    }

    // 3. Résolution de la NON-CONNEXITÉ (Arbre incomplet)
    nb_aretes = 0;
    for(int i=0; i<n; i++) for(int j=0; j<m; j++) if(base[i][j]) nb_aretes++;

    if (nb_aretes < nb_noeuds - 1) {
        printf("  [Graphe] Non-connexe. Ajout variables artificielles (epsilon)...\n");
        for (int i = 0; i < n; i++) {
            if (nb_aretes >= nb_noeuds - 1) break;
            for (int j = 0; j < m; j++) {
                if (nb_aretes >= nb_noeuds - 1) break;
                if (base[i][j] == 0) {
                    // On ajoute l'arête SI elle ne crée pas de cycle
                    if (!trouverCheminBFS(p, base, i, n + j, -1, -1)) {
                        base[i][j] = 1; // Variable artificielle (0 quantite, mais en base)
                        nb_aretes++;
                        printf("    -> Ajout artificielle en (%d, %d)\n", i, j);
                    }
                }
            }
        }
    }

    return (nb_aretes == nb_noeuds - 1);
}

void calculerPotentiels(ProblemeTransport* p, int** base, double* E_s, double* E_t) {
    int n = p->n, m = p->m;
    bool* s_ok = (bool*)calloc(n, sizeof(bool));
    bool* t_ok = (bool*)calloc(m, sizeof(bool));
    Queue* q = creerQueue(n + m);

    // Init à "infini"
    for(int i=0; i<n; i++) E_s[i] = DBL_MAX;
    for(int j=0; j<m; j++) E_t[j] = DBL_MAX;

    // Fixer arbitrairement E_s[0] = 0
    E_s[0] = 0.0; s_ok[0] = true;
    enqueue(q, 0, -1);

    while (!isQueueEmpty(q)) {
        int u = dequeue(q).index;
        if (u < n) { // Fournisseur i
            int i = u;
            for (int j = 0; j < m; j++) {
                // Si connecté par la base et E_t[j] inconnu
                if (base[i][j] && !t_ok[j]) {
                    E_t[j] = E_s[i] - p->A[i][j]; // c = Es - Et => Et = Es - c
                    t_ok[j] = true;
                    enqueue(q, n + j, i);
                }
            }
        } else { // Client j
            int j = u - n;
            for (int i = 0; i < n; i++) {
                if (base[i][j] && !s_ok[i]) {
                    E_s[i] = E_t[j] + p->A[i][j]; // Es = Et + c
                    s_ok[i] = true;
                    enqueue(q, i, n + j);
                }
            }
        }
    }
    free(s_ok); free(t_ok); libererQueue(q);
}

bool calculerCoutsMarginaux(ProblemeTransport* p, int** base, double* E_s, double* E_t,
                            int* i_ajout, int* j_ajout, double** couts_marginaux) {
    double meilleur_gain = 0.0;
    bool non_optimal = false;
    *i_ajout = -1; *j_ajout = -1;

    for (int i = 0; i < p->n; i++) {
        for (int j = 0; j < p->m; j++) {
            if (base[i][j]) {
                couts_marginaux[i][j] = 0.0;
            } else {
                // d_ij = c_ij - (Es - Et)
                double d = p->A[i][j] - (E_s[i] - E_t[j]);
                couts_marginaux[i][j] = d;

                // On cherche la valeur la plus négative (le plus gros gain)
                // Tolerance 1e-9 pour erreurs flottantes
                if (d < -1e-9 && d < meilleur_gain) {
                    meilleur_gain = d;
                    *i_ajout = i;
                    *j_ajout = j;
                    non_optimal = true;
                }
            }
        }
    }
    return non_optimal; // Retourne true si on peut encore optimiser
}

void trouverEtResoudreCycle(ProblemeTransport* p, int** base, int i_ajout, int j_ajout) {
    int n = p->n, m = p->m;
    int nb_noeuds = n + m;

    // 1. BFS pour trouver le cycle
    // On cherche un chemin de Client(j_ajout) vers Fournisseur(i_ajout) dans la base
    int* parent = (int*)malloc(nb_noeuds * sizeof(int));
    bool* visite = (bool*)calloc(nb_noeuds, sizeof(bool));
    for(int k=0;k<nb_noeuds;k++) parent[k] = -1;

    Queue* q = creerQueue(nb_noeuds);
    int start = n + j_ajout;
    int target = i_ajout;

    enqueue(q, start, -1);
    visite[start] = true;

    while (!isQueueEmpty(q) && parent[target] == -1) {
        int u = dequeue(q).index;
        if (u < n) { // Fourn i
            for (int j = 0; j < m; j++) {
                if (base[u][j] && !visite[n + j] && !(u==i_ajout && j==j_ajout)) {
                    visite[n + j] = true; parent[n + j] = u; enqueue(q, n + j, u);
                }
            }
        } else { // Cli j
            for (int i = 0; i < n; i++) {
                if (base[i][u-n] && !visite[i] && !(i==i_ajout && (u-n)==j_ajout)) {
                    visite[i] = true; parent[i] = u; enqueue(q, i, u);
                }
            }
        }
    }

    // 2. Remonter le cycle pour trouver Delta (min sur les arêtes sortantes -)
    // Cycle : + (i_ajout, j_ajout) - (j_ajout, parent) + ...
    double delta = DBL_MAX;
    int cur = target;
    int i_out = -1, j_out = -1;
    bool est_plus = false; // On commence par le parent de i_ajout qui est une arête '-' par rapport au sens du cycle

    // Parcours arrière depuis i_ajout vers j_ajout
    while(cur != start) {
        int par = parent[cur];
        int i, j;
        if (cur < n) { i = cur; j = par - n; } // Lien (i, j)
        else { i = par; j = cur - n; }         // Lien (i, j)

        if (!est_plus) { // Arête type (-)
            if (p->B[i][j] < delta) {
                delta = p->B[i][j];
                i_out = i; j_out = j;
            }
        }
        est_plus = !est_plus;
        cur = par;
    }

    printf("    -> Delta trouvé: %.2f. Arête sortante: (%d, %d)\n", delta, i_out, j_out);

    // 3. Appliquer Delta
    p->B[i_ajout][j_ajout] += delta;
    base[i_ajout][j_ajout] = 1; // Entre en base

    cur = target;
    est_plus = false;
    while(cur != start) {
        int par = parent[cur];
        int i, j;
        if (cur < n) { i = cur; j = par - n; } else { i = par; j = cur - n; }

        if (!est_plus) p->B[i][j] -= delta;
        else p->B[i][j] += delta;

        est_plus = !est_plus;
        cur = par;
    }

    // 4. Sortie de base (Si delta=0 c'est un pivot dégénéré, mais on sort quand même l'arête)
    base[i_out][j_out] = 0;

    free(parent); free(visite); libererQueue(q);
}

void algoMarchePied(ProblemeTransport* p) {
    int n = p->n, m = p->m;

    // Allocations structures temporaires
    int** base = (int**)malloc(n * sizeof(int*));
    double** couts_marginaux = (double**)malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        base[i] = (int*)calloc(m, sizeof(int));
        couts_marginaux[i] = (double*)calloc(m, sizeof(double));
    }
    double* E_s = (double*)malloc(n * sizeof(double));
    double* E_t = (double*)malloc(m * sizeof(double));

    int iter = 0;
    bool optimal = false;

    while (!optimal && iter < 50) { // Sécurité anti-boucle infinie
        iter++;
        printf("\n=== ITERATION %d ===\n", iter);

        // 1. Vérifier si base valide
        if (!testerEtResoudreDegenerescence(p, base)) {
            fprintf(stderr, "Erreur fatale structure graphe.\n");
            break;
        }

        // 2. Potentiels
        calculerPotentiels(p, base, E_s, E_t);
        afficherTableauPotentiels(p, E_s, E_t);

        // 3. Coûts marginaux
        int i_in, j_in;
        bool ameliorable = calculerCoutsMarginaux(p, base, E_s, E_t, &i_in, &j_in, couts_marginaux);
        afficherTableauMarginaux(p, couts_marginaux, base);

        if (!ameliorable) {
            optimal = true;
            printf("\n>>> SOLUTION OPTIMALE ATTEINTE <<<\n");
        } else {
            printf("\n>>> Amélioration possible via (%d, %d). Coût marginal: %.2f\n",
                   i_in, j_in, couts_marginaux[i_in][j_in]);

            trouverEtResoudreCycle(p, base, i_in, j_in);
            printf("    Nouveau coût total: %.2f\n", calculerCoutTotal(p));
            afficherTableauSolution(p);
        }
    }

    // Nettoyage final
    for(int i=0; i<n; i++) { free(base[i]); free(couts_marginaux[i]); }
    free(base); free(couts_marginaux); free(E_s); free(E_t);
}
