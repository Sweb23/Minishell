#include "jobs.h"
#include <string.h>

/** Ajoute un job à la liste des jobs */
void ajouter_job(struct liste_job **job, int pid, int jid, char *cmdline) {
    struct liste_job *new_job = malloc(sizeof(struct liste_job));
    new_job->pid = pid;
    new_job->jid = jid;
    new_job->cmdline = malloc(strlen(cmdline)*sizeof(char));
    new_job->statut = 1; /* actif par defaut */
    new_job->cmdline = strcpy(new_job->cmdline,cmdline); /* Copie de la ligne de commande */
    new_job->next = *job; /* Ajout en tête de liste */
    *job = new_job;
}

void supprimer_job(struct liste_job **job, int jid) {
    struct liste_job *job_courant = *job;
    struct liste_job *job_precedent = NULL;
    while (job_courant != NULL) {
        if (job_courant->jid == jid) {
            /* Le job a été trouvé, suppression de la liste */
            if (job_precedent == NULL) {
                /* Premier élément de la liste */
                *job = job_courant->next;
            } else {
                job_precedent->next = job_courant->next;
            }
            free(job_courant->cmdline);
            free(job_courant);
            return;
        }
        job_precedent = job_courant;
        job_courant = job_courant->next;
    }
}

void afficher(struct liste_job * job) {
    int status;
    printf("JID\tPID\tSTATUS\tCOMMAND\n");
    struct liste_job *job_courant = job;
    while (job_courant != NULL) {
        pid_t pid = waitpid(job_courant->pid, &status, WNOHANG);
        if (pid == -1) {
            printf("Erreur lors de l'appel à waitpid\n");
            return;
        } else if (job_courant->statut == 0) {
            printf("%d\t%d\tsuspendu\t%s\n", job_courant->jid, job_courant->pid, job_courant->cmdline);
        } else {
            printf("%d\t%d\tactif\t%s\n", job_courant->jid, job_courant->pid, job_courant->cmdline);
        } 
        job_courant = job_courant->next;
    }
}


pid_t jid_to_pid(struct liste_job* job, int jid) {
	struct liste_job *job_courant = job;
	while (job_courant != NULL) {
		if (job_courant->jid == jid) {
			return job_courant->pid;
		}
		job_courant = job_courant->next;
	}
	// Pas trouvé : on retourne -1
	return -1;
}


int set_statut(struct liste_job *job, int jid, int statut) {
	struct liste_job *job_courant = job;
	while (job_courant != NULL) {
		if (job_courant->jid == jid) {
			job_courant->statut = statut;
			return 0;
		}
		job_courant = job_courant->next;
	}
	return -1;
}
