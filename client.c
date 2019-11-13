//Gabriele Sergi
//matricola 532362

#include "include.h"

uint64_t ID = 0;
uint64_t nboID = 0;

int secret = 0;

uint64_t ID_gen();

int main(int argc, char* argv[]){
	if(argc != 4){
		printf("Ho bisogno di 3 elementi per poter funzionare\n");
		return -1;
	}

	//ignoro il siegnale di sigint
	signal(SIGINT, SIG_IGN);


	//p -> numero di server a cui collegarsi
	//k -> numero totale dei server (uguale al valore che viene passato al supervisor)
	//w -> numero totale di messaggi inviati dal client
	int p=0, k=0, w=0;
	
	p = strtol(argv[1], NULL, 0);
	k = strtol(argv[2], NULL, 0);
	w = strtol(argv[3], NULL, 0);

	//-------------CONTROLLO INPUT-----------//
	if(p == 0){
		printf("Non posso collegarmi a ZERO server\n");
		return -1;
	}
	
	if(p > k){
		printf("Non posso collegarmi con piu' server di quanti ne esistano\n");
		return -1;
	}
	
	if(w < (3*p)){
		printf("devo inviare almeno 3p messaggi\n");
		return -1;
	}
	if(w<0 || p<0){
		printf("Non sono ammessi valori negativi\n");
		return -1;
	}
	//---------------------------------------//





	//------INIZIALIZZAZIONE VARIABILI-------//
	//inizializzo il seed per la generazione di numeri randomici
	srand((unsigned int) time(NULL) * (unsigned int)getpid());

	//genero l'ID e il secret
	ID = ID_gen();
	nboID = htonlx(ID);

	secret = (int)rand()%3000;
	
	printf("CLIENT %lx SECRET %d\n", ID, secret);


	//setto il timespec per la nanosleep
	struct timespec tm;
	int sec=secret/1000;

	//converto da milli a nano secondi
	tm.tv_nsec = (secret%1000)*1000000;
	tm.tv_sec = sec;


	//ora creo un array lungo p, mi serve per tenere traccia dei serever con i quali dovro' comunicare
	//lo inizializzo con p server scelti a caso tra i k, ed effettuo la connessione con li stessi
	server* serverArr =  initServerArr(p, k);
	//---------------------------------------//



	//---------INIZIO COMUNICAZIONE----------//


	//parte il ciclo di invio dei messaggi
	for(int i=0; i<w; i++){
		//devo inviare il messaggio ad un server a caso
		//tra i p scelti
		int r = rand()%p;

		if(!sendMsg(&nboID, serverArr[r].sFd))
			printf("Failure\n");
		
		//Aspetto secret nanosecondi		
		if(nanosleep(&tm, NULL) != 0)
			perror("Nanosleep failed");		
	}
	//-------------------------------------//


	//CLEANUP
	for(int i=0; i<p; i++){
		free(serverArr[i].sName);
		close(serverArr[i].sFd);
	}
	free(serverArr);
	
	//

	//----------------FINE-----------------//

	printf("CLIENT %lx DONE\n", ID);
	return 0;
}

uint64_t ID_gen(){
	uint64_t num = 0;

	//creo un numero di 64 bit
	//shifto a sinistra e insierisco il nuovo bit in fondo
	for(int i=0; i<64; i++)
		num = num*2 + rand()%2;

	return num;
}
