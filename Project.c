/* Progetto Api 2019 */

/*******************************************************************************
Risolvero il prolema unendo una tabella di Hash ad una lista
La tabella di hash terrà conto delle entità, le entità conterranno al loro 
interno gli indirizzi di memoria di tutte le relazioni che ha inviato.
Le relazioni sono sottoforma di puntatori.
Ogni blocco contiene il nome della relazione ricevente ed un contatore.

Ho una lista di relazioni.
Ad ogni relaione sono concatenate le entità che hanno ricevuto almeno una
relazione di quella tipologia.
All'interno di questa lista è presente anche un contatore, per selezionare quella con 
con maggiori relazioni ricevute.


La tabella di Hash sfrutterà l'algoritmo della moltiplicazione di Knuth, efficiente
per quanto riguarda i nomi e di semplice implementazione.
La tabella sfrutta un doppiohasing, è un ibridio tra l'algoritmo di moltiplicazione, con il 
numero di nepero e l'incremento lineare.

Importante sono le funzioni di cancellazione, 
X)Nel caso in cui cancello una relazione.
1) verifico se le entità esistono.
2) rimuovo il puntatore dell'entità sorgente.
3) decremento il counter nella lista dell'entità ricevente.


X)Nel caso in cui cancello un entità
1) decremento tutte le liste tramite i puntatori
2) effettuo una free del blocco di relazioni
3) cancello l'entità
4) la marchio come "###", al fine di non danneggiare il doppiohasing


Come valori imponiamo un numero infinito di possibili entità
Ogni entità potrà stringere un massimo di 570 relazioni per tipo di relazione
nel caso non dovesse bastare, ci regoleremo di conseguenza

ricordiamo che 570 relazioni x 64 bit = 36480b = 4,56 KB
supponiamo almeno 500 entità attive = 18.240.00 b = 2,28 MB

poichè il test con limite minore è di 9MB, mi sembrano dei parametri accettabili
********************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
 
/**************************************/

//                            Default
#define LINE_SIZE 256         //256
#define MAXREL_TYPE 20        //50
#define MAXREL 570            //570
#define NAME_SIZE 35          //35
#define HASH_SIZE 8192        //8192
#define MAX_PRINT 1000         //500
#define TRASH_SIZE 500        //500

/**************************************/


/*Elenco di potenza del 2 per testare l'HashTable:*/
/* 8192 2^13 - 16384 2^14 - 32768 2^15 - 65536 2^16 
 131072 2^17 - 262144 2^18 - 524288 2^19 - 1048576 2^20*/




/*************************Strutture per la lista di relazioni*************************************/



struct relazione* relazioni[MAXREL_TYPE];


/*Struttura Lista delle relzioni*/
typedef struct relazione {         
	char* tipo;            
	struct relazione* next;
	struct node* tail;
} relazione;



/*Nodo di una lista di un tipo di relazione*/
typedef struct node {         
	char* nome;            
	unsigned long int n;
	struct node* next;
} node;



/*Funzione per inserire una relazione nella lista delle relazioni*/
void insertrel(char* name, int j) {
	struct relazione* rel = (struct relazione*)malloc(sizeof(struct relazione));
	char* relname = (char*)malloc(sizeof(char)*strlen(name));	
	strcpy(relname, name);
	rel->tipo = relname;
	rel->tail = NULL;
	relazioni[j] = rel;
}






/*************************Strutture per l'hash table*************************************/



/*Struttura entità HashTable*/
typedef struct entity {        
	char* name;             //nome entità
	node** rel;             //puntatore all'array di relazioni inviate (puntatori)
} entity;



/*Blocco inizale di memoria per le hash table*/
struct entity* hashtable[HASH_SIZE];


/*spazzatura*/
struct node* trash[TRASH_SIZE];      //lo userò per riordinare l'array di relazioni inviate
                                     //qualora un DelEnt eliminasse una relazione
                         	         //mantenere quel puntatore potrebbe causaremi problemi								//in caso di un addent con la relazione precedente
									 //in caso di un addent con la relazione precedente

/*nodo vuoto*/                      
struct entity* empty;				//necessario in casco di DELENT
node** VUOTO;						//Devo crearmi un entità con nome = "###"
node* vuot[1];						//ed un array di puntatori vuoto per risparmiare spazio





/***************************************Funzioni****************************************/



/*Convete in nome nel corrispettivo numero ascii modulo HASH_SIZE*/
unsigned long int name_to_ascii_hashmod (char* name){
	int i = 1;
	unsigned long int d;
	unsigned long int k;

	k = 0 ;
	d = name[0];

	while(name[i] != '\0'){
		k = name[i];
		if(k < 100) {
			d = d*10;
			d = ((d + k) % HASH_SIZE);
		}
		else {
			d = d*100;
			d = ((d + k) % HASH_SIZE);
		}
		i++;
	}
	return d;
}



/*Lettore riga*/
void readline(char* line) {
	int i = 0;
	char ch;
	while ((ch=getchar()) != EOF){
		if (ch == '\n') break;
		line[i] = ch;
		i++;
	}
	line[i] = '\0';
}



/*Funzione di hash, metodo moltiplicazione*/
unsigned long int hashfunction(unsigned long int ascii) { 
	unsigned long int key;
	double A = 0.61803398875;    //Uso come parametro il numero di Knuth [(rad(5)-1)/2]
	double i = ascii;            //Converto la stringa in un intero
	double d;
	key = (HASH_SIZE * modf(A*i, &d));	
	return key;
}



/*Funzione di doppio hash in caso di collisione*/
unsigned long int doublehashing(unsigned long int ascii/*,int num*/){
	unsigned long int key;
	double A = 0.718281828459;    //Uso come parametro la parte decimale del numero di nepero
	double i = ascii;        //Converto la stringa in un intero
	double d;
	key = (HASH_SIZE * modf(A*i, &d));	
	return key;
}



/* Funzione di ricerca (Se Hash Esiste, ritorna 1; altrimenti ritorna la cella dove inserire l'entità*/
unsigned long int search(char* name) {
	unsigned long int ascii_value;
	unsigned long int hash, hash2, emp;
	unsigned long int num;
	unsigned long int i;
	emp = 0;
	i = 0;
	num = 0;
	ascii_value = (name_to_ascii_hashmod(name));
	hash = (hashfunction(ascii_value));
	hash2 = doublehashing(ascii_value);
	while(hashtable[hash] != NULL){
		if(strcmp(hashtable[hash]->name,name)== 0){
			return 1;
		}
		else{
			if((strcmp(hashtable[hash]->name,"###") == 0) && i == 0){
				emp = hash; i++;
			}
			hash = ((hash2 + num) % HASH_SIZE);
			num++;
		}	
	}
	if(emp != 0){ hash = emp;}
	return hash;
}	



/* Funzione di controllo  (Se l'entità esiste, ritorna il suo indirizzo Hash, altrimento 0)*/
unsigned long int check(char* name) {
	unsigned long int ascii_value;
	unsigned long int hash, hash2;
	unsigned long int num;
	char compare[strlen(name)];
	num = 0;
	ascii_value = (name_to_ascii_hashmod(name));
	hash = (hashfunction(ascii_value));
	hash2 = doublehashing(ascii_value);
	while(hashtable[hash] != NULL){
	strcpy(compare, hashtable[hash]->name);
		if(strcmp(compare,name)== 0){
			return hash;
		}
		else{
			hash = ((hash2 + num) % HASH_SIZE);
			num++;
		}	
	}
	return 0;
}	



/*Funzione di inserimento entità nell'Hashtable*/
void insert(char* name, unsigned long int genhash) {
	struct entity* ent = (struct entity*)malloc(sizeof(struct entity));
	char* newname = (char*)malloc(sizeof(char)*strlen(name));	
	node** relnew = (node**)calloc(MAXREL,sizeof(node*));

	relnew[0] = NULL;
	strcpy(newname, name);
	ent->name = newname;
	ent->rel = relnew;

	hashtable[genhash] = ent;
}



/*Verifica se una relazione è presente, se presente ritorna 1, altrimenti l'aggiunge*/
int findrelazione(char* src, char* dest, char* relaz, unsigned long int genhash){
	int j = 0;
	int w = 0;
	int m = 0;
	struct node* nodo = (struct node*)malloc(sizeof(struct node));
	char* newnome = (char*)malloc(sizeof(char)*NAME_SIZE);
	node* pointer;
	node* current;
	strcpy(newnome, dest);

	
	while(relazioni[j] != NULL){      /*Verifichiamo se esiste il tipo di relazione*/
		
		if(strcmp(relazioni[j]->tipo,relaz) == 0){              //Tipo di relazione presente (CASO 1)
			pointer = relazioni[j]->tail;                       
			current = pointer;
			while(pointer != NULL){                             /*Verifichiamo se il destinatario ha un blocco nella lista*/
				
				if(strcmp(pointer->nome, dest) == 0) {                     //Tipo di relazione presente e destinatario presente nella lista (CASO 2)
					m = 0;                                                 //verifico se l'entità ha già una relazione col destinatario
						while(hashtable[genhash]->rel[m] != NULL){            /*Verifichiamo se l'entità ha già una relazione*/

                 
						    if(hashtable[genhash]->rel[m] == pointer){            //L'entità sorgente ha già effettuato questa relazione in passato  (CASO 3)
						    	free(nodo);      //libero il blocco non usato
						    	free(newnome);   //libero il blocco non usato
								return 1;        //RELAZIONE PRESENTE, RITORNA 1!
							}
							else {
								m++;
							}
						}

																					//l'entità NON aveva tale rapporto, lo aggiungo ed incremento il contatore (CASO NOT 3)
					hashtable[genhash]->rel[m] = pointer;               //aggiungo il puntatore alla relazione
					hashtable[genhash]->rel[m+1] = NULL;
					w = pointer->n;                                     //incremento il counter
					w = w + 1;  
					pointer->n = w;
					return 0;                                           //RELAZIONE AGGIUNTA, RITORNA 0
				}
				else {             
					current = pointer;                         
					pointer = pointer->next;
			}
		}		

		                                                           //Tipo di relazione presente ma destinatario ASSENTE nella lista (CASO NOT 2)
		nodo->nome = newnome;                                      //Creo un nuovo nodo da inserire nella lista, inserisco il nome del desinatario
		nodo -> n = 1;											   //Impongo il coutner ad 1, e inserisco il puntatore nell'array dell'entità
		nodo->next = NULL;
		if(pointer == relazioni[j]->tail){
			pointer = relazioni[j]->tail = nodo;
		}
		else{
		current->next = nodo;
		}

		m = 0;
		while(hashtable[genhash]->rel[m] != NULL){
			m++;
		}
		hashtable[genhash]->rel[m] = nodo;
		hashtable[genhash]->rel[m+1] = NULL;
		return 0;                                              //Relazione aggiunta, nuovo nodo creato, RITORNO 0.
			
		}
		else {
			j++;
		}

	}																 //relazione asente nella lista (CASO NOT 1)
	insertrel(relaz, j);                                      		//chiedo alla funzione insertrel di aggiungere la tipologia di relazione

	nodo->nome = newnome;                                           //eseguo i passaggi di caso not 2
	nodo->next = NULL;
	relazioni[j]->tail = nodo;
	m = 0;

	while(hashtable[genhash]->rel[m] != NULL){
		m++;
	}
	hashtable[genhash]->rel[m] = nodo;
	hashtable[genhash]->rel[m+1] = NULL;
	nodo->n = 1;
	return 0;		                                                //Tipo di relazione aggiunto, relazione aggiunta, array dell'entità aggiornato, ritorno 0
}



/*Sorting per i TIPI di relazioni (E' un banale bubble sort per adesso ver 1.0)*/
void quicksort(){
	int i = 0;
	struct relazione* swap;
	while(relazioni[i+1] != NULL){
    	if(strcmp(relazioni[i+1]->tipo, relazioni[i]->tipo) < 0){
     		swap = relazioni[i];
     		relazioni[i] = relazioni[i+1];
     		relazioni[i+1] = swap;
    		i++;
     	}
     	else{i++;}
    }
}

/*quicksort 2.0 */
int partition(char nomimax[MAX_PRINT][NAME_SIZE], int low, int high) {
	char pivot[NAME_SIZE];
	char swap[NAME_SIZE];
	strcpy(pivot,nomimax[high]);
	int i = (low - 1);

	for (int j = low; j < high ; j++){
		if (strcmp(nomimax[j],pivot) <= 0){
		i++;
		strcpy(swap, nomimax[j]);
		strcpy(nomimax[j],nomimax[i]);
		strcpy(nomimax[i], swap);
		}
	}
	strcpy(swap, nomimax[high]);
	strcpy(nomimax[high],nomimax[i+1]);
	strcpy(nomimax[i+1], swap);
	return(i+1);
}

void quicksortrel2(char nomimax[MAX_PRINT][NAME_SIZE], int low, int high){
	if (low<high)
	{
	int pi = partition (nomimax,low,high);

	quicksortrel2(nomimax,low,pi-1);
	quicksortrel2(nomimax,pi+1, high);
	}
}


 
/*Sorting per le relazioni (era un banale bubble sort prima, versione 1.0)*/
void quicksortrel(char nomimax[MAX_PRINT][NAME_SIZE], int lon){
	int p = 0;
	char swap[NAME_SIZE];
	char swap2[NAME_SIZE];
	for(int i = 0; i<lon; i++){
		p = 0;
		while(strcmp(nomimax[p+1], "#") != 0){
			strcpy(swap, nomimax[p]);
     		strcpy(swap2, nomimax[p+1]);
    			if(strcmp(swap2, swap) < 0){
     				strcpy(nomimax[p], swap2);
     				strcpy(nomimax[p+1], swap);
    				p++;
     			}
     			else{
     				p++;
     			}
   		}
	}
}



/*Trova per ogni relazione, le entità con il counter più alto, e le stampa*/
void maxrel(){
   struct node* pointer;
   int i = 0;
   int p,z,u;
   int lon = 0;
   u = 0;
   int y = 0;
   int k = 0;
   char ff = '"';
   int max = 0;
   int none = 0;
   char nomimax[MAX_PRINT][NAME_SIZE];

    	while(relazioni[i] != NULL){
      	
        	max = 0;
        	u = 0;
        	z = 0;
        	k = 0;
        	lon = 0;
        	pointer = relazioni[i]->tail;
        	if(relazioni[i]->tail != NULL){
     		if(y != 0){printf(" ");}
     		}

     			while(pointer != NULL) {


     				if(max < pointer->n){
     					z = 0;
     					none++;
     					max = pointer->n;
     					strcpy(nomimax[z], pointer->nome);
      					pointer = pointer->next;
     					z++;
     					lon = 1;
     					strcpy(nomimax[z], "#");
     				}
     				else if (pointer->n == max){
     					strcpy(nomimax[z], pointer->nome);
     					z++;
     					lon++;
     					pointer = pointer->next;
     					strcpy(nomimax[z], "#");

     				}
     				else{
     					pointer = pointer->next;
     				}
     			
     			}

     			quicksortrel2(nomimax,0,lon-1);
     			p = 0;
     			if(u == 0 && max != 0){
     				u++;
     					printf("%c%s%c %c%s%c",ff,relazioni[i]->tipo,ff,ff, nomimax[p],ff);
     					p++;
     				
     			}
     			if(k == 0 && max != 0){
     				y++;
     				k++;
     				while(strcmp(nomimax[p], "#") != 0){
     				printf(" %c%s%c",ff,nomimax[p],ff);
     				p++;
     				}
     			}
     			if(max != 0)
     			printf(" %i;",max);

     	 i++;
     	}

          			if(none == 0) {
     				printf("none");
     				return;
     			}
    return;
 }	



/*Data uan riga, estre una parola tra virgolette*/
void extract_word(char* line, char* name, unsigned int* pointer){
	unsigned long int j = 0;
	while(line[*pointer] != '"'){                
		name[j] = line[*pointer];
		j++;
		++*pointer;
	}
	name[j] = '\0'; 
	return;
}



/*Cerca e distrugge i nodi della lista con il contatore a 0*/
void cleaner(){
	int j = 0;
	node* pointer;
	node* current = NULL;
	while(relazioni[j] != NULL){     
		pointer = relazioni[j]->tail;          	
		current = pointer;		
			while(pointer != NULL){
				if(pointer->n == 0) { 		
					if(current != pointer){
						current->next = pointer->next;
						free(pointer);
						break;
					}
					else{
						if(pointer->next == NULL){
							relazioni[j]->tail = NULL;
							free(pointer);
							break;
						}
						else{
							relazioni[j]->tail = pointer->next;
							free(pointer);
							break;
					}
				}
						
			}
			else{
				current = pointer;
				pointer = pointer->next;
			}
		}
	j++;
	}
	return;
}



/*Dopo una DELENT, rimuove per ogni relazione il blocco relzione di quella entità*/
void cleanerent(char* nome){
	int j = 0;
	int u = 0;
	node* pointer;
	node* current = NULL;
	while(relazioni[j] != NULL){     
		pointer = relazioni[j]->tail;          	
		current = pointer;		
			while(pointer != NULL){
				if(strcmp(pointer->nome, nome)==0) { 
				pointer->n = 0;		
					if(current != pointer){
						current->next = pointer->next;
						pointer->nome = NULL;
						break;
					}
					else{
						if(pointer->next == NULL){
							relazioni[j]->tail = NULL;
							pointer->nome = NULL;
							break;
						}
						else{
							relazioni[j]->tail = pointer->next;
							pointer->nome = NULL;
							break;
					}
				}
				trash[u] = pointer;
				u++;		
			}
			else{
				current = pointer;
				pointer = pointer->next;
			}
		}
	j++;
	}
	return;
}



/*Pulisce l'array delle relazioni delle entità dai puntatori a NULL*/
void cleanhashrel(unsigned long int genhash){
	int m = 0;
	int w = 0;
	while(hashtable[genhash]->rel[w] != NULL){
	w++;
	}
	if(w == 0){
		return;
	}
	w--;

	if(w != 0 && hashtable[genhash]->rel[w]->nome == NULL){
		while(hashtable[genhash]->rel[w]->nome == NULL){
			if(w==0){
				break;
			}
			hashtable[genhash]->rel[w] = NULL;
			w--;
		}
	}

	if(w != 0 && w>m){
		while(w>m){

			if(hashtable[genhash]->rel[m]->nome == NULL){
				hashtable[genhash]->rel[m] = hashtable[genhash]->rel[w];
				hashtable[genhash]->rel[w] = NULL;
				m++;
				w--;
				while(hashtable[genhash]->rel[w]->nome == NULL){
					hashtable[genhash]->rel[w] = NULL;
					w--;
				}
			}
			else{
				m++;
			}
		}
	}

	if(hashtable[genhash]->rel[0]->nome == NULL){
		hashtable[genhash]->rel[0] = NULL;
	}

	return;
}



/*Rimozione Entità*/
void delent(unsigned long int genhash, char* name){
	int m = 0;
	int k;
	int c = 0;
	int u = 0;
	unsigned long p = 0;
	node* nodo;
	while(hashtable[genhash]->rel[m] != NULL){
		nodo = hashtable[genhash]->rel[m];
		k = nodo->n;
		k--;
		if(k == 0){c++;}
		nodo->n = k;
		hashtable[genhash]->rel[m] = NULL;
		m++;
		}

	cleanerent(name);
	free(hashtable[genhash]->name);
	
	for(int i = 0; i< MAXREL; i++){	
	free(hashtable[genhash]->rel[i]);
	}
	free(hashtable[genhash]->rel);		

	(hashtable[genhash]) = empty;
	if(c != 0){
		cleaner();
	}
	for(p= 0; p< HASH_SIZE; p++){
		if(hashtable[p] != NULL){
		if(strcmp(hashtable[p]->name,"###") != 0)
		cleanhashrel(p);
		}
	}

	while(trash[u] != NULL){
		trash[u] = NULL;
		u++;
	}

	return;
}



/*Rimozione relazione*/
void delrel(unsigned long int genhash, char* dest, char* relaz){
	int j = 0;
	int k = 0;
	int w = 0;
	int m = 0;
	node* pointer;
	node* current = NULL;
	while(relazioni[j] != NULL){      /*Verifichiamo se il tipo di relazione è presente nella lista*/
	

		if(strcmp(relazioni[j]->tipo,relaz) == 0){             
			pointer = relazioni[j]->tail;
			current = pointer;
			while(pointer != NULL){

				if(strcmp(pointer->nome, dest) == 0) {         //Cerchiamo se il destinatario possiede già un nodo con quella relazione
					m = 0;
						while(hashtable[genhash]->rel[m] != NULL){                //Verifichiamo se l'entità sorgente aveva già quel tipo di relazione col destinatario
						    if(hashtable[genhash]->rel[m] == pointer){              //L'entita ha quella relazione, andiamo a rimuoverla

								k = pointer->n;
								k--;
								pointer->n = k;
								if(k == 0 && current != pointer){                //Se il contatore è pari a 0, e non sono in testa alla lsita
									current->next = pointer->next;
									free(pointer);

								}
							    else if(k == 0 && current == pointer){          //Se il contatore è pari a 0, sono in testa alla lista E SONO L'UNICO NODO
							    	if(pointer->next == NULL){
							    		relazioni[j]->tail = NULL;
									}
									else{                                         //Se il contatore è pari a 0, sono in testa alla lista e NON sono l'unico nodo
										relazioni[j]->tail = pointer->next;
										free(pointer);
									}
								}
								while(hashtable[genhash]->rel[w] != NULL){                    //Cerco l'ultima relazione
									w++;
								}
								if(w != 0){
									w--;
									hashtable[genhash]->rel[m] = hashtable[genhash]->rel[w];           //La sostituisco con la prima
									hashtable[genhash]->rel[w] = NULL;	                                //cancello l'ultima posizione
									return;  
									
								}
								else{
									
									hashtable[genhash]->rel[0] = NULL;                                //Se sono l'unica relazione, cancello e basta
									return;

								}
							}
							m++;
						}
						return;
				}
				else {current = pointer; pointer = pointer->next;}
				
			}
			return;
		}
		j++;
	}
	return;
}


/********************************** DEBUGGING **********************************/

/*Funzione per stampare l'hash table*/
void print_lista() {
   struct node* pointer;
   int i = 0;
   printf("\n\n------------------LISTA-----------------");
   for(i = 0; i<MAXREL_TYPE; i++) {
      if(relazioni[i] != NULL){
         printf("\n [%s]",relazioni[i]->tipo);
         pointer = relazioni[i]->tail;
     		for(int j = 0; j<MAXREL; j++) {
     			if(pointer != NULL) {
     				printf("->[%s|%li]",pointer->nome,pointer->n);
     				pointer = pointer->next;
     			}
     		}
     	}
   }
	
   printf("\n----------------------------------------\n");
}


/*Funzione per stampare l'hash table*/
void print_hash_table() {
   int i = 0;
   printf("\n\n---------------HASH TABLE---------------");
   for(i = 0; i<HASH_SIZE; i++) {
	
      if(hashtable[i] != NULL)
         printf("\n[%i] -> (%s)",i,hashtable[i]->name);
   }
	
   printf("\n----------------------------------------\n");
}


void print_hashrel(unsigned long int genhash){
	int m = 0;
	if(hashtable[genhash] != NULL){
		if(hashtable[genhash]->rel[0] == NULL){
			printf("empty\n");
			return;
		}

		else{
			printf("[%s]",hashtable[genhash]->name);
			while(hashtable[genhash]->rel[m] != NULL){
				printf(" -> %s",hashtable[genhash]->rel[m]->nome);
				m++;
			}
			printf("\n");
			return;
		}	
	return;
	}
}

void print_all_hashrel(){
	printf("\n--------------ALL HASHREL---------------\n");
	for(int i=0; i< HASH_SIZE; i++){
			print_hashrel(i);
		}
	return;
}




/********************************** MAIN **********************************/



int main () {


	VUOTO = (node**)malloc(sizeof(node));           //Array di relazioni di un entità cancellata da una DELENT
	vuot[1] = (node*)malloc(sizeof(node));
	vuot[0] = NULL;
	VUOTO = vuot;


	empty = (struct entity*)malloc(sizeof(struct entity));     //Entità cancellata da una DELENT
	char* newname = (char*)malloc(sizeof(char)*4);
	strcpy(newname, "###");
	empty->name = newname;
	empty->rel = vuot;


	char line[LINE_SIZE];
	char name[NAME_SIZE];
	char name_dest[NAME_SIZE];
	char rel[NAME_SIZE];
	char select[6];
	unsigned int* pointer;
	unsigned int i;
	unsigned long int genhash, desthash;
	unsigned long int u = 0;


	

	while(!feof(stdin)){
		readline(line);                                
		if(line[0] != 'e'){
			strncpy(select, line, 6);
			select[6] = '\0';
		}
		else {
			strncpy(select, "end", 3);
			select[3] = '\0';
		}
		
		i = 8;
		pointer = &i;
		
		if(strcmp(select,"addent") == 0){
			//printf("\n--------------ADDENT STATE--------------\n");
			extract_word(line, name, pointer);

			genhash = search(name);
			if(genhash != 1){
				insert(name,genhash);
			}
		 }


		else if(strcmp(select,"addrel") == 0){
		 	//printf("\n--------------ADDREL STATE--------------\n");

		    extract_word(line, name, pointer);
		    *pointer = *pointer + 3;
		    extract_word(line, name_dest, pointer);
		    *pointer = *pointer + 3;
		    extract_word(line, rel, pointer);
		    genhash = check(name);
		    desthash = check(name_dest);

		    if((genhash != 0) && (desthash != 0)){
		    	findrelazione(name,name_dest,rel,genhash);
			}
		}


		else if(strcmp(select,"delent") == 0){
			//printf("\n-------------DELENT STATE--------------\n");
			extract_word(line,name,pointer);
			genhash = check(name);
			if(genhash != 0){
				delent(genhash,name);
			}
		}


		else if(strcmp(select,"delrel") == 0){
			//printf("\n-------------DELREL STATE--------------\n");
		    extract_word(line, name, pointer);
		    *pointer = *pointer + 3;
		    extract_word(line, name_dest, pointer);
		    *pointer = *pointer + 3;
		    extract_word(line, rel, pointer);
		    genhash = check(name);
		    desthash = check(name_dest);

		    if((genhash != 0) && (desthash != 0)){
		    	delrel(genhash,name_dest,rel);
		    	cleanhashrel(genhash);
			}
		}

		else if(strcmp(select,"report") == 0){
			if(u == 0)
				{u++;}
			else{
				printf("\n");
			}
			//printf("\n--------------REPORT STATE--------------\n");
		    quicksort();
			maxrel();
		}
		
		else{ 
			printf("\n");
			return 0;
		}



    }
    	return 0;

}
