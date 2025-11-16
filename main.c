// Fichier: main.c
// "Chef d'orchestre" qui teste les Etapes 1, 2 et 3

#include "projet_ro.h" // On inclut notre "sommaire"
#include <stdio.h>

// -----------------------------------------------------------------
// FONCTION DE TEST POUR L'ETAPE 1
// -----------------------------------------------------------------
void testerEtape1(const char* nomFichier) {
    printf("==================================================\n");
    printf("--- TEST DE L'ETAPE 1 (LECTURE/AFFICHAGE) ---\n");
    printf("Fichier: %s\n", nomFichier);
    printf("==================================================\n");

    // 1. Lecture
    ProblemeTransport* p = lireDonnees(nomFichier);

    // 2. Affichage
    afficherTableauCouts(p);
    afficherTableauSolution(p); // Affiche la solution (initialisee a 0)

    // 3. Liberation
    libererProbleme(p);

    printf("--- FIN TEST ETAPE 1 ---\n\n");
}


// -----------------------------------------------------------------
// FONCTION DE TEST POUR L'ETAPE 2
// -----------------------------------------------------------------
void testerEtape2(const char* nomFichier) {
    printf("==================================================\n");
    printf("--- TEST DE L'ETAPE 2 (SOLUTIONS INITIALES) ---\n");
    printf("Fichier: %s\n", nomFichier);
    printf("==================================================\n");

    // 1. Lecture
    ProblemeTransport* p = lireDonnees(nomFichier);
    afficherTableauCouts(p);

    double cout_no, cout_bh;

    // --- Test Nord-Ouest ---
    printf("\n==================================================\n");
    printf("ALGORITHME: Coin Nord-Ouest\n");
    printf("==================================================\n");

    algoNordOuest(p); // Calcule B
    afficherTableauSolution(p); // Affiche B

    cout_no = calculerCoutTotal(p);
    printf(">>> Cout total (Nord-Ouest): %.2f\n\n", cout_no);


    // --- Test Balas-Hammer ---
    printf("\n==================================================\n");
    printf("ALGORITHME: Balas-Hammer (Vogel)\n");
    printf("==================================================\n");

    algoBalasHammer(p); // Calcule B (en ecrasant la solution N-O)
    afficherTableauSolution(p); // Affiche B

    cout_bh = calculerCoutTotal(p);
    printf(">>> Cout total (Balas-Hammer): %.2f\n\n", cout_bh);


    // --- Comparaison Finale Etape 2 ---
    printf("\n==================================================\n");
    printf("COMPARAISON DES COUTS INITIAUX pour: %s\n", nomFichier);
    printf("==================================================\n");
    printf("* Nord-Ouest   : %.2f\n", cout_no);
    printf("* Balas-Hammer : %.2f\n", cout_bh);
    printf("==================================================\n\n");


    // 4. Liberation de la memoire
    libererProbleme(p);

    printf("--- FIN TEST ETAPE 2 ---\n\n");
}

// -----------------------------------------------------------------
// FONCTION DE TEST POUR L'ETAPE 3 (NOUVEAU)
// -----------------------------------------------------------------
void testerEtape3(const char* nomFichier) {
    printf("==================================================\n");
    printf("--- TEST DE L'ETAPE 3 (OPTIMISATION MARCHE-PIED) ---\n");
    printf("Fichier: %s\n", nomFichier);
    printf("==================================================\n");

    // 1. Lecture
    ProblemeTransport* p = lireDonnees(nomFichier);
    afficherTableauCouts(p);

    // 2. Calculer la meilleure solution initiale (Balas-Hammer)
    // C'est la solution de depart pour l'optimisation
    printf("\n==================================================\n");
    printf("Solution de depart (Balas-Hammer)\n");
    printf("==================================================\n");

    algoBalasHammer(p);
    afficherTableauSolution(p);
    double cout_initial = calculerCoutTotal(p);
    printf(">>> Cout total initial (Balas-Hammer): %.2f\n\n", cout_initial);

    // 3. Lancer l'optimisation (Marche-Pied)
    printf("\n==================================================\n");
    printf("LANCEMENT DE L'OPTIMISATION (MARCHE-PIED)\n");
    printf("==================================================\n");

    algoMarchePied(p); // <-- APPEL DE L'ETAPE 3

    // 4. Afficher le resultat final
    printf("\n==================================================\n");
    printf("RESULTAT FINAL (OPTIMISE)\n");
    printf("==================================================\n");
    double cout_final = calculerCoutTotal(p);
    afficherTableauSolution(p);
    printf(">>> Cout initial (Balas-Hammer): %.2f\n", cout_initial);
    printf(">>> Cout total OPTIMAL: %.2f\n\n", cout_final);

    // 5. Liberation de la memoire
    libererProbleme(p);

    printf("--- FIN TEST ETAPE 3 ---\n\n");
}


// -----------------------------------------------------------------
// LE SEUL ET UNIQUE "VRAI" MAIN
// -----------------------------------------------------------------
int main(int argc, char* argv[]) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <fichier_donnees.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char* nomFichier = argv[1];

    // --- CHOISIS QUEL TEST LANCER ---

    // Decommente pour tester l'Etape 1 (Lecture/Affichage)
    // testerEtape1(nomFichier);

    // Decommente pour tester l'Etape 2 (N-O vs B-H)
    // testerEtape2(nomFichier);

    // Laisse cette ligne active pour tester l'Etape 3 (Optimisation)
    testerEtape3(nomFichier);

    return EXIT_SUCCESS;
}