/** Minishell questions 1 à 6 
Auteur : BOSWELL Ethan / 1SN-B
*/

#define _POSIX_C_SOURCE 200809L /* sinon kill donne un warning */

#include <stdio.h>
#include <stdlib.h>
#include "readcmd.h"
#include <unistd.h> /* fork */
#include <sys/wait.h> /* wait */
#include <sys/types.h> /* kill */
#include <string.h>
#include "jobs.h"
#include <signal.h>

/** Teste si l'appel système a correctement fonctionné
* err : la valeur de retour de l'appel système
* msg : le message d'erreur à afficher en cas de problème
* code : un code d'erreur à spécifier
* * Liste des codes d'erreur :
* * * 1 : Echec de la commande
* * * 2 : Le fork a échoué
* * * 3 : Le changement de dossier a échoué
* * * 4 : Processus inconnu
* * * 5 : Echec de la suspension du processus
* * * 6 : Echec de la reprise du processus
*/
int test(int err, char* msg, int code) {
	if (err == -1) {
		perror(msg);
		exit(code);
	}
	return err;
}



/** Fusionner un tableau de chaine de caractères en une seule chaine */
char* flatten(char** c) {
	const int MAX_SIZE = 100;
	int i = 0;
	char* resultat = malloc(MAX_SIZE*sizeof(char));
	while (c[i] != NULL) {
        strcat(resultat, c[i]);
        strcat(resultat, " ");
        i++;
    }
    return resultat;
}

int main() {
	int pidFils, term, errCmd; // Le PID du processus fils (la commande), la variable modifiée par waitpid(pidFils...) et le code éventuel d'erreur de execvp
	int jid = 1; // L'ID propre au minishell d'un job 
	int pid_to_kill; // PID d'un processus à qui envoyer un signal
	struct cmdline *commande;
	struct liste_job* jobs = NULL; // Liste des jobs du minishell
	
	void handler_sigchld(int sig) {
		sig = sig; // Pour éviter le warning
		int status;
		pid_t pidFils = 0;
		/* On parcourt les fils existants et on vérifie leur statut pour une éventuelle màj */
		do {
			pidFils = waitpid(-1,&status,WNOHANG | WUNTRACED | WCONTINUED);
			if (pidFils > 0) {
				/* Processus terminé */
				if (WIFEXITED(status) || WIFSIGNALED(status)) {
					supprimer_job(&jobs,jid);
					jid--;
				/* Processus suspendu */
				} if (WIFSTOPPED(status)) {
					set_statut(jobs,atoi(commande->seq[0][1]),0);
				/* Processus repris */
				} if (WIFCONTINUED(status)) {
					set_statut(jobs,atoi(commande->seq[0][1]),1);
				}
			}
		} while (pidFils > 0);
	}
	struct sigaction fin_job;
	fin_job.sa_handler = handler_sigchld;
	sigaction(SIGCHLD,&fin_job,NULL);
	
	do {
		printf(">>>");
		commande = readcmd();
		if (!commande) {
            printf("\n--- Erreur de la fonction de saisie ou EOF\n");
            
        }
        else {
            
            if (commande->err != NULL) {
                /* tous les autres champs de commande sont NULL */
                printf("--- Erreur de structure de la commande : %s\n", commande->err);
                
            }
            
            else if (strcmp(commande->seq[0][0], "exit") == 0) {
            	exit(0);
            } 
            else if (strcmp(commande->seq[0][0], "cd") == 0) {
            	test(chdir(commande->seq[0][1]),"Le changement de dossier a echoue", 3);
            }
            else if (strcmp(commande->seq[0][0], "lj") == 0) {
            	afficher(jobs);
            }
            else if (strcmp(commande->seq[0][0], "sj") == 0) {
            	pid_to_kill = test(jid_to_pid(jobs,atoi(commande->seq[0][1])),"Processus inconnu", 4);
            	
            	test(kill(pid_to_kill,SIGSTOP),"Echec de la suspension du processus",5);
           		printf("Processus suspendu\n"); 
            }
            else if (strcmp(commande->seq[0][0], "bg") == 0 || strcmp(commande->seq[0][0], "fg") == 0) {
            	pid_to_kill = test(jid_to_pid(jobs,atoi(commande->seq[0][1])),"Processus inconnu", 4);
            	
            	test(kill(pid_to_kill,SIGCONT),"Echec de la reprise du processus",6);
            	printf("Reprise du processus de PID %d\n",pid_to_kill);
            	/* Si foreground : on fait attendre le père que le fils se termine */
            	if (strcmp(commande->seq[0][0], "fg") == 0) {
            	 	test(waitpid(pid_to_kill,&term,0),"Echec de la commande",1);
            	}
            }
            else {
            	pidFils = test(fork(), "Le fork a echoue",2);
            	if (pidFils == 0) { //fils
            		
            		errCmd = execvp(commande->seq[0][0], commande->seq[0]);
            		if (errCmd == -1) {
            			printf("Erreur dans l'execution de la commande\n");
            			exit(2);
            		}
            	} else { //père
            		if (commande->backgrounded) { // cmd en background : on l'ajoute aux jobs et on attend pas
            			ajouter_job(&jobs,pidFils,jid,flatten(commande->seq[0]));
            			printf("[%d]\t%d\n",jobs->jid,jobs->pid);
            			jid++;
            		}
            		else { /*cmd en foreground : on ne l'ajoute pas aux jobs et on attend.*/
            			
            			test(waitpid(pidFils,&term,WUNTRACED),"Echec de la commande",1);
            		}
            		
            	}
            }
        }
	
	} while (1);
	return EXIT_SUCCESS;
}
