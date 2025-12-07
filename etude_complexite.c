/**
 * @file etude_complexite.c
 * @brief Programme d'analyse de la complexité (Console + Gnuplot).
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "projet_ro.h"

// Valeurs de n à tester
// 7 tailles réparties de manière quasi-logarithmique
const int TAILLES[] = {10, 20, 40, 60, 80, 100, 120};
const int NB_TAILLES = 7;
// 20 essais suffisent pour avoir une moyenne stable
const int NB_ESSAIS = 20;

// Structure pour stocker les résultats
typedef struct {
    double theta_NO;  // Temps Nord-Ouest
    double theta_BH;  // Temps Balas-Hammer
    double t_NO;      // Temps Marche-Pied après NO
    double t_BH;      // Temps Marche-Pied après BH
} ResultatTemps;

// --- FONCTIONS UTILITAIRES ---

ProblemeTransport* genererProblemeAleatoire(int n) {
    ProblemeTransport* p = creerProbleme(n, n);

    // 1. Coûts aléatoires
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            p->A[i][j] = (double)(rand() % 100 + 1);
        }
    }

    // 2. Garantir l'équilibre Offre/Demande
    int** temp = (int**)malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        temp[i] = (int*)malloc(n * sizeof(int));
        for (int j = 0; j < n; j++) {
            temp[i][j] = rand() % 100 + 1;
        }
    }

    for (int i = 0; i < n; i++) {
        p->P[i] = 0;
        for (int j = 0; j < n; j++) p->P[i] += temp[i][j];
    }
    for (int j = 0; j < n; j++) {
        p->C[j] = 0;
        for (int i = 0; i < n; i++) p->C[j] += temp[i][j];
    }

    for (int i = 0; i < n; i++) free(temp[i]);
    free(temp);

    return p;
}

void executerBatterieTests(int n, ResultatTemps resultats[]) {
    printf("\n------------------------------------------------------------\n");
    printf("   TEST TAILLE N = %d  (%d essais)\n", n, NB_ESSAIS);
    printf("------------------------------------------------------------\n");

    // Variables pour calculer la moyenne (RESTAURÉ)
    double somme_NO = 0, somme_BH = 0;
    double somme_MpNO = 0, somme_MpBH = 0;

    for (int essai = 0; essai < NB_ESSAIS; essai++) {
        if (essai % 5 == 0) { printf("."); fflush(stdout); } // Barre de progression simple

        ProblemeTransport* p = genererProblemeAleatoire(n);

        // 1. Nord-Ouest
        clock_t d = clock();
        algoNordOuestSilencieux(p);
        double t1 = ((double)(clock() - d)) / CLOCKS_PER_SEC;
        resultats[essai].theta_NO = t1;
        somme_NO += t1;

        // 2. Marche-Pied (depuis NO)
        d = clock();
        algoMarchePiedSilencieux(p);
        double t2 = ((double)(clock() - d)) / CLOCKS_PER_SEC;
        resultats[essai].t_NO = t2;
        somme_MpNO += t2;

        reinitialiserSolution(p);

        // 3. Balas-Hammer
        d = clock();
        algoBalasHammerSilencieux(p);
        double t3 = ((double)(clock() - d)) / CLOCKS_PER_SEC;
        resultats[essai].theta_BH = t3;
        somme_BH += t3;

        // 4. Marche-Pied (depuis BH)
        d = clock();
        algoMarchePiedSilencieux(p);
        double t4 = ((double)(clock() - d)) / CLOCKS_PER_SEC;
        resultats[essai].t_BH = t4;
        somme_MpBH += t4;

        libererProbleme(p);
    }

    printf(" Termine !\n");

    // --- AFFICHAGE CONSOLE (RESTAURÉ) ---
    printf("   > Moyenne Nord-Ouest (Init)    : %.6f s\n", somme_NO / NB_ESSAIS);
    printf("   > Moyenne Marche-Pied (post NO): %.6f s\n", somme_MpNO / NB_ESSAIS);
    printf("   > Moyenne Balas-Hammer (Init)  : %.6f s\n", somme_BH / NB_ESSAIS);
    printf("   > Moyenne Marche-Pied (post BH): %.6f s\n", somme_MpBH / NB_ESSAIS);

    double total_NO = (somme_NO + somme_MpNO) / NB_ESSAIS;
    double total_BH = (somme_BH + somme_MpBH) / NB_ESSAIS;

    printf("\n   >>> TEMPS TOTAL MOYEN (NO + Optim) : %.6f s\n", total_NO);
    printf("   >>> TEMPS TOTAL MOYEN (BH + Optim) : %.6f s\n", total_BH);

    if (total_BH < total_NO) {
        printf("   [CONCLUSION] Balas-Hammer est %.1fx plus rapide !\n", total_NO / total_BH);
    } else {
        printf("   [CONCLUSION] Nord-Ouest est plus rapide.\n");
    }
}

// Sauvegarde au format .dat pour Gnuplot
void sauvegarderResultatsDat(int n, ResultatTemps resultats[]) {
    char nom_fichier[256];
    sprintf(nom_fichier, "raw_n%d.dat", n);
    FILE* f = fopen(nom_fichier, "w");
    if (!f) return;

    fprintf(f, "# essai theta_NO theta_BH t_NO t_BH total_NO total_BH\n");
    for (int i = 0; i < NB_ESSAIS; i++) {
        fprintf(f, "%d %.6f %.6f %.6f %.6f %.6f %.6f\n",
                i + 1,
                resultats[i].theta_NO,
                resultats[i].theta_BH,
                resultats[i].t_NO,
                resultats[i].t_BH,
                resultats[i].theta_NO + resultats[i].t_NO,
                resultats[i].theta_BH + resultats[i].t_BH);
    }
    fclose(f);
}

// Lit les fichiers .dat bruts et crée le fichier de synthèse
void sauvegarderSyntheseDat(const char* nom_fichier) {
    FILE* f = fopen(nom_fichier, "w");
    if (!f) return;

    fprintf(f, "# n max_total_NO max_total_BH\n");

    for (int idx = 0; idx < NB_TAILLES; idx++) {
        int n = TAILLES[idx];
        char nom_brut[256];
        sprintf(nom_brut, "raw_n%d.dat", n);
        FILE* fb = fopen(nom_brut, "r");
        if (!fb) continue;

        char ligne[512];
        fgets(ligne, sizeof(ligne), fb); // Sauter le header

        double max_total_NO = 0, max_total_BH = 0;
        // Lecture format espace
        while (fgets(ligne, sizeof(ligne), fb)) {
            int e;
            double v[6];
            if(sscanf(ligne, "%d %lf %lf %lf %lf %lf %lf",
               &e, &v[0], &v[1], &v[2], &v[3], &v[4], &v[5]) == 7) {
                if (v[4] > max_total_NO) max_total_NO = v[4];
                if (v[5] > max_total_BH) max_total_BH = v[5];
            }
        }
        fclose(fb);
        fprintf(f, "%d %.6f %.6f\n", n, max_total_NO, max_total_BH);
    }
    fclose(f);
}

void genererScriptGnuplot() {
    FILE* f = fopen("script_plot.plt", "w");
    if (!f) return;

    fprintf(f, "set terminal pngcairo size 800,600 enhanced font 'Arial,10'\n");
    fprintf(f, "set output 'resultat_complexite.png'\n");
    fprintf(f, "set title 'Comparaison des performances : Nord-Ouest vs Balas-Hammer'\n");
    fprintf(f, "set xlabel 'Taille de la matrice (n)'\n");
    fprintf(f, "set ylabel 'Temps total d execution (s)'\n");
    fprintf(f, "set grid\n");
    fprintf(f, "set key left top box\n");
    fprintf(f, "set logscale x 10\n");
    fprintf(f, "set logscale y 10\n");

    fprintf(f, "plot 'synthese.dat' using 1:2 with linespoints lw 2 title 'Nord-Ouest + MP', \\\n");
    fprintf(f, "     'synthese.dat' using 1:3 with linespoints lw 2 title 'Balas-Hammer + MP'\n");

    fclose(f);
}

// --- FONCTION PRINCIPALE ---

void lancer_etude_complete() {
    printf("\n=== ETUDE DE COMPLEXITE (Console + Gnuplot) ===\n");
    srand(time(NULL));

    for (int idx = 0; idx < NB_TAILLES; idx++) {
        int n = TAILLES[idx];
        ResultatTemps* res = (ResultatTemps*)malloc(NB_ESSAIS * sizeof(ResultatTemps));
        if(!res) continue;

        executerBatterieTests(n, res); // Affiche les stats dans la console
        sauvegarderResultatsDat(n, res); // Sauvegarde pour le graphique
        free(res);
    }

    sauvegarderSyntheseDat("synthese.dat");
    genererScriptGnuplot();

    printf("\n------------------------------------------------\n");
    printf("Generation graphique...\n");
    int ret = system("\"C:\\Program Files\\gnuplot\\bin\\gnuplot.exe\" script_plot.plt");
    if (ret == 0) {
        printf("Succes ! 'resultat_complexite.png' a ete cree.\n");
        system("start resultat_complexite.png"); // Ouvre l'image
    } else {
        printf("[Info] Gnuplot n'est pas lance automatiquement (pas dans le PATH ?).\n");
        printf("Vous pouvez generer le graphique manuellement avec : gnuplot script_plot.plt\n");
    }

    printf("\nAppuyez sur Entree pour revenir au menu...");
    getchar(); getchar();
}
