/**
 * @file main.c
 * @brief Programme principal interactif pour le projet de Recherche Operationnelle.
 * Version simplifiee avec actions essentielles uniquement.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "projet_ro.h"

// ==========================================================
// UTILITAIRES D'AFFICHAGE (ASCII COMPATIBLE)
// ==========================================================

void afficher_menu_principal() {
    printf("\n");
    printf("============================================================\n");
    printf("     PROJET RECHERCHE OPERATIONNELLE - TRANSPORT           \n");
    printf("============================================================\n");
    printf("\n");
}

void afficher_separateur() {
    printf("\n============================================================\n\n");
}

void afficher_ligne() {
    printf("------------------------------------------------------------\n");
}

// ==========================================================
// SELECTION DE FICHIER
// ==========================================================

void afficher_menu_fichiers() {
    printf("\n--- SELECTION DU PROBLEME ---\n");
    printf(" 1.  prop1.txt\n");
    printf(" 2.  prop2.txt\n");
    printf(" 3.  prop3.txt\n");
    printf(" 4.  prop4.txt\n");
    printf(" 5.  prop5.txt\n");
    printf(" 6.  prop6.txt\n");
    printf(" 7.  prop7.txt\n");
    printf(" 8.  prop8.txt\n");
    printf(" 9.  prop9.txt\n");
    printf(" 10. prop10.txt\n");
    printf(" 11. prop11.txt\n");
    printf(" 12. prop12.txt\n");
    printf(" 13. Fichier personnalise (saisie manuelle)\n");
    printf(" 0.  Quitter\n");
    printf("------------------------------\n");
}

ProblemeTransport* charger_probleme_interactif() {
    ProblemeTransport* p = NULL;
    char nom_fichier[256];
    int choix_fichier = -1;

    while (p == NULL) {
        afficher_menu_fichiers();
        printf("Votre choix : ");

        if (scanf("%d", &choix_fichier) != 1) {
            printf("\n/!\\ Entree invalide.\n");
            while(getchar() != '\n'); // Vider le buffer
            continue;
        }

        if (choix_fichier == 0) {
            return NULL; // Quitter
        }
        else if (choix_fichier >= 1 && choix_fichier <= 12) {
            sprintf(nom_fichier, "prop%d.txt", choix_fichier);
            printf("\n--> Chargement de : %s\n", nom_fichier);
        }
        else if (choix_fichier == 13) {
            printf("\nEntrez le nom complet du fichier : ");
            scanf("%s", nom_fichier);
        }
        else {
            printf("\n/!\\ Choix invalide. Reessayez.\n");
            continue;
        }

        // Tentative de chargement
        p = lireDonnees(nom_fichier);

        if (p == NULL) {
            printf("\n============================================================\n");
            printf("  /!\\ ERREUR : Impossible de charger '%s'\n", nom_fichier);
            printf("      Verifiez que le fichier existe et est au bon format.\n");
            printf("============================================================\n");

            int reessayer = 0;
            printf("\nReessayer ? (1: Oui, 0: Quitter) : ");
            scanf("%d", &reessayer);
            if (reessayer == 0) return NULL;
            continue;
        }

        // Verification de l'equilibre
        int sum_P = 0, sum_C = 0;
        for (int i = 0; i < p->n; i++) sum_P += p->P[i];
        for (int j = 0; j < p->m; j++) sum_C += p->C[j];

        if (sum_P != sum_C) {
            printf("\n============================================================\n");
            printf("  /!\\ ALERTE : PROBLEME NON EQUILIBRE\n");
            printf("============================================================\n");
            printf("  Total provisions : %d\n", sum_P);
            printf("  Total commandes  : %d\n", sum_C);
            printf("  Difference       : %d\n", abs(sum_P - sum_C));
            printf("\n  Le programme ne traite que les cas equilibres.\n");
            printf("============================================================\n");

            libererProbleme(p);
            p = NULL;

            int reessayer = 0;
            printf("\nCharger un autre fichier ? (1: Oui, 0: Quitter) : ");
            scanf("%d", &reessayer);
            if (reessayer == 0) return NULL;
            continue;
        }

        // Succes !
        printf("\n============================================================\n");
        printf("  [OK] FICHIER CHARGE AVEC SUCCES\n");
        printf("============================================================\n");
    }

    return p;
}

// ==========================================================
// MENU ACTIONS (SIMPLIFIE)
// ==========================================================

void afficher_menu_actions() {
    printf("\n--- MENU ACTIONS ---\n");
    printf(" 1. Afficher les donnees du probleme\n");
    printf(" 2. Comparer Nord-Ouest et Balas-Hammer\n");
    printf(" 3. Balas-Hammer + Marche-Pied\n");
    printf(" 4. Nord-Ouest + Marche-Pied\n");
    printf(" 0. Charger un nouveau fichier\n");
    afficher_ligne();
}

// ==========================================================
// ACTIONS ESSENTIELLES
// ==========================================================

void action_afficher_donnees(ProblemeTransport* p) {
    printf("\n[ACTION] Affichage des donnees du probleme\n");
    afficher_separateur();
    afficherTableauCouts(p);
    afficherTableauSolution(p);
    double cout = calculerCoutTotal(p);
    printf("\n>>> Cout actuel : %.2f\n", cout);
}

void action_comparer_initiaux(ProblemeTransport* p) {
    printf("\n[ACTION] Comparaison des methodes initiales\n");
    afficher_separateur();

    printf("\n--- [1/2] Test NORD-OUEST ---\n");
    algoNordOuest(p);
    afficherTableauSolution(p);
    double cout_no = calculerCoutTotal(p);

    printf("\n--- [2/2] Test BALAS-HAMMER ---\n");
    algoBalasHammer(p);
    afficherTableauSolution(p);
    double cout_bh = calculerCoutTotal(p);

    printf("\n============================================================\n");
    printf("               BILAN COMPARATIF\n");
    printf("============================================================\n");
    printf(" -> Cout Nord-Ouest   : %12.2f\n", cout_no);
    printf(" -> Cout Balas-Hammer : %12.2f\n", cout_bh);
    afficher_ligne();

    if (cout_bh < cout_no) {
        printf(" [OK] Balas-Hammer est MEILLEUR (Gain : %.2f)\n", cout_no - cout_bh);
    } else if (cout_no < cout_bh) {
        printf(" [OK] Nord-Ouest est MEILLEUR (Gain : %.2f)\n", cout_bh - cout_no);
    } else {
        printf(" [=] Les deux methodes donnent le MEME resultat.\n");
    }
}

void action_pipeline_balas(ProblemeTransport* p) {
    printf("\n[ACTION] Pipeline : Balas-Hammer + Marche-Pied\n");
    afficher_separateur();

    printf("\n--- ETAPE 1/2 : Solution initiale (Balas-Hammer) ---\n");
    algoBalasHammer(p);
    double cout_init = calculerCoutTotal(p);
    afficherTableauSolution(p);
    printf(">>> Cout Balas-Hammer : %.2f\n", cout_init);

    printf("\n--- ETAPE 2/2 : Optimisation (Marche-Pied) ---\n");
    algoMarchePied(p);
    double cout_final = calculerCoutTotal(p);

    printf("\n============================================================\n");
    printf("           RESULTAT FINAL OPTIMISE\n");
    printf("============================================================\n");
    afficherTableauSolution(p);
    printf("\n -> Cout initial : %12.2f\n", cout_init);
    printf(" -> Cout final   : %12.2f\n", cout_final);
    printf(" -> Gain total   : %12.2f\n", cout_init - cout_final);
}

void action_pipeline_nordouest(ProblemeTransport* p) {
    printf("\n[ACTION] Pipeline : Nord-Ouest + Marche-Pied\n");
    afficher_separateur();

    printf("\n--- ETAPE 1/2 : Solution initiale (Nord-Ouest) ---\n");
    algoNordOuest(p);
    double cout_init = calculerCoutTotal(p);
    afficherTableauSolution(p);
    printf(">>> Cout Nord-Ouest : %.2f\n", cout_init);

    printf("\n--- ETAPE 2/2 : Optimisation (Marche-Pied) ---\n");
    algoMarchePied(p);
    double cout_final = calculerCoutTotal(p);

    printf("\n============================================================\n");
    printf("           RESULTAT FINAL OPTIMISE\n");
    printf("============================================================\n");
    afficherTableauSolution(p);
    printf("\n -> Cout initial : %12.2f\n", cout_init);
    printf(" -> Cout final   : %12.2f\n", cout_final);
    printf(" -> Gain total   : %12.2f\n", cout_init - cout_final);
}

// ==========================================================
// MAIN
// ==========================================================

int main(int argc, char* argv[]) {

    afficher_menu_principal();

    ProblemeTransport* p = NULL;
    int continuer_programme = 1;

    // Mode ligne de commande - TRAITEMENT MULTIPLE
    if (argc >= 2) {
        printf("Mode batch : Traitement de %d fichier(s).\n\n", argc - 1);

        for (int arg_idx = 1; arg_idx < argc; arg_idx++) {
            printf("\n");
            printf("############################################################\n");
            printf("### FICHIER %d/%d : %s\n", arg_idx, argc - 1, argv[arg_idx]);
            printf("############################################################\n\n");

            p = lireDonnees(argv[arg_idx]);

            if (p == NULL) {
                fprintf(stderr, "[ERREUR] Impossible de charger '%s'. Passage au suivant.\n", argv[arg_idx]);
                continue;
            }

            // Verification equilibre
            int sum_P = 0, sum_C = 0;
            for (int i = 0; i < p->n; i++) sum_P += p->P[i];
            for (int j = 0; j < p->m; j++) sum_C += p->C[j];

            if (sum_P != sum_C) {
                fprintf(stderr, "[ERREUR] Probleme non equilibre (%d != %d). Passage au suivant.\n", sum_P, sum_C);
                libererProbleme(p);
                p = NULL;
                continue;
            }

            printf("[OK] Fichier charge (%dx%d, flux=%d).\n", p->n, p->m, sum_P);

            // EXECUTION AUTOMATIQUE : Pipeline Balas-Hammer + Marche-Pied
            printf("\n>>> EXECUTION AUTOMATIQUE : Pipeline Balas-Hammer + Marche-Pied\n");
            action_pipeline_balas(p);

            // Liberation
            libererProbleme(p);
            p = NULL;

            printf("\n>>> Fichier traite. Memoire liberee.\n");
        }

        printf("\n");
        printf("############################################################\n");
        printf("### TRAITEMENT TERMINE : %d fichiers traites\n", argc - 1);
        printf("############################################################\n\n");

        return EXIT_SUCCESS;
    }

    // Boucle principale (Mode interactif)
    while (continuer_programme == 1) {

        // Charger un fichier si necessaire
        if (p == NULL) {
            p = charger_probleme_interactif();
            if (p == NULL) {
                printf("\n>>> Fin du programme.\n");
                break;
            }
        }

        // Afficher menu et lire choix
        afficher_menu_actions();
        int choix = -1;
        printf("Votre choix : ");

        if (scanf("%d", &choix) != 1) {
            printf("\n/!\\ Entree invalide.\n");
            while(getchar() != '\n');
            continue;
        }

        // Executer action
        switch (choix) {
            case 1:
                action_afficher_donnees(p);
                break;
            case 2:
                action_comparer_initiaux(p);
                break;
            case 3:
                action_pipeline_balas(p);
                break;
            case 4:
                action_pipeline_nordouest(p);
                break;
            case 0:
                libererProbleme(p);
                p = NULL;
                printf("\n>>> Memoire liberee. Pret pour un nouveau fichier.\n");
                break;
            default:
                printf("\n/!\\ Choix invalide.\n");
                break;
        }

        // Demander si continuer
        if (choix != 0 && p != NULL) {
            afficher_ligne();
            printf("Actions : 1=Continuer, 0=Nouveau fichier, -1=Quitter : ");
            int continuer_actions = 0;
            scanf("%d", &continuer_actions);

            if (continuer_actions == -1) {
                continuer_programme = 0;
            } else if (continuer_actions == 0) {
                libererProbleme(p);
                p = NULL;
                printf("\n>>> Memoire liberee.\n");
            }
        }
    }

    // Nettoyage final
    if (p != NULL) {
        libererProbleme(p);
    }

    printf("\n============================================================\n");
    printf("              FIN DU PROGRAMME - MERCI !\n");
    printf("============================================================\n\n");

    return EXIT_SUCCESS;
}
