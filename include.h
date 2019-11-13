//Gabriele Sergi
//matricola 532362

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <inttypes.h>
#include <limits.h>
#include <sys/wait.h>
#include <signal.h>


#define FALSE 0
#define TRUE 1
#define SOK_BUF_SIZE 8

//utilizzata dal client
//per aprire le connessioni
//verso i server
typedef struct{
	int sNum;
	char* sName;
	int sFd;
}server;


//usata dal supervisor
typedef struct{
	pid_t pid;
	int nome;
	int fd[2];
}figli;


typedef struct st{
	uint64_t id;
	int secret;
	int contribuenti;
	struct st* next;
}stimeFinali;



//utilizzata da server
//per tenere traccia dei
//thread creati per gestire
//i client
typedef struct lista{
	//to free
	pthread_t thread;
	int fd;
	uint64_t id;
	//-----//

	struct lista* next;
}list;

/*
typedef struct tim{
	struct timespec val;
	struct tim* next;
}mineTime;
*/


//--------------------------------------
//operazioni che definiscono la comunicazione tra
//client e server
int openConn(char* sokName){
	unlink(sokName);

	//setting della struct della socket
	struct sockaddr_un sa;
	memset(&sa, '\0', sizeof(sa));
	strncpy(sa.sun_path, sokName, strlen(sokName));
	sa.sun_family = AF_UNIX;

	//creazione della socket
	int fd_sok = 0;
	if((fd_sok = socket(AF_UNIX, SOCK_STREAM, 0))== -1){
		perror("Creazione socket");
		return -1;
	}
	if(bind(fd_sok, (struct sockaddr*) &sa, sizeof(sa)) == -1){
		perror("Socket bind");
		return -1;
	}
	listen(fd_sok, SOMAXCONN);
	
	return fd_sok;
}

int directConnect(char* sokName){

	int fd_sok = 0;

	struct sockaddr_un sa;
	memset(&sa, 0, sizeof(sa));
	strncpy(sa.sun_path, sokName, strlen(sokName));
	sa.sun_family = AF_UNIX;
	fd_sok = socket(AF_UNIX, SOCK_STREAM, 0);

	//Cerco di effettura la connessione verso il server
	if(connect(fd_sok, (struct sockaddr*) &sa, sizeof(sa)) == -1){
		perror("Errore connessione");
		return FALSE;
	}

	return fd_sok;
}

static inline int writen(long fd, char *buf, size_t size) {
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;
    while(left>0) {
	if ((r=write((int)fd ,bufptr,left)) == -1) {
	    if (errno == EINTR) continue;
	    return -1;
	}
	if (r == 0) return 0;  
        left    -= r;
	bufptr  += r;
    }
    return 1;
}

static inline int readn(long fd, char *buf, size_t size){
    size_t left = size;
    int r;
    char *bufptr = (char*)buf;

    while(left>0) {
		if ((r=read((int)fd ,bufptr,left)) == -1) {
	    	if (errno == EINTR) continue;
	    	
	    	return -1;
		}
		if (r == 0) return 0;   // gestione chiusura socket
        left    -= r;
		bufptr  += r;
    }
    return size;
}

int sendMsg(void* msg, int fd_sok){
	
	if(writen(fd_sok, msg, SOK_BUF_SIZE) == -1){
		perror("Errore nell'invio del messaggio");
		return FALSE;
	}
	return TRUE;
}

//------------------------------------------------------
//funzione che inizializza la struttura del client
//dove si tiene traccia di tutte le connessini ai server
server* initServerArr(int p, int k){
	//flag array che serve per tener traccia dei server a cui ci siamo connessi
	//kArr[i] = 0 se non siamo connessi
	//kArr[i] = 1 se abbiamo gia' effettuato la connessione verso quel server
	bool* kArr = (bool*) calloc(k, sizeof(bool));

	//array che serve per immagazinare i fd dei server a cui siamo connessi
	server* pArr = (server*) calloc(p, sizeof(server));
	
	for(int i=0; i<p; i++){
		int servNum = rand()%k;

		//verifico se questo server e' gia stato utilizzato per 
		//una connessione cercandolo nell'array kArr 
		while(kArr[servNum]!=0){
			servNum++;
			servNum = servNum%k;
		}
		kArr[servNum] = true;

		//qui ci vuole la connessione con directConnect
		pArr[i].sNum = servNum;

		//genero la stringa per far connettere il server
		pArr[i].sName = malloc(sizeof(char)*20);

		memset(pArr[i].sName, '\0', 20);
		sprintf(pArr[i].sName, "OOB-server-%d", pArr[i].sNum);

		//mi connetto al server
		pArr[i].sFd = directConnect(pArr[i].sName);
		if(pArr[i].sFd == 0)
			printf("Cosa non va? %s\n", pArr[i].sName);
	}

	free(kArr);
	
	return pArr;
}

//-------------------------------------------
//formattazione del messaggio 
//da inviare da client a server
//
//
int isLilEnd(){
	int x=1;
	char* y = (char*)&x;
	return *y+48;
}

uint64_t htonlx(uint64_t x){
	uint32_t x1 = (uint32_t)(x>>32);
	uint32_t x2 = (uint32_t)x;

	
	uint64_t reverse = htonl(x2);
	reverse= reverse<<32;
	reverse = reverse + htonl(x1);

	return reverse;
}

uint64_t ntohlx(uint64_t x){
	if(isLilEnd()){

		uint32_t x1 = ntohl((uint32_t)(x>>32));
		uint32_t x2 = ntohl((uint32_t)x);
		uint64_t reverse = x2;
		reverse = reverse << 32;
		reverse += x1;

		return reverse;
	
	}else
		return x;
}
