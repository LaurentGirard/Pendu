/*----------------------------------------------
Serveur à lancer avant le client
------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <linux/types.h> 
/* pour les sockets */
#include <sys/socket.h>
#include <netdb.h> 
/* pour hostent, servent */
#include <string.h> 
/* pour bcopy, ... */ 
#include <pthread.h> // for threading, link with lpthread
#include <time.h> //For random int

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
int joueurCourant = 0;
int endgame = 1;
int nbErrors = 0;
socketList* sockList;
char *mot;
char *motCourant;

char **dictionnaire;


/*-----------------------------------------------------*/
char* currentInterface(int nbErrors){

    char* hangman = (char*) malloc (sizeof(char)*256);

    switch(nbErrors){

        case 1:
            hangman = "0============\n";
        break;

        case 2:
            hangman = "0   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n============";        
        break;

        case 3:
            hangman = "0    ==========Y===\n   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n   ||\n============";
        break;

        case 4:
            hangman = "0    ==========Y===\n   ||  /\n   || /\n   ||/\n   ||\n   ||\n   ||\n  /||\n //||\n============";
        break;

        case 5:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/\n   ||\n   ||\n   ||\n  /||\n //||\n============";
        break;

        case 6:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/        O\n   ||\n   ||\n   ||\n  /||\n //||\n============";
        break;

        case 7:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/        O\n   ||         |\n   ||\n   ||\n  /||\n //||\n============";
        break;

        case 8:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/        O\n   ||         |\\\n   ||\n   ||\n  /||\n //||\n============";
        break;

        case 9:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/        O\n   ||        /|\\\n   ||        /\n   ||\n  /||\n //||\n============";
        break;
    
        case 10:
            hangman = "0    ==========Y===\n   ||  /      |\n   || /       |\n   ||/        O\n   ||        /|\\\n   ||        /|\n   ||\n  /||\n //||\n============";
        break;
    }

    return hangman;
}


/*-----------------------------------------------------*/
void initDico(){
    
    dictionnaire = (char**) malloc (sizeof(char*)*20);
    unsigned int i;
    for(i = 0; i < 20 ; ++i){
        dictionnaire[i] = (char*) malloc(sizeof(char)*256);
    }

    dictionnaire[0] = "souris";
    dictionnaire[1] = "chat";
    dictionnaire[2] = "brouette";
    dictionnaire[3] = "rideau";
    dictionnaire[4] = "caillou";
    dictionnaire[5] = "tractopelle";
    dictionnaire[6] = "cheval";
    dictionnaire[7] = "ordinateur";
    dictionnaire[8] = "pont";
    dictionnaire[9] = "camion";
    dictionnaire[10] = "moniteur";
    dictionnaire[11] = "rhododendron";
    dictionnaire[12] = "tulipe";
    dictionnaire[13] = "chaise";
    dictionnaire[14] = "conscience";
    dictionnaire[15] = "anatomie";
    dictionnaire[16] = "saxophone";
    dictionnaire[17] = "bonjour";
    dictionnaire[18] = "enfer";
    dictionnaire[19] = "conjugaison";
}

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


//Fonction d'ajout d'un socket dans une liste
/*-------------------------------------------------------*/
socketList* addSocketToList(socketList* list, int socket_descriptor){

    socketList* listCopy;

    // Si la liste est pleine, double l'espace mémoire de la liste avec recopie des éléments déjà stockés
    if(list->currentSize >= list->size){
        listCopy = recopySocketList(list);
        listCopy->listSocketsDescriptor[listCopy->currentSize] = socket_descriptor;
        listCopy->currentSize++;

        return listCopy;
    }else{
        list->listSocketsDescriptor[list->currentSize] = socket_descriptor;
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

/*------------------------------------------------------*/
/*fonction qui permet de recevoir un message d'un socket_descriptor particulier*/
char* receiveMessage(int socket) {
 int sockfd, ret;
 char buffer[256]; 
 char *copy = (char*) malloc (sizeof(char)*256);
 sockfd = (int) socket;
 memset(buffer, 0, 256);  
 

  ret = recvfrom(sockfd, buffer, 256, 0, NULL, NULL);  
  if (ret < 0) {  
   printf("Error receiving data!\n");    
  } else {
   printf("client: ");
   fputs(buffer, stdout);
   printf("\n");
   copy = strcpy(copy, buffer);
   return copy;
   }
 
}

/*------------------------------------------------------*/
char* askClient (int sock) {

    char *message = (char*) malloc (sizeof(char)*256);
    char *rep = (char*) malloc (sizeof(char)*256);
    message = "1\nC'est à vous de jouer, que proposez vous ?. \n";
    sendMessage(message, sock);

    rep  = receiveMessage(sock);
    return rep;
}





/*-----------------------------------------------------*/
void sendMessage(char *message, int sock){

    char *copy = (char*) malloc (sizeof(char)*256);;
    int longueur;
    unsigned int i = 0;

    longueur = strlen(message);
    
    for (i; i < longueur;i++){
        copy[i] = message[i];
    }

    write(sock,copy,strlen(copy)+1);    
    printf("message envoyé! \n");       
    return;
}

/*-----------------------------------------------------*/
void sendInterfaceAll(){

    char *messageError = currentInterface(nbErrors);

    sendMessage(messageError,sockList->listSocketsDescriptor[0] );
    sendMessage(messageError,sockList->listSocketsDescriptor[1] );
    sendMessage(messageError,sockList->listSocketsDescriptor[2] );
    sendMessage(messageError,sockList->listSocketsDescriptor[3] );

    return;
}


/*-----------------------------------------------------*/
void sendMessageAll(char *message){

    char *copy = (char*) malloc (sizeof(char)*256);
    int longueur;
    unsigned int i = 0;

    longueur = strlen(message);
    
    for (i; i < longueur;i++){
        copy[i] = message[i];
    }



    sendMessage(copy,sockList->listSocketsDescriptor[0] );
    sendMessage(copy,sockList->listSocketsDescriptor[1] );
    sendMessage(copy,sockList->listSocketsDescriptor[2] );
    sendMessage(copy,sockList->listSocketsDescriptor[3] );

    printf("message envoyé à tout le monde ! \n");       
    return;
}

/*------------------------------------------------------*/
void nextPlayer(){
    
   switch(joueurCourant) {

    case 0 :
      joueurCourant++;
      break; /* optional */

    case 1 :
      joueurCourant++;
      break; /* optional */

    case 2 :
      joueurCourant++;
      break; /* optional */

    case 3 :
      joueurCourant = 0;
      break; /* optional *
  
   /* you can have any number of case statements */
    default : /* Optional */
        printf("Error in nextPlayer");
    }

}


/*-------------------------------------------------------*/
void endGame() {

    close (sockList->listSocketsDescriptor[0]);
    close (sockList->listSocketsDescriptor[1]);
    close (sockList->listSocketsDescriptor[2]);
    close (sockList->listSocketsDescriptor[3]);
    NB_CLIENTS = 0;
    joueurCourant = 0;
    endgame = 1;
    nbErrors = 0;
    sockList->currentSize = 0;


}

/*--------------------------------------------------------*/
// renvoie 0 si le motCourant est inchangé sinon, 1
unsigned int replaceLetter (char *rep){

    int longueur = strlen(mot);
    int retour = 0;
    unsigned int i = 0;


    for (i;i<longueur;i++) {

        if (mot[i] == rep[0]) {
            motCourant[i] = rep[0];
            retour = 1;
        }
    }

    return retour ;
}

/*----------------------------------------------------------*/
// 0 quand les deux chaines sont différentes, sinon 1.
int compareMots(char* chaine1, char* chaine2) {

    int longueur1 = strlen(chaine1)-1;
    int longueur2 = strlen(chaine2);
    unsigned int i;

    if(longueur1 != longueur2) {

        return 0;
    } else {
        for (i = 0; i < longueur1 ; ++i){
       
            if(chaine1[i] != chaine2[i]){
                return 0;
            }
        }
    }

    return 1;
}


/*---------------------------------------------------------*/
char* initMotCourant(){

    int longueur = strlen(mot);
    unsigned int i = 0;
    char*copy = (char*) malloc (sizeof(char)*256);
    for (i; i < longueur ; i++ ){

       copy[i] = '*';
    }
    
    printf("copy = %s \n", copy);
    return copy;

}


/*-----------------------------------------------------*/
int tourDeJeu(int joueur){

    char* rep = askClient(sockList->listSocketsDescriptor[joueur]);
    char *message = (char*) malloc (sizeof(char)*256);
    char *message2 = (char*) malloc (sizeof(char)*256);
    int letterInWord;
    int longueurRep;
    longueurRep = strlen(rep)-1;


    //Si le client propose un mot
    if (longueurRep > 1) {

        // si le mot est le bon
        if(compareMots(rep, mot) == 1){

           message = "0Bravo vous avez trouvé le bon mot ! :) \n";
           sendMessage(message,sockList->listSocketsDescriptor[joueur]);
           sleep(2);

           message2 = "0Le joueur courant a trouvé le bon mot ! Partie terminée. Vous allez etre déconnecté. \n";
           
           sendMessageAll(message2); 

           return 0;


        // si le mot n'est pas le bon
        }else{
            message = "0\nLe joueur a proposé un mot mais ce n'est pas le bon. Mot proposé : ";

            char* name_with_extension;
            name_with_extension = malloc(strlen(message)+1+strlen(rep)); /* make space for the new string (should check the return value ...) */
            strcpy(name_with_extension, message); /* copy name into the new var */
            strcat(name_with_extension, rep); /* add the extension */

            sendMessageAll(name_with_extension);

            nbErrors++;
            sendInterfaceAll();

            nextPlayer();
            return 1;

        }

    // si le client propose une lettre
    } else {

   
        letterInWord = replaceLetter(rep);
        printf("motCourant = %s \n", motCourant);

        // la lettre est bien dans le mot
        if (letterInWord > 0){

            message = "0\nLe joueur a proposé une lettre et elle est dans le mot ! : ";

            char* name_with_extension;
            name_with_extension = malloc(strlen(message)+1+strlen(rep)); /* make space for the new string (should check the return value ...) */
            strcpy(name_with_extension, message); /* copy name into the new var */
            strcat(name_with_extension, rep); /* add the extension */

            sendMessageAll(name_with_extension);
            sleep(2);

            message2 = "0\nLe mot courant est : ";

            char* name_with_extension2;
            name_with_extension2 = malloc(strlen(message2)+1+strlen(motCourant)); /* make space for the new string (should check the return value ...) */
            strcpy(name_with_extension2, message2); /* copy name into the new var */
            strcat(name_with_extension2, motCourant); /* add the extension */

            sendMessageAll(name_with_extension2);
            sleep(2);

            nextPlayer();
            return 1;


        // la lettre n'est pas dans le mot
        } else {

            message = "0\nLe joueur a proposé une lettre mais elle n'est pas dans le mot. Lettre proposée : ";

            char* name_with_extension;
            name_with_extension = malloc(strlen(message)+1+strlen(rep)); /* make space for the new string (should check the return value ...) */
            strcpy(name_with_extension, message); /* copy name into the new var */
            strcat(name_with_extension, rep); /* add the extension */

            sendMessageAll(name_with_extension);
            sleep(2);

            message2 = "0\nLe mot courant est : ";

            char* name_with_extension2;

            name_with_extension2 = malloc(strlen(message2)+1+strlen(motCourant)); /* make space for the new string (should check the return value ...) */
            strcpy(name_with_extension2, message2); /* copy name into the new var */
            strcat(name_with_extension2, motCourant); /* add the extension */

            sendMessageAll(name_with_extension2);
            sleep(2);

            nbErrors++;

            sendInterfaceAll();

            nextPlayer();
            return 1;

        }

    }

}


/*-----------------------------------------------------*/
void *startGame() {

    joueurCourant = 0;
    mot = (char*) malloc (sizeof(char)*256);
    motCourant = (char*) malloc (sizeof(char)*256);
    char *message = (char*) malloc (sizeof(char)*256);
    char *message2 = (char*) malloc (sizeof(char)*256);
    char* name_with_extension;
    char *rep = (char*) malloc (sizeof(char)*256);
    char *errorMessage;

    int random = rand()%(19);

    mot = dictionnaire[random];
    printf("In startGame, Mot = %s \n", mot);
    motCourant = initMotCourant();
    printf("In startGame, MotCourant = %s \n", motCourant);
    message = "0Tout les joueurs sont arrivés, la partie va pouvoir commencer. \n";
    sendMessageAll(message);
    sleep(2);
    message2 = "0Le mot à trouver est : ";

    name_with_extension = malloc(strlen(message2)+1+strlen(motCourant)); /* make space for the new string (should check the return value ...) */
    strcpy(name_with_extension, message2); /* copy name into the new var */
    strcat(name_with_extension, motCourant); /* add the extension */

    printf("Message avec extension : %s \n", name_with_extension);

    sendMessageAll(name_with_extension);


    while (endgame > 0 && nbErrors < 12 ){

        endgame = tourDeJeu(joueurCourant);

    }

    if(nbErrors >= 12){
        errorMessage = (char*) malloc (sizeof(char)*256);
        errorMessage = "0Le jeu est terminé, vous avez perdu, il ne vous reste plus aucun essai";
        sendMessageAll(errorMessage);
    }
    
    endGame();

}



/*-----------------------------------------------------*/
/*fonction qui permet de recevoir un message d'un socket_descriptor particulier*/
void * receiveMessageThread(void * socket) {
 int sockfd, ret;
 char buffer[256]; 
 sockfd = (int) socket;
 memset(buffer, 0, 256);  
 

  ret = recvfrom(sockfd, buffer, 256, 0, NULL, NULL);  
  if (ret < 0) {  
   printf("Error receiving data!\n");    
  } else {
   printf("client: ");
   fputs(buffer, stdout);
   printf("\n");
   char *message;
   message = "0C'est génial :D";
   sendMessage(message, (int) socket);
   close ((int ) socket);
  }
 
}

/*-----------------------------------------------------*/



/*------------------------------------------------------*/
void *traitementClient (void *socket_descriptor) {
     
    // printf("socket_dexcriptor : %d \n", (int) *socket_descriptor);
    
    sockList = addSocketToList(sockList, (int) socket_descriptor);

    char *message = (char*) malloc (sizeof(char)*256);
   // char *message2 = (char*) malloc (sizeof(char)*256);
    message = "0Bienvenue dans la partie ! \n";
    sendMessage( message, (int) socket_descriptor); 

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

    initDico();

    for(;;) {

        longueur_adresse_courante = sizeof(adresse_client_courant);      

         /* adresse_client_courant sera renseignée par accept via les infos du connect */
           if ((nouv_socket_descriptor = accept(socket_descriptor, 
               (sockaddr*)(&adresse_client_courant),
               &longueur_adresse_courante))< 0) {
               perror("erreur : impossible d'accepter la connexion avec le client.");
               exit(1);
           }  

        /*Si la partie est pleine, on envoie un message au client*/
        if (NB_CLIENTS == 4) {
            char gameFull[256] = "Il y a déjà une partie en cours, veuillez essayer un peu plus tard. \n";
            sendMessage(gameFull, nouv_socket_descriptor);
            close(nouv_socket_descriptor);
        } else {

          //création du thread pour le nouveau client
            if( pthread_create( &monThread1, NULL , traitementClient , (void *) nouv_socket_descriptor)){
                perror("could not create thread");
                return 1;
            }

            NB_CLIENTS += 1;
            printf("NB_CLIENTS = %d \n", NB_CLIENTS);
            printf("Il y %d sur 4 joueurs de connectés.\n", NB_CLIENTS);

            if (NB_CLIENTS == 4) {

            printf("start game \n");
  
             /*lancer la partie quand tout les clients sont arrivés.*/
            //création du thread pour la nouvelle partie
              if( pthread_create( &monThread1, NULL , startGame, NULL)){
                   perror("could not create startgame thread");
                   return 1;
                }


            }


        }

    }    
    
}
