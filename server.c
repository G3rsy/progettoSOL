#include "include.h"

void* thread_function(void* arg);

//sokcet setting
int fd_sok=0;
char* sokName;
int serverIndex = 0;

//pipe anonima
int supervisor = 0;

//lista di thread che gestiscono la connessione tra i client
list* HEAD = NULL;
list* last =  NULL;

static void handler(int num){
	list* toRm = NULL;
	close(fd_sok);

	while(HEAD != NULL){
		pthread_join(HEAD->thread, NULL);

		close(HEAD->fd);

		//scorri di uno
		toRm = HEAD;
		HEAD = HEAD->next;

		//libera quello di prima
		free(toRm);
	}
	free(sokName);
	close(supervisor);

	_exit(0);
}

int main(int argc, char* argv[]){
	if(argc != 3){
		printf("Ho bisogno di 2 argomenti \n");
		return -1;
	}
	
	
	signal(SIGINT, SIG_IGN);
	

	serverIndex = (int)(*argv[1]);
	supervisor = (int)(*argv[2]);
	
	
	//creo il nome per la socket
	sokName = (char*)malloc(20 * sizeof(char));

	memset(sokName, '\0', 20);
	sprintf(sokName, "OOB-server-%d", serverIndex);


	//apertura connessione
	fd_sok = openConn(sokName);
    //-----------------------------------------------------------//


	struct sigaction s;
	memset(&s, 0, sizeof(s));
	s.sa_handler = handler;

	sigaction(SIGTERM, &s, NULL);


    //-----------------------------------------------------------//
	//mi metto in attesa di una connessione da parte di un client
	//dprintf(2, "SERVER %d ACTIVE", serverIndex);

	list* el = NULL;
	int fd_client=0;
	int i =0;
	while(1){
		//lo inizializzo facendo l'accept
		fd_client = accept(fd_sok, NULL, 0);

		//genero un nuovo elemento che rappresenta il thread
		el = (list*) malloc(sizeof(list));
		el->fd=fd_client;
		el->next = NULL;

		if(el->fd != -1){
			//faccio l'inserimento in coda del nuovo thread creato
			if(HEAD == NULL){
				HEAD = el;
				last = el;
			}else{
				last->next = el;
				last = el;
			}

			pthread_create(&(el->thread), NULL, thread_function, el);
		}
		i++;
	}
}

void* thread_function(void* arg){
	list* thread = (list*) arg;

	int readen = 0;
	char fake[20];
	struct timespec clock;

	int oldTime = 0;
	int newTime = 0;
	int newStima = INT_MAX;
	int stima = INT_MAX;

	printf("SERVER %d CONNECTED FROM CLIENT\n", serverIndex);


	readen = readn(thread->fd, (void*)&(thread->id), 8);
	clock_gettime(CLOCK_REALTIME, &(clock));
	oldTime = ((clock.tv_sec%10000)*1000)+ (clock.tv_nsec/1000000);

	//traduco l'id in host bytes order
	thread->id = ntohlx(thread->id);

	printf("SERVER %d INCOMING FROM %lx @ %d\n", serverIndex, thread->id, oldTime);
	while(readen != -1 && readen != 0){

		readen = readn(thread->fd, fake, 8);
		clock_gettime(CLOCK_REALTIME, &(clock));

		newTime = ((clock.tv_sec%10000)*1000)+ (clock.tv_nsec/1000000);
	
		printf("SERVER %d INCOMING FROM %lx @ %d\n", serverIndex, thread->id, newTime);

		newStima = newTime - oldTime;
		if(newStima < stima)
			stima = newStima;

		oldTime = newTime;

	}

	//creo la struttura per inviare la stima su pipe
	stimeFinali client;
	client.id = thread->id;
	client.secret = stima%10000;
	client.contribuenti = 0;
	client.next = NULL;

	if(client.secret != INT_MAX)
		writen(supervisor, (char*)&client, sizeof(stimeFinali));

	printf("SERVER %d CLOSING %lx ESTIMATE %d\n", serverIndex, client.id, client.secret);

	fflush(stdout);

	
	return 0;
}
