/*C'est un serveur qui utilise les sockets TCP/IP pour établir une connexion avec un client
On définit une adresse IP d'écoute qui est utilisée pour la mise en place de la connexion et un port d'écoute pour que le serveur puisse écouter les demandes de connexion entrante
On utilise des variables pour stocker les infos de connexion
Puis il y a des instructions pour traiter les erreurs et fermer les sockets une fois la connexion terminée*/

#include  <stdio.h>
#include  <stdlib.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <string.h>
#include  <unistd.h>
#include  <stdbool.h>
#include "simpleSocketAPI.h"

#define SERVADDR "127.0.0.1"        // Définition de l'adresse IP d'écoute, qui est aussi l'adresse de bouclage donc le serveur ne sera accessible que à partir de l'ordinateur local
#define SERVPORT "0"                // Définition du port d'écoute, si 0 le serveur choisira un port d'écoute dynamiquement au lieu d'un port spécifique
#define LISTENLEN 1                 // Longueur de la file d'attente pour les connexions entrantes, le serveur peut gérer une seule connexion à la fois
#define MAXBUFFERLEN 1024           // Taille maximale de la mémoire tampon pour les échanges de données entre le client et le serveur
#define MAXHOSTLEN 64               // Taille maximale d'un nom d'hôte
#define MAXPORTLEN 64               // Taille maximale d'un numéro de port

int main(){
    int ecode;                       // Code erreur pour retour des fonctions 
    char serverAddr[MAXHOSTLEN];     // Adresse du serveur dans le proxy
    char serverPort[MAXPORTLEN];     // Port du serveur dans le proxy

    int descSockRDV;                 // Descripteur de socket de rendez-vous
    int descSockCOM;                 // Descripteur de socket de communication
    int SockServeur;                 // Descripteur pour la connexion au serveur
    int Client;                      // Descripteur pour les données du Client
    int Serveur;                     // Descripteur pour les données du Serveur

    struct addrinfo hints;           // Contrôle la fonction getaddrinfo
    struct addrinfo *res;            // Contient le résultat de la fonction getaddrinfo
    struct sockaddr_storage myinfo;  // Informations sur la connexion de RDV
    struct sockaddr_storage from;    // Informations sur le client connecté
    socklen_t len;                   // Variable utilisée pour stocker les longueurs des structures de socket
    char buffer[MAXBUFFERLEN];       // Tampon de communication entre le client et le serveur
    char login [MAXBUFFERLEN];       // Extraction du login
    char ServeurFTP [MAXBUFFERLEN];  // Extraction du serveur
    
    // Initialisation de la socket de RDV IPv4/TCP
    descSockRDV = socket(AF_INET, SOCK_STREAM, 0);
    if (descSockRDV == -1) {
         perror("Erreur de création socket RDV\n");
         exit(2);
    }
    // Publication de la socket au niveau du système
    // Assignation d'une adresse IP et un numéro de port
    // Mise à zéro de hints
    memset(&hints, 0, sizeof(hints));
    // Initialisation de hints
    hints.ai_flags = AI_PASSIVE;      // mode serveur, nous allons utiliser la fonction bind
    hints.ai_socktype = SOCK_STREAM;  // Utilisation TCP
    hints.ai_family = AF_INET;        // seules les adresses IPv4 seront présentées par la fonction getaddrinfo

     // Récupération des informations du serveur
     ecode = getaddrinfo(SERVADDR, SERVPORT, &hints, &res);
     if (ecode) {
         fprintf(stderr,"getaddrinfo: %s\n", gai_strerror(ecode));
         exit(1);
     }
     // Publication de la socket au niveau du système
     ecode = bind(descSockRDV, res->ai_addr, res->ai_addrlen);
     if (ecode == -1) {
         perror("Erreur avec la liaison de la socket de RDV");
         exit(3);
     }
     // Nous n'avons plus besoin de cette liste chainée addrinfo
     freeaddrinfo(res);

     // Récupération du nom de la machine et du numéro de port pour afficher à l'écran
     len=sizeof(struct sockaddr_storage);
     ecode=getsockname(descSockRDV, (struct sockaddr *) &myinfo, &len);
     if (ecode == -1)
     {
         perror("SERVEUR: getsockname");
         exit(4);
     }
     ecode = getnameinfo((struct sockaddr*)&myinfo, sizeof(myinfo), serverAddr,MAXHOSTLEN, 
                         serverPort, MAXPORTLEN, NI_NUMERICHOST | NI_NUMERICSERV);
     if (ecode != 0) {
             fprintf(stderr, "error in getnameinfo: %s\n", gai_strerror(ecode));
             exit(4);
     }
     printf("L'adresse d'écoute est: %s\n", serverAddr);
     printf("Le port d'écoute est: %s\n", serverPort);

     // Définition de la taille du tampon contenant les demandes de connexion
     ecode = listen(descSockRDV, LISTENLEN);
     if (ecode == -1) {
         perror("Erreur avec l'initialisation buffer d'écoute");
         exit(5);
     }

	len = sizeof(struct sockaddr_storage);
     // Attente connexion du client
     // Lorsque demande de connexion, création d'une socket de communication avec le client
     descSockCOM = accept(descSockRDV, (struct sockaddr *) &from, &len);
     if (descSockCOM == -1){
         perror("Erreur accept\n");
         exit(6);
     }
    // Échange de données avec le client connecté

    //Testez de mettre 220 
    strcpy(buffer, "220 anonymous@ftp.fau.de\n");
    write(descSockCOM, buffer, strlen(buffer));

    //Message qui demande de rentrer anonymous@ftp.fau.de
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN-1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture\n");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n", buffer);

    //Découpage du login et du serveur
    sscanf(buffer,"%[^@]@%s", login, ServeurFTP);
    printf("%s\n", login);
    printf("%s\n", ServeurFTP);

    //Création du Socket pour la connexion au serveur
    ecode = connect2Server (ServeurFTP,"21",&SockServeur);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture\n");
    }else {
        printf("Connexion réussie\n" );
    }

    // Afficher le contenu du buffer dans la console
    ecode = read(SockServeur, buffer, MAXBUFFERLEN);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture\n");
        exit(3);
    }   
    buffer[ecode] = '\0'; 
    printf("%s\n", buffer);

    //Envoi du login au serveur
    strcat(login, "\r\n");
    write(SockServeur, login, strlen(login));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le serveur\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);

    //Message de demande du mot de passe au client
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le serveur\n");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Lecture du mot de passe entrée
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le client\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);

    //Transmission du mot de passe entrée
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN - 1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le client");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n", buffer);

    //Envoi du mot de passe au serveur
    ecode = write(SockServeur, buffer, strlen(buffer));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le serveur\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);
    
    //Réception de la confirmation de connexion du serveur
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le serveur\n");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Transmission du SYST au client
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le client\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);

    //Lire la commande système (SYST)
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le client");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Envoyer la commande système au client (SYST)
    ecode = write(SockServeur, buffer, strlen(buffer));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le serveur\n");
        exit(3);
    }
    
    //Réception des informations systèmes reçus par serveur
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le serveur\n");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Envoi des informations système au client
    ecode = write(descSockCOM, buffer, strlen(buffer));
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le client\n");
        exit(3);
    }

    //Réception du PORT pour le client
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le client");
        exit(3);
    }
    buffer[ecode]='\0';
    printf("%s\n", buffer);

    //Découpage de l'adresse et du port pour la création du client
    int n1, n2, n3, n4, n5, n6;
    sscanf(buffer, "PORT %d,%d,%d,%d,%d,%d", &n1, &n2, &n3, &n4, &n5, &n6);
    char adresseClient[MAXHOSTLEN];
    char portClient[MAXPORTLEN];
    sprintf(adresseClient, "%d.%d.%d.%d", n1, n2, n3, n4);
    sprintf(portClient, "%d", n5 * 256 + n6);

    //Création du client
    ecode = connect2Server (adresseClient, portClient, &Client);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture\n");
    }else {
        printf("Connexion réussie\n" );
    }

    //Commande pour la passage du serveur en mode passif
    char passif[15] = "PASV\n";
    bzero(buffer, MAXBUFFERLEN);
    strncat(buffer, passif, strlen(passif));

    //Passage du serveur en mode passif
    ecode = write(SockServeur, buffer, strlen(buffer)); 
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le serveur\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);

    //Message de confirmation sur le passage en mode passif du serveur
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le serveur\n");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n", buffer);

    //Découpage de l'adresse et du port pour la création du serveur
    n1, n2, n3, n4, n5, n6 = 0;
    sscanf(buffer, "%*[^(](%d,%d,%d,%d,%d,%d", &n1, &n2, &n3, &n4, &n5, &n6);
    char adresseServeur[MAXHOSTLEN];
    char portServeur[MAXPORTLEN];
    sprintf(adresseServeur, "%d.%d.%d.%d", n1, n2, n3, n4);
    sprintf(portServeur, "%d", n5 * 256 + n6);

    //Création du Socket des données du Serveur
    ecode = connect2Server (adresseServeur, portServeur, &Serveur);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture\n");
    }else {
        printf("Connexion réussie\n" );
    }
    bzero(buffer, MAXBUFFERLEN - 1);

    //Commande de la connexion sur le PORT 200
    char connexionAuPort[30] = "200 Connecter sur le bon PORT\n";
    strncat(buffer, connexionAuPort, strlen(connexionAuPort));
    
    //Envoie de la réponse du serveur sur le client
    ecode = write(descSockCOM, buffer, strlen(buffer)); 
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le client\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);
    
    //Réception sur le client
    ecode = read(descSockCOM, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le client");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n", buffer);

    //Écriture du message du client sur le serveur
    ecode = write(SockServeur, buffer, strlen(buffer)); 
    if (ecode == -1) {
        perror("Erreur :Problème d'écriture sur le serveur\n");
        exit(3);
    }
    bzero(buffer, MAXBUFFERLEN);

    // Réception de la réponse par le serveur
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le serveur\n"); 
        exit(3); 
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Transfert de la réponse du serveur sur le client
    ecode = write(descSockCOM, buffer, strlen(buffer)); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture sur le client");
        exit(3);
    }

    //Réception de la réponse envoyé par le serveur
    ecode = read(Serveur, buffer, MAXBUFFERLEN - 1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Boucle pour envoyer les données du serveur sur le client
    while (ecode != 0) {
        ecode = write(Client, buffer, strlen(buffer));
        bzero(buffer, MAXBUFFERLEN - 1);
        ecode = read(Serveur, buffer, MAXBUFFERLEN - 1);
        if (ecode == -1) {
            perror("Erreur :Problème de lecture");
            exit(3);
        }
    }

    close(Client);
    close(Serveur);

    bzero(buffer, MAXBUFFERLEN - 1);

    //Affichage de la liste ls 
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1);
    if (ecode == -1) {
        perror("Erreur :Problème de lecture");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Transfert des données sur le client
    ecode = write(descSockCOM, buffer, strlen(buffer));
    bzero(buffer, MAXBUFFERLEN);
    
    //Commande pour la fermeture du client
    char Fermeture[10] = "quit\n";
    strncat(buffer, Fermeture, strlen(Fermeture));
    ecode = write(SockServeur, buffer, strlen(buffer)); 
    bzero(buffer, MAXBUFFERLEN);
    
    //Réception des données sur le serveur
    ecode = read(SockServeur, buffer, MAXBUFFERLEN - 1); 
    if (ecode == -1) {
        perror("Erreur :Problème de lecture");
        exit(3);
    }
    buffer[ecode] = '\0';
    printf("%s\n",buffer);

    //Transfert des données sur le client
    ecode = write(descSockCOM, buffer, strlen(buffer));

    //Fermeture de la connexion
    close(descSockCOM);
    close(descSockRDV);
}

/*Maintenant que descSockCOM et descSockRDV sont fermés, les autres connexions associées aux deux sont aussi fermées*/