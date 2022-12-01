#include <iostream>
#include <fstream>
#include <unistd.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>  // pour le inet_pton()
#include <sys/types.h>  // pour les connect...
#include <cstring>  // pour le strlen()

using namespace std;

#define TAILLE_MAX 1000

int main(int argc, char *argv[]){

    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket error");
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(28772);

    // Conversion de string vers IPv4 ou IPv6 en binaire
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connectiong error");
    }

    /*
    dans la boucle, on fait de telle sorte que chaque requête lue à partir du terminal
    va être envoyée au serveur pour la traiter et puis le client lit directement après 
    le résultat de cette requête, puis fait un clear de la requête.
    */
    char requete[256];
    cout << "> ";
    while (cin.getline(requete, 256)) {
        size_t longueur = strlen(requete) + 1;
        cout << "Envoi..." << endl;
        if (write(sock_fd, requete, longueur) < 0) {
            perror("write error");
        }
        cout << "Query sent." << endl;

        // lecture du resultat écrit sur le fichier et l'afficher sur le terminal
        char result[TAILLE_MAX];
        while ((read(sock_fd, result, TAILLE_MAX)) > 0) {
            cout << result;
        }
        cout << "> ";

    }
    close(sock_fd);
    return 0;
}