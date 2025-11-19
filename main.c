/**
 * @file main.c
 * @brief Programme principal.
 * Permet de tester indépendamment les étapes 1 (Lecture), 2 (Solutions Initiales) et 3 (Optimisation).
 */

#include <stdio.h>
#include <stdlib.h>
#include "projet_ro.h"

// ==========================================================
// FONCTION DE TEST - ETAPE 1 (Lecture & Affichage)
// ==========================================================
void testerEtape1(const char* nomFichier) {
    printf("\n**************************************************\n");
    printf("*** TEST ETAPE 1 : LECTURE DONNEES         ***\n");
    printf("**************************************************\n");
    printf("Fichier : %s\n", nomFichier);

    // 1. Lecture
    ProblemeTransport* p = lireDonnees(nomFichier);
    if (p == NULL) return;

    // 2. Affichage des structures
    afficherTableauCouts(p);

    printf("Note : La solution (B) est initialisee a 0 par defaut.\n");
    afficherTableauSolution(p);

    // 3. Nettoyage
    libererProbleme(p);
    printf("--> Fin du Test Etape 1 (Memoire liberee).\n\n");
}

// ==========================================================
// FONCTION DE TEST - ETAPE 2 (Comparaison Algorithmes)
// ==========================================================
void testerEtape2(const char* nomFichier) {
    printf("\n**************************************************\n");
    printf("*** TEST ETAPE 2 : SOLUTIONS INITIALES     ***\n");
    printf("**************************************************\n");

    ProblemeTransport* p = lireDonnees(nomFichier);
    if (p == NULL) return;

    afficherTableauCouts(p);

    // --- Test 1 : Nord-Ouest ---
    printf("\n[1] Execution algorithme COIN NORD-OUEST...\n");
    algoNordOuest(p);
    afficherTableauSolution(p);
    double cout_no = calculerCoutTotal(p);

    // --- Test 2 : Balas-Hammer ---
    printf("\n[2] Execution algorithme BALAS-HAMMER (Vogel)...\n");
    algoBalasHammer(p);
    afficherTableauSolution(p);
    double cout_bh = calculerCoutTotal(p);

    // --- Bilan ---
    printf("\n--------------------------------------------------\n");
    printf(" BILAN COMPARATIF (%s)\n", nomFichier);
    printf("--------------------------------------------------\n");
    printf(" -> Cout Nord-Ouest   : %10.2f\n", cout_no);
    printf(" -> Cout Balas-Hammer : %10.2f\n", cout_bh);

    if (cout_bh < cout_no) {
        printf("\n => Balas-Hammer est meilleur (Gain: %.2f)\n", cout_no - cout_bh);
    } else if (cout_no < cout_bh) {
        printf("\n => Nord-Ouest est meilleur (Gain: %.2f)\n", cout_bh - cout_no);
    } else {
        printf("\n => Les deux methodes donnent le meme resultat.\n");
    }

    libererProbleme(p);
    printf("\n--> Fin du Test Etape 2.\n\n");
}

// ==========================================================
// FONCTION DE TEST - ETAPE 3 (Optimisation Complète)
// ==========================================================
void testerEtape3(const char* nomFichier) {
    printf("\n**************************************************\n");
    printf("*** TEST ETAPE 3 : OPTIMISATION MARCHE-PIED  ***\n");
    printf("**************************************************\n");

    ProblemeTransport* p = lireDonnees(nomFichier);
    if (p == NULL) return;

    // 1. Solution de départ
    printf("\n--- PHASE A : Solution Initiale (Balas-Hammer) ---\n");
    algoBalasHammer(p);
    double cout_init = calculerCoutTotal(p);
    afficherTableauSolution(p);
    printf(">>> Cout de depart : %.2f\n", cout_init);

    // 2. Optimisation
    printf("\n--- PHASE B : Lancement du Marche-Pied ---\n");
    algoMarchePied(p);

    // 3. Résultats
    double cout_final = calculerCoutTotal(p);

    printf("\n--------------------------------------------------\n");
    printf(" RESULTAT FINAL OPTIMISE\n");
    printf("--------------------------------------------------\n");
    afficherTableauSolution(p);
    printf(" -> Cout Initial : %.2f\n", cout_init);
    printf(" -> Cout Final   : %.2f\n", cout_final);
    printf(" -> Amelioration : %.2f\n", cout_init - cout_final);

    libererProbleme(p);
    printf("\n--> Fin du Test Etape 3.\n\n");
}

// ==========================================================
// MAIN (Selecteur)
// ==========================================================
int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_donnees.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* nomFichier = argv[1];

    // ------------------------------------------------------
    // ZONE DE SELECTION DU TEST
    // Decommentez la ligne correspondant au test voulu
    // ------------------------------------------------------

    // testerEtape1(nomFichier);  // Lecture seule
     testerEtape2(nomFichier);  // Comparaison NO vs BH
    // testerEtape3(nomFichier);  // Optimisation complète (Le "Vrai" programme)

    // ------------------------------------------------------
    // BONUS : MENU INTERACTIF (Si tu preferes ne pas recompiler)
    /*
    int choix = 0;
    printf("Quel test voulez-vous lancer ?\n");
    printf("1. Lecture (Etape 1)\n");
    printf("2. Comparaison Initiale (Etape 2)\n");
    printf("3. Optimisation Marche-Pied (Etape 3)\n");
    printf("Votre choix : ");
    scanf("%d", &choix);

    switch(choix) {
        case 1: testerEtape1(nomFichier); break;
        case 2: testerEtape2(nomFichier); break;
        case 3: testerEtape3(nomFichier); break;
        default: printf("Choix invalide.\n");
    }
    */
    // ------------------------------------------------------

    return EXIT_SUCCESS;
}
