//Gabriele Sergi
//matricola 532362

#define _GNU_SOURCE
#include "include.h"



figli* son = NULL;
int k=0;



stimeFinali* risultatiH = NULL;

static void handlingPrint(){
	stimeFinali* tmp = risultatiH;
	while(tmp != NULL){
		printf("SUPERVISOR ESTIMATE %d FOR %lx BASED ON %d\n",  tmp->secret, tmp->id, tmp->contribuenti);
		tmp=tmp->next;
	}
	fflush(NULL);		
}

static void cleanKill(){
	stimeFinali* tmp = NULL;

	while(risultatiH != NULL){
		tmp=risultatiH;
		risultatiH = risultatiH->next;
		free(tmp);
	}

	//Mando un sigterm ai server per la chiusura
	//pulita
	for(int i=0; i<k; i++){
		kill(son[i].pid, SIGTERM);
		waitpid(son[i].pid, NULL, WUNTRACED);
		close(son[i].fd[0]);
	}
	free(son);

	printf("SUPERVISOR EXITING\n");
	fflush(NULL);
	_exit(0);
}

volatile sig_atomic_t sigint = 0;


static void handler(int num){

	if(num == SIGINT){
		sigint++;
		if(sigint == 1){
			handlingPrint();
			alarm(1);
		}
		if(sigint == 2)
			cleanKill();
	}

	if(num == SIGALRM && sigint==1){
		sigint = 0;
	}
}



int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Ho bisogno del numero di server da avvaire\n");
		return -1;
	}

	k = atoi(argv[1]);
	int n = 0;

	printf("SUPERVISOR STARTING %d\n", k);	
	fflush(NULL);

	son = malloc(sizeof(figli)*k);
	
	//avvio dei server
	for(int i=0; i<k; i++){

		son[i].nome = i;
		
		if(pipe(son[i].fd) == -1){
			printf("error");
		}else{
			if( !(n = fork()) ){
				//chiudo la pipe in lettura
				close(son[i].fd[0]);

				//figlio
				execl("server.o", "supervisor", &son[i].nome, &son[i].fd[1],(char*)NULL);
			
			}

			//rimango aperto solo in read
			//chiudo la write
			int flags = fcntl(son[i].fd[0], F_GETFL, 0);
			fcntl(son[i].fd[0], F_SETFL, flags | O_NONBLOCK);
			close(son[i].fd[1]);

			son[i].pid = n;
		}
	}




	//Setto la gestione dei segnali
	struct sigaction s;
	memset(&s, 0, sizeof(s));
	s.sa_handler = handler;

	sigaction(SIGINT, &s, NULL);
	sigaction(SIGALRM, &s, NULL);
	//------------------------------//




	//ciclo infinito di lettura dai file descriptor delle pipe
	//termina solo con il segnale di interruzione
	stimeFinali inEl;
	int i=0;
	while(1){

		if(readn(son[i].fd[0], (char*)&inEl, sizeof(stimeFinali)) > 0){
			//devo inserire questa stima all'interno della struttura
			//------scorro fino a trovare il punto giusto della lista------//

			printf("SUPERVISOR ESTIMATE %d FOR %lx FROM %d\n", inEl.secret, inEl.id, i);
			stimeFinali* tmp = risultatiH;
			while(tmp != NULL && tmp->id != inEl.id)
				tmp = tmp->next;



			if(tmp == NULL){
				//devo inserire l'elemento nella lista
				stimeFinali* insert = (stimeFinali*) malloc(sizeof(stimeFinali));
				
				insert->id= inEl.id;
				insert->secret=inEl.secret;
				insert->contribuenti=1;

				insert->next = risultatiH;

				risultatiH = insert;
			}else{
				tmp->contribuenti++;
				//ricalcolo la stima e libero l'elemento allocato in precedenza
				if(tmp->secret > inEl.secret)
					tmp->secret = inEl.secret;
			}

		}
			
		fflush(NULL);
		i = (i+1)%k;
		pthread_yield();
	}
}