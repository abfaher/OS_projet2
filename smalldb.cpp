#include "db.hpp"
#include "queries.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <fstream>
#include <iostream>
#include <atomic>

using namespace std;

#define RESULT_TAILLE 512
#define WAITING_REQUESTS 5

atomic<int> communication_socket;
database_t db;


void* queries_management(void* ptr) {
	atomic<int>* sock = (atomic<int>*)ptr;
	char requete[256];
	query_result_t result;
	string stop = "STOP";

	while (read(*sock, requete, 256) > 0) {
		parse_and_execute(result, &db, requete);
		for (size_t i = 0; i < result.students.size(); i++) {
			if ((write(*sock, result.students[i].c_str(), RESULT_TAILLE)) < 0) {
			perror("write error");
			}
		}
		result.students.clear();
		write(*sock, stop.c_str(), stop.length());  // envoyer un message d'arrêt
	}
	// printf("Client %d disconnected (normal). closing connections and thread",(int)(*sock));
	return NULL;
}

int main(int argc, char *argv[]) {
		(void)argc;
		const char* db_path = argv[1];
		cout << "Welcome to the Tiny Database!" << endl;
		cout << "Loading the database..." << endl;
		db_load(&db, db_path);

		// creating a socket
		int listening_fd;
		if ((listening_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Socket error");
			exit(EXIT_FAILURE);
		}

		struct sockaddr_in srv_address;

		// réutilisation du port/adresse
		// int opt = 1;
		// setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

		// struct sockaddr_in address;
		srv_address.sin_family = AF_INET;
		srv_address.sin_addr.s_addr = INADDR_ANY;
		srv_address.sin_port = htons(28772);

		// liaison du serveur/client
		if (bind(listening_fd, (struct sockaddr *)&srv_address, sizeof(srv_address)) < 0) {
				perror("bind error");
				exit(EXIT_FAILURE);
		}

		// listen va attendre jusqu'à ce qu'un client fait une demande de connexion     
		if (listen(listening_fd, WAITING_REQUESTS) < 0) { // 2ème paramètre => nombre de connexions en attente
				perror("listening error");
				exit(EXIT_FAILURE);
		}
		
		// création du thread
		pthread_t threads[WAITING_REQUESTS];
		int i = 0;

		while (true) {
				cout << "waiting for a conection..." << endl;
				// accept va accepter la demande de connexion
				size_t addrlen = sizeof(srv_address);
				communication_socket = accept(listening_fd, (struct sockaddr *)&srv_address, (socklen_t *)&addrlen);

				printf("Accepted connection (%d)\n", communication_socket.load());

				// déterminer les paramètres du thread
				pthread_create(&threads[i], NULL, queries_management, &communication_socket);
				i++;
		}

		cout << "thread's job is done. " << endl;

		close(listening_fd);
		return 0;
}
