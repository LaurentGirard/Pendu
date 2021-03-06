/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> 
------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <linux/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
typedef struct sockaddr sockaddr;
typedef struct sockaddr_in sockaddr_in;
typedef struct hostent hostent;
typedef struct servent servent;


/*-------------------------------------------------------------------------*/
// Méthode retirant le premier caractère d'une chaine.
char *enlevePremierChar(char chaine[256]){

    char *copy = (char*) calloc (256,sizeof(char));
    char *copy2 = (char*) calloc (256,sizeof(char));
    int longueur = strlen(chaine);
    unsigned int i = 1;

    /* on enlève le premier char*/
    for (i; i < longueur ; i++ ) {
        copy2[i-1]=chaine[i];
    }

    return copy2;
}


int main(int argc, char **argv) {
    int socket_descriptor, 
/* descripteur de socket */
    longueur; 
/* longueur d'un buffer utilisé */
    sockaddr_in adresse_locale; 
/* adresse de socket local */
    hostent *ptr_host; 
/* info sur une machine hote */
    servent *ptr_service; 
/* info sur service */
    char buffer[256];
    char *prog; 
/* nom du programme */
    char *host; 
/* nom de la machine distante */
    char *mesg; 
/* message envoyé */
    if (argc != 2) {
        perror("usage : client <adresse-serveur>");
        exit(1);
    }

    prog = argv[0];
    host = argv[1];

    if ((ptr_host = gethostbyname(host)) == NULL) {
        perror("erreur : impossible de trouver le serveur a partir de son adresse.");
        exit(1);
    }
    /* copie caractere par caractere des infos de ptr_host vers adresse_locale */
    bcopy((char*)ptr_host->h_addr, (char*)&adresse_locale.sin_addr, ptr_host->h_length);
    adresse_locale.sin_family = AF_INET; /* ou ptr_host->h_addrtype; */
    /* 2 facons de definir le service que l'on va utiliser a distance (le même que sur le serveur) */
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
    /*-----------------------------------------------------------*/
    printf("numero de port pour la connexion au serveur : %d \n", ntohs(adresse_locale.sin_port));
    /* creation de la socket */
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("erreur : impossible de creer la socket de connexion avec le serveur.");
        exit(1);
    }
    /* tentative de connexion au serveur dont les infos sont dans adresse_locale */
    if ((connect(socket_descriptor, (sockaddr*)(&adresse_locale), sizeof(adresse_locale))) < 0) {
        perror("erreur : impossible de se connecter au serveur. BLEU");
        exit(1);
    }
    printf("connexion etablie avec le serveur. \n");

    char *copy;

    while ((longueur = recvfrom(socket_descriptor, buffer, 256, 0, NULL, NULL) > 0 )) {

        /* message simple à afficher */
        if ( buffer[0] == 48){

            copy = (char*) calloc (256,sizeof(char));
            copy = enlevePremierChar(buffer);            

            longueur = strlen(copy);

            write(1,copy,longueur);
            free(copy);
        } 

        /* le serveur nous pose une question */
        if ( buffer[0] == 49){

            char *copy;
            copy = enlevePremierChar(buffer);
            longueur = strlen(copy);
            write(1,copy,longueur);
            sleep(2);

            char strvar[256];
            fgets (strvar, 256, stdin);
            
            if ((write(socket_descriptor, strvar, sizeof(strvar))) < 0) {
                perror("erreur : impossible d'ecrire le message destine au serveur.");
                exit(1);
            }

        }        
    }

    printf("\nfin de la reception.\n");
    close(socket_descriptor);
    printf("connexion avec le serveur fermee, fin du programme.\n");
    exit(0);  
}
