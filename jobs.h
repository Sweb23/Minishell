#ifndef JOBS_H
#define JOBS_H

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
# include <unistd.h> /* fork */
#include <sys/wait.h> /* wait */


struct liste_job {
    int jid; /* Identifiant propre au minishell */
    int pid; /* PID du processus */
    char *cmdline; /* Ligne de commande lancée */
    int statut; /* 0 ou 1 si suspendu ou actif */
    struct liste_job *next; /* Pointeur vers le prochain job dans la liste */
};

/** Ajoute un job à la liste des jobs */
void ajouter_job(struct liste_job **job, int pid, int jid, char *cmdline);

/** Supprime un job de la liste */
void supprimer_job(struct liste_job **job, int jid);

/** Affiche la liste des jobs */
void afficher(struct liste_job * job);

/** Trouve le pid du job associé au jid passé en paramètre */
pid_t jid_to_pid(struct liste_job* job, int jid);

/** Modifie le status (actif / suspendu) du job associé au jid passé en paramètre */
int set_statut(struct liste_job *job, int jid, int statut);

#endif
