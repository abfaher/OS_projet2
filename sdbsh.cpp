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
    if (signal == SIGINT) {
        if (write(sock_fd, "EXIT", TAILLE_REQUETE) < 0) { 
            cerr << "ERROR : write error" << endl;
            exit(EXIT_FAILURE); 
        }
        close(sock_fd);
        exit(EXIT_SUCCESS);
    }
}

bool connect_to_server(sockaddr_in& server_address, char* server_port) {
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        cerr << "ERROR : socket error" << endl; 
        return false;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(28772);

    // Conversion de string vers IPv4 ou IPv6 en binaire
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
    if (connect_to_server(serv_addr, argv[1])) {
        char requete[TAILLE_REQUETE], result[TAILLE_RESULTAT];
        signal(SIGINT, handler);

        /* dans la boucle, on fait de telle sorte que chaque requête lue à partir du terminal
        va être envoyée au serveur pour la traiter et puis le client lit directement après 
        le résultat de cette requête, puis fait un clear de la requête. */
        while (cin.getline(requete, 256) && (strcmp(requete, " ") != 0)) {
            if (write(sock_fd, requete, TAILLE_REQUETE) < 0) { cerr << "ERROR : write error" << endl; break; }
            while(read(sock_fd, result, TAILLE_RESULTAT) > 0) {
                if (strcmp(result, "STOP") == 0) { break; }
                else { 
                    cout << result;
                    if (strcmp(result, "SERVER SHUTTING DOWN") == 0) { break; }
                }
                memset(result, 0, TAILLE_RESULTAT);
            }
        }
        kill(getpid(), SIGINT);
    } else {
        cerr << "ERROR : unable to connect to the server" << endl;
        exit(EXIT_FAILURE);
    }
}