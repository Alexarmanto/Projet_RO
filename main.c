/**
 * @file main.c
 * @brief Programme principal interactif pour le projet de Recherche Operationnelle.
 * Version : Choix du mode (Fichier vs Complexité) au démarrage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "projet_ro.h"

// ==========================================================
// UTILITAIRES D'AFFICHAGE
// ==========================================================

void afficher_banniere() {
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
// MENUS
// ==========================================================

void afficher_menu_mode_general() {
    printf("\n=== MENU PRINCIPAL ===\n");
    printf(" 1. Charger et resoudre un probleme (Fichier)\n");
    printf(" 2. Lancer l'etude de complexite (Automatique)\n");
    printf(" 0. Quitter le programme\n");
    printf("----------------------\n");
}

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
    printf(" 0.  Retour au menu principal\n");
    printf("------------------------------\n");
}

void afficher_menu_actions() {
    printf("\n--- MENU ACTIONS (Sur le fichier en cours) ---\n");
    printf(" 1. Afficher les donnees du probleme\n");
    printf(" 2. Comparer Nord-Ouest et Balas-Hammer\n");
    printf(" 3. Pipeline : Balas-Hammer + Marche-Pied\n");
    printf(" 4. Pipeline : Nord-Ouest + Marche-Pied\n");
    // Option 5 supprimée ici car déplacée au menu principal
    printf(" 0. Fermer ce fichier et revenir au menu principal\n");
    afficher_ligne();
}

// ==========================================================
// LOGIQUE DE CHARGEMENT
// ==========================================================

ProblemeTransport* charger_probleme_interactif() {
    ProblemeTransport* p = NULL;
    char nom_fichier[256];
    int choix_fichier = -1;

    while (p == NULL) {
        afficher_menu_fichiers();
        printf("Votre choix : ");

        if (scanf("%d", &choix_fichier) != 1) {
            printf("\n/!\\ Entree invalide.\n");
            while(getchar() != '\n'); // Vider buffer
            continue;
        }

        if (choix_fichier == 0) {
            return NULL; // Retour menu principal
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
            printf("\n/!\\ Choix invalide.\n");
            continue;
        }

        // Tentative de chargement
        p = lireDonnees(nom_fichier);

        if (p == NULL) {
            printf("\n/!\\ ERREUR : Impossible de charger '%s'\n", nom_fichier);
            int reessayer = 0;
            printf("Reessayer ? (1: Oui, 0: Non) : ");
            scanf("%d", &reessayer);
            if (reessayer == 0) return NULL;
            continue;
        }

        // Verification equilibre
        int sum_P = 0, sum_C = 0;
        for (int i = 0; i < p->n; i++) sum_P += p->P[i];
        for (int j = 0; j < p->m; j++) sum_C += p->C[j];

        if (sum_P != sum_C) {
            printf("\n/!\\ ALERTE : PROBLEME NON EQUILIBRE (%d vs %d)\n", sum_P, sum_C);
            libererProbleme(p);
            p = NULL;
            int reessayer = 0;
            printf("Charger un autre ? (1: Oui, 0: Non) : ");
            scanf("%d", &reessayer);
            if (reessayer == 0) return NULL;
            continue;
        }

        printf("\n[OK] FICHIER CHARGE AVEC SUCCES\n");
    }
    return p;
}

// ==========================================================
// ACTIONS
// ==========================================================

void action_afficher_donnees(ProblemeTransport* p) {
    printf("\n[ACTION] Affichage des donnees\n");
    afficher_separateur();
    afficherTableauCouts(p);
    afficherTableauSolution(p);
    printf(">>> Cout actuel : %.2f\n", calculerCoutTotal(p));
}

void action_comparer_initiaux(ProblemeTransport* p) {
    printf("\n[ACTION] Comparaison Nord-Ouest vs Balas-Hammer\n");
    afficher_separateur();

    // Nord Ouest
    algoNordOuest(p);
    double cout_no = calculerCoutTotal(p);
    printf(" -> Cout Nord-Ouest   : %.2f\n", cout_no);

    // Balas Hammer
    algoBalasHammer(p);
    double cout_bh = calculerCoutTotal(p);
    printf(" -> Cout Balas-Hammer : %.2f\n", cout_bh);

    afficher_ligne();
    if (cout_bh < cout_no) printf(" [CONCLUSION] Balas-Hammer est meilleur.\n");
    else if (cout_no < cout_bh) printf(" [CONCLUSION] Nord-Ouest est meilleur.\n");
    else printf(" [CONCLUSION] Resultats identiques.\n");
}

void action_pipeline_balas(ProblemeTransport* p) {
    printf("\n[ACTION] Pipeline : Balas-Hammer + Marche-Pied\n");
    afficher_separateur();

    algoBalasHammerSilencieux(p);
    double c1 = calculerCoutTotal(p);
    printf("1. Initial (BH) : %.2f\n", c1);

    algoMarchePied(p);
    double c2 = calculerCoutTotal(p);
    printf("2. Final (MP)   : %.2f\n", c2);

    afficherTableauSolution(p);
}

void action_pipeline_nordouest(ProblemeTransport* p) {
    printf("\n[ACTION] Pipeline : Nord-Ouest + Marche-Pied\n");
    afficher_separateur();

    algoNordOuestSilencieux(p);
    double c1 = calculerCoutTotal(p);
    printf("1. Initial (NO) : %.2f\n", c1);

    algoMarchePied(p);
    double c2 = calculerCoutTotal(p);
    printf("2. Final (MP)   : %.2f\n", c2);

    afficherTableauSolution(p);
}

// ==========================================================
// MAIN RESTRUCTURE
// ==========================================================

int main(int argc, char* argv[]) {
    afficher_banniere();

    // MODE BATCH (Ligne de commande) - inchangé
    if (argc >= 2) {
        // ... (Code batch identique à votre version précédente)
        // Je l'omets ici pour la clarté, mais gardez votre bloc batch si nécessaire
        printf("Mode Batch non disponible dans cet extrait simplifie.\n");
        return 0;
    }

    // MODE INTERACTIF
    int choix_mode = -1;

    while (choix_mode != 0) {
        // 1. Menu Principal (Le changement demandé est ici)
        afficher_menu_mode_general();
        printf("Votre choix : ");
        if (scanf("%d", &choix_mode) != 1) {
            while(getchar() != '\n'); continue;
        }

        switch (choix_mode) {
            case 0: // Quitter
                printf("\nAu revoir !\n");
                break;

            case 2: // Etude de complexité
                // Appel direct à l'étude, sans charger de fichier
                lancer_etude_complete();
                break;

            case 1: // Travailler sur un fichier
            {
                // Appel du menu de sélection de fichier
                ProblemeTransport* p = charger_probleme_interactif();

                // Si un fichier a bien été chargé, on entre dans la boucle d'actions
                if (p != NULL) {
                    int choix_action = -1;
                    while (choix_action != 0) {
                        afficher_menu_actions();
                        printf("Votre choix : ");
                        if (scanf("%d", &choix_action) != 1) {
                            while(getchar() != '\n'); continue;
                        }

                        switch (choix_action) {
                            case 1: action_afficher_donnees(p); break;
                            case 2: action_comparer_initiaux(p); break;
                            case 3: action_pipeline_balas(p); break;
                            case 4: action_pipeline_nordouest(p); break;
                            case 0:
                                printf("\nFermeture du fichier.\n");
                                break;
                            default: printf("\nChoix invalide.\n"); break;
                        }
                    }
                    // Fin de la boucle d'actions : on libère la mémoire
                    libererProbleme(p);
                    p = NULL;
                }
                break;
            }

            default:
                printf("\nOption inconnue.\n");
                break;
        }
    }

    return EXIT_SUCCESS;
}
