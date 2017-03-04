/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h> 
/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 
/* pour hostent, servent */
#include <string.h> 
/* pour bcopy, ... */ 
#include <pthread.h> // for threading, link with lpthread

#define TAILLE_MAX_NOM 256



typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;

typedef struct socketList{
    int* listSocketsDescriptor;
    int size;
    int currentSize;
}socketList;

int NB_CLIENTS = 0;
socketList* sockList;
char message[254];


/*------------------------------------------------------*/
socketList* initSocketList(int s){
    socketList* structList = (socketList*) malloc(sizeof(socketList));
    structList->size = s;
    structList->currentSize = 0;

    structList->listSocketsDescriptor = (int*) malloc(sizeof(int)*structList->size);

    return structList;
}

// Fonction de recopie d'une liste de socket dans une autre 2 fois plus grande
/*------------------------------------------------------*/
socketList* recopySocketList(socketList* source){

    socketList* copy = initSocketList(source->size*2);
    unsigned int i;

    if(source != NULL && copy != NULL && source->size < copy->size){
        for(i = 0 ; i < source->size ; ++i){
            copy->listSocketsDescriptor[i] = source->listSocketsDescriptor[i];
            copy->currentSize++;
        }
        free(source);
        return copy;
    }

    return NULL;
}

//Fonction d'ajout d'un coket dans une liste
/*-------------------------------------------------------*/
socketList* addSocketToList(socketList* list, int socket_descriptor){

    socketList* listCopy;

    // Si la liste est pleine, double l'espace mémoire de la liste avec recopie des éléments déjà stockés
    if(list->currentSize >= list->size){
        listCopy = recopySocketList(list);
        listCopy->listSocketsDescriptor[listCopy->currentSize];
        listCopy->currentSize++;

        return listCopy;
    }else{
        list->listSocketsDescriptor[list->currentSize];
        list->currentSize++;

        return list;
    }
}

/*------------------------------------------------------*/
void renvoi (int sock) {
    char buffer[256];
    int longueur;
    if ((longueur = read(sock, buffer, sizeof(buffer))) <= 0) 
        return;

    printf("message lu : %s \n", buffer);
    buffer[0] = 'R';
    buffer[1] = 'E';
    buffer[longueur] = '#';
    buffer[longueur+1] ='\0';
    printf("message apres traitement : %s \n", buffer);  
    printf("renvoi du message traite.\n");
    /* mise en attente du programme pour simuler un delai de transmission */
    sleep(3);
    write(sock,buffer,strlen(buffer)+1);    
    printf("message envoye. \n");       
    return;
}

/*-----------------------------------------------------*/
void sendMessage(char message[256], int sock){

    char cheval[256];
    int longueur;
    unsigned int i = 0;

    longueur = sizeof(message);

    for (i; i < longueur;i++){
        cheval[i] = message[i];
    }

    write(sock,cheval,strlen(cheval)+1);    
    printf("message envoyé! \n");       
    return;
}


/*-----------------------------------------------------*/
void startGame() {



}




/*-----------------------------------------------------*/
void enterGame() {




}


/*------------------------------------------------------*/
void *traitementClient (void *socket_descriptor) {
     
    // printf("socket_dexcriptor : %d \n", (int) *socket_descriptor);
    
    addSocketToList(sockList, (int) socket_descriptor);

    /* créer procédure pour lancer le jeu pour chaque client, avec une boucle qui tourne tant que le jeu est pas fini 
       changer des variables globales pour lancer le jeu
    */
    //enterGame();

    char *message;
    message = "coucou";
    
    //renvoi ( (int) socket_descriptor);

    sendMessage( message, (int) socket_descriptor); 
    close( (int) socket_descriptor);


}




/*-------------------- MAIN ----------------------------------*/
main(int argc, char **argv) {
    int socket_descriptor, 
/* descripteur de socket */
    nouv_socket_descriptor, 
/* [nouveau] descripteur de socket */
    longueur_adresse_courante; 
/* longueur d'adresse courante d'un client */
    sockaddr_in adresse_locale, 
/* structure d'adresse locale*/
    adresse_client_courant; 
/* adresse client courant */
    hostent* ptr_hote; 
/* les infos recuperees sur la machine hote */
    servent* ptr_service; 
/* les infos recuperees sur le service de la machine */
    char machine[TAILLE_MAX_NOM+1]; 
/* nom de la machine locale */
    gethostname(machine,TAILLE_MAX_NOM);
/* recuperation du nom de la machine */
    /* recuperation de la structure d'adresse en utilisant le nom */
    if ((ptr_hote = gethostbyname(machine)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son nom.");
        exit(1);
    }

    // Déclaration de deux threads
    pthread_t monThread1, monThread2;

    /* initialisation de la structure adresse_locale avec les infos recuperees */
    /* copie de ptr_hote vers adresse_locale */
    bcopy((char*)ptr_hote->h_addr, (char*)&adresse_locale.sin_addr, ptr_hote->h_length);
    adresse_locale.sin_family = ptr_hote->h_addrtype; 
/* ou AF_INET */
    adresse_locale.sin_addr.s_addr = INADDR_ANY; 
/* ou AF_INET */
    /* 2 facons de definir le service que l'on va utiliser a distance */
    /* (commenter l'une ou l'autre des solutions) */
    /*-----------------------------------------------------------*/
    /* SOLUTION 1 : utiliser un service existant, par ex. "irc" */
    /*
    if ((ptr_service = getservbyname("irc","tcp")) == NULL) {
perror("erreur : impossible de recuperer le numero de port du service desire.");
exit(1);
    }
    adresse_locale.sin_port = htons(ptr_service->s_port);
    /*-----------------------------------------------------------*/
    /* SOLUTION 2 : utiliser un nouveau numero de port */
    
    adresse_locale.sin_port = htons(5000);
    /*
    /*-----------------------------------------------------------*/
    printf("numero de port pour la connexion au serveur : %d \n", 
   ntohs(adresse_locale.sin_port) /*ntohs(ptr_service->s_port)*/);
    /* creation de la socket */
        if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("erreur : impossible de creer la socket de connexion avec le client.");
            exit(1);
        }
    /* association du socket socket_descriptor à la structure d'adresse adresse_locale */
        if ((bind(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
            perror("erreur : impossible de lier la socket a l'adresse de connexion.");
            exit(1);
        }
    
    /* initialisation de la file d'ecoute */
        listen(socket_descriptor,5);
    /* attente des connexions et traitement des donnees recues */
  
    /*----------------------------------------------------*/
    // On initialise la SocketList
    
    sockList = initSocketList(4);
   
    /*int newSocketDescriptor, newSocketDescriptor2, newSocketDescriptor3;
    newSocketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
    newSocketDescriptor2 = socket(AF_INET, SOCK_STREAM, 0);
    newSocketDescriptor3 = socket(AF_INET, SOCK_STREAM, 0);
    sockList = addSocketToList(sockList, newSocketDescriptor);
    
    sockList = addSocketToList(sockList, newSocketDescriptor2);
     sockList = addSocketToList(sockList, newSocketDescriptor3);
    
    printf("socket descriptor : %d \n",sockList->listSocketsDescriptor[0]);
    printf("socket descriptor : %d \n",sockList->listSocketsDescriptor[1]);
    printf("socket descriptor : %d \n",sockList->listSocketsDescriptor[2]);*/
    
        
    
    

    for(;;) {
        longueur_adresse_courante = sizeof(adresse_client_courant);
      

        if (NB_CLIENTS == 4) {
            printf("Il y a déjà une partie en cours, veuillez essayer un peu plus tard. \n");
            exit(1);
        }
     
        /* adresse_client_courant sera renseignée par accept via les infos du connect */
           if ((nouv_socket_descriptor = accept(socket_descriptor, 
               (sockaddr*)(&adresse_client_courant),
               &longueur_adresse_courante))< 0) {
               perror("erreur : impossible d'accepter la connexion avec le client.");
               exit(1);
           }  

          //création du thread pour le nouveau client
            if( pthread_create( &monThread1, NULL , traitementClient , (void *) nouv_socket_descriptor)){
                perror("could not create thread");
                return 1;
            }

            NB_CLIENTS += 1;
            printf("NB_CLIENTS = %d \n", NB_CLIENTS);
            //TODO : c'est vgraiment chelou
            printf("Il y %d sur 4 joueurs de connectés.", NB_CLIENTS);

            if (NB_CLIENTS == 4) {

            printf("start game");
  
             /*lancer la partie quand tout les clients sont arrivés.*/
                startGame();
            }
        
    }    
}
