#include "db.hpp"
#include "queries.hpp"
#include "errorcodes.hpp"
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
#include <signal.h>

using namespace std;

#define RESULT_TAILLE 512
#define WAITING_REQUESTS 5

vector<int> communication_socket;
database_t db;
int listening_fd, connected_clients = 0;
vector<pthread_t> threads;

void wait_threads() {
	for (size_t i = 0; i < threads.size(); i++) {
		pthread_join(threads[i], NULL);
	}
	threads.clear();
}

void handler(int signal) {
	if (signal == SIGINT || signal == SIGTERM) {
		for (size_t i = 0; i < communication_socket.size(); i++) {
			write(communication_socket[i], "SERVER SHUTTING DOWN", RESULT_TAILLE);
		}
		wait_threads();		/* Fermeture des sessions clients */
		cout << "\nSmallDB : Saving database" << endl;
		db_save(&db);		/* Sauvegarde de la base de données */
		cout << "SmallDB : database saved" << endl;
		cout << "SmallDB : Closing the server" << endl;
		close(listening_fd);	/* Fermeture du socket du serveur */
		exit(EXIT_SUCCESS);
	} else if (signal == SIGUSR1) {
		/* Empêche à n'importe quel clients d'accéder à la db durant la sauvegarde */
		pthread_mutex_lock(&db.exclusiveAccess);
		pthread_mutex_lock(&db.readAccess);
		pthread_mutex_lock(&db.writeAccess);
		pthread_mutex_unlock(&db.exclusiveAccess);
		cout << "SmallDB : Saving database" << endl;
		db_save(&db);	/* Sauvegarde de la base de données */
		cout << "SmallDB : database saved" << endl;
		pthread_mutex_unlock(&db.readAccess);
		pthread_mutex_unlock(&db.writeAccess);
	}
}

void disconnect_client(int socket, bool connectionLost) {
	if (connectionLost) {		/* Vérifie si la déconnection du lcient était intentionnel */
		cout << "SmallDB : Lost connection to client " << socket << endl;
		cout << "SmallDB : Closing connection " << socket << endl;
		cout << "SmallDB : Closing thread for connection " << socket << endl;
	} else {
		cout << "SmallDB : Client " << socket << " disconnected (normal). Closing connections and thread" << endl;
	}
	close(socket);
	connected_clients--;
}

void* queries_management(void* ptr) {
	int sock = *(int*)ptr;
	char requete[256], text[512];
	query_result_t result;
	bool connectionStatus = true;

	while (read(sock, requete, 256) > 0) {		/* le serveur se met en attente d'une requête venant du client */
		if (strcmp(requete, "EXIT") == 0) { connectionStatus = false; break; } 
		else {
			parse_and_execute(result, &db, requete);
			if (result.status == QUERY_SUCCESS) {
				if ((strncmp(requete, "select", sizeof("select")-1) == 0) || (strncmp(requete, "insert", sizeof("insert")-1) == 0)) {
					for (size_t i = 0; i < result.students.size(); i++) {
						strcpy(text, student_to_str(&result.students[i]).c_str());
						if ((write(sock, text, RESULT_TAILLE)) < 0) { cerr << "ERROR : write error" << endl; }
						memset(text, 0, RESULT_TAILLE);
					}
				}
				if (strncmp(requete, "select", sizeof("select") - 1) == 0) { sprintf(text, "%d student(s) selected\n", (int)result.students.size()); }
				else if (strncmp(requete, "update", sizeof("update") - 1) == 0) { sprintf(text, "%d student(s) updated\n", (int)result.students.size()); }
				else if (strncmp(requete, "delete", sizeof("delete") - 1) == 0) { sprintf(text, "%d deleted student(s)\n", (int)result.students.size()); }
				write(sock, text, RESULT_TAILLE);
			} else { write(sock, result.errorMessage, RESULT_TAILLE); }
			result.students.clear();
			memset(text, 0, RESULT_TAILLE);
			memset(result.errorMessage, 0, RESULT_TAILLE);
			write(sock, "STOP", RESULT_TAILLE);  // envoyer un message d'arrêt
		}
	}
	disconnect_client(sock, connectionStatus);
	pthread_exit(NULL);
}

bool server_init(sockaddr_in& server_address) {
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(28772);

	// creating a socket
	if ((listening_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		cerr << "ERROR : socket error" << endl;
		return false;
	}
	/* Bind the socket to the server */
	if (bind(listening_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		cerr << "ERROR : bind error" << endl;
		return false;
	}
	// listen va attendre jusqu'à ce qu'un client fait une demande de connexion     
	if (listen(listening_fd, WAITING_REQUESTS) < 0) { // 2ème paramètre => nombre de connexions en attente
			cerr << "ERROR : listening error" << endl;
			return false;
	}
	return true;
}

void client_management(sockaddr_in& server_address, int& nb_clients) {
	// accept va accepter la demande de connexion
	size_t addrlen = sizeof(server_address);
	int client_socket = accept(listening_fd, (struct sockaddr *)&server_address, (socklen_t *)&addrlen);

	if (client_socket > 0) { 
		communication_socket.push_back(client_socket);
		cout << "SmallDB : Accepted connection (" << communication_socket.back() << ")" << endl;

		threads.reserve(1);
		// Bloque le signal (pour le thread courant)
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGUSR1);
		sigaddset(&mask, SIGINT);
		sigaddset(&mask, SIGTERM);
		sigprocmask(SIG_BLOCK, &mask, NULL);
		pthread_create(&threads[nb_clients], NULL, queries_management, &communication_socket.back());
		// Débloque le signal (pour le thread courant)
		sigprocmask(SIG_UNBLOCK, &mask, NULL);
		nb_clients++;
	}
}

int main(int argc, char *argv[]) {
		(void)argc;
		const char* db_path = argv[1];
		cout << "Loading the database..." << endl;
		db_load(&db, db_path);

		struct sockaddr_in srv_address;
		if (server_init(srv_address)) {
			cout << "SmallDB : DB loaded (" << db_path << ") : " << db.data.size() << " student(s) in database" << endl;

			/* Initialise les mutex pour les accés à la db (écriture/lecture) */
			pthread_mutex_init(&db.exclusiveAccess, NULL);
			pthread_mutex_init(&db.readAccess, NULL);
			pthread_mutex_init(&db.writeAccess, NULL);

			// Définit le signal handler
			struct sigaction action;
			action.sa_handler = handler;
			sigemptyset(&action.sa_mask);
			action.sa_flags = 0;
			sigaction(SIGUSR1, &action, NULL);
			sigaction(SIGINT, &action, NULL);
			sigaction(SIGTERM, &action, NULL);
			while (true) {
				// Si un pthread_create est fait ici, le signal est bloqué pour les deux threads
				client_management(srv_address, connected_clients);
			}
		} else {
			cerr << "ERROR : unable to launch the server" << endl;
			exit(EXIT_FAILURE); 
		}
}
