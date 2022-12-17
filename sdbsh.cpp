#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // pour le inet_pton()
#include <sys/types.h>  // pour les connect...
#include <cstring>  // pour le strlen()
#include <vector>
#include <atomic>
#include <signal.h>

#include "queries.hpp"

#define TAILLE_REQUETE 256
#define TAILLE_RESULTAT 512

using namespace std;

int sock_fd;

void handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        if (write(sock_fd, "EXIT", TAILLE_REQUETE) < 0) { 
            cerr << "ERROR : write error" << endl;
            exit(EXIT_FAILURE); 
        }
        close(sock_fd);     /* Coupe la connection avec le serveur */
        exit(EXIT_SUCCESS);
    }
}

bool connect_to_server(sockaddr_in& server_address, char* server_port) {
    /*  */
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        cerr << "ERROR : socket error" << endl; 
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(28772);

    // Conversion du port entré par le client en ip ou ipv6 binaire 
    if (inet_pton(AF_INET, server_port, &server_address.sin_addr) < 0) {
        cerr << "ERROR : invalid ip address" << endl;
        return false;
    }

    // demande au serveur pour se connecter
    if (connect(sock_fd, (struct sockaddr *)&server_address, sizeof(server_address)) != 0) {
        cerr << "ERROR : connection error" << endl;
        return false;
    }
    return true;
}

int main(int argc, char *argv[]){
    (void)argc;
    struct sockaddr_in serv_addr;
    bool disconnect = false;        /* Permet de savoir si le serveur est fermé ou pas */
    if (connect_to_server(serv_addr, argv[1])) {
        char requete[TAILLE_REQUETE], result[TAILLE_RESULTAT];
        signal(SIGINT, handler);
        signal(SIGTERM, handler);

        /* Boucle dans laquelle chaque requête est envoyée au serveur et pour chacune d'entres elles
        on attends que le serveur renvoie les résultat puis on les affiche */
        while (cin.getline(requete, TAILLE_REQUETE)) {
            if (write(sock_fd, requete, TAILLE_REQUETE) < 0) { cerr << "ERROR : write error" << endl; break; }
            while(read(sock_fd, result, TAILLE_RESULTAT) > 0) {
                if (strcmp(result, "END") == 0) { disconnect = true; }
                else if (strcmp(result, "STOP") == 0) { break; }
                else { cout << result; }
                memset(result, 0, TAILLE_RESULTAT);
            }
            /* On sort de la boucle quand le serveur se ferme pour ne plus donner d'autres requêtes à traiter */
            if (disconnect) { break; }
        }
        if (disconnect) { cout << "SERVER GOT SHUT DOWN" << endl; }
        kill(getpid(), SIGINT);
    } else {
        cerr << "ERROR : unable to connect to the server" << endl;
        exit(EXIT_FAILURE);
    }
}