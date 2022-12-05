#include "db.hpp"
#include "queries.hpp"
#include "utils.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <fstream>
#include <iostream>

using namespace std;

#define TAILLE_MAX 1000

struct th_param {
	int sock;
	database_t db;
};

void* queries_management(void* ptr) {
	th_param* param = (th_param*)ptr;
	char requete[256];
	query_result_t result;

	while (safe_read(param->sock, requete, 256) > 0) {
		parse_and_execute(result, &param->db, requete);
		// cout << result;
		char buffer[512];
		result.students.push_back("end");
		for (size_t i = 0; i < result.students.size(); i++) {
			strcpy(buffer, result.students[i].c_str());
			safe_write(param->sock, buffer, sizeof(buffer));
			memset(buffer, 0, sizeof(buffer));
		}
		result.students.clear();
	}
	return NULL;
}

int main(int argc, char *argv[]) {
		(void)argc;
		const char* db_path = argv[1];
		database_t db;
		cout << "Welcome to the Tiny Database!" << endl;
		cout << "Loading the database..." << endl;
		db_load(&db, db_path);

		// creating a socket
		int listening_fd;
		if ((listening_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("Socket error");
			exit(EXIT_FAILURE);
		}

		// réutilisation du port/adresse
		int opt = 1;
		setsockopt(listening_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

		struct sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(28772);

		// liaison du serveur/client
		if (bind(listening_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
				perror("bind error");
				exit(EXIT_FAILURE);
		}

		// listen va attendre jusqu'à ce qu'un client fait une demande de connexion
		if (listen(listening_fd, 3) < 0) { // 2ème paramètre => nombre de connexions en attente (3 CORRECT ??)
				perror("listening error");
				exit(EXIT_FAILURE);
		}

		while (true) {
				size_t addrlen = sizeof(address);
				// accept va accepter la demande de connexion
				cout << "waiting for a conection..." << endl;
				int communication_socket = accept(listening_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);

				// création du thread
				pthread_t t;

				// déterminer les paramètres du thread
				th_param param;
				param.db = db;
				param.sock = communication_socket;
				pthread_create(&t, NULL, queries_management, &param);
				pthread_join(t, NULL);
				cout << "thread's job is done. " << endl;
		}
		close(listening_fd);
		return 0;
}
