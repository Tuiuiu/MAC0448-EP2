/* Standard headers */
#include <cstdio> 
#include <iostream>
#include <vector>
#include <errno.h>
#include <strings.h>
#include <cstring>


/* Linux headers */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

/* Network headers */
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

/* POSIX Thread headers */
#include <pthread.h>

#include "Partida.hpp"


/* Macros */
#define LISTENQ 1
#define MAX_CONNECTIONS 100

// UHUL
void* client_connection(void*);


int main (int argc, char **argv) {

	int listenfd, connfd;
	int number_of_connections = 0;
	/* Informações sobre o socket (endereço e porta) ficam nesta struct */
	struct sockaddr_in servaddr;

	char string[100];

	pthread_t aux;
	std::vector<pthread_t> TCPThreads;



	if (argc != 2) {
		fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
		fprintf(stderr,"Vai rodar um servidor de jogo da velha na porta <Porta> TCP\n");
		exit(1);
	}

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket :(\n");
		exit(2);
	}

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family     = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port         = htons(atoi(argv[1]));
	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
		perror("bind :(\n");
		exit(3);
	}

	if (listen(listenfd, LISTENQ) == -1) {
		perror("listen :(\n");
		exit(4);
	}

	for (;;) {
		if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
			perror("accept :(\n");
			exit(5);
		}


		if (pthread_create(&aux, NULL, client_connection, NULL))
		{
			printf ("Erro na criação da thread %d.\n", number_of_connections);
			exit (EXIT_FAILURE);
		}
		 
		TCPThreads.emplace_back(aux);

		sprintf(string, "Conexão estabelecida\n");
		write(connfd, string, strlen(string));  
		break; // TEM QUE TIRAR ISSO DAQUI DEPOIS!!!
	}


	Partida partida;

	std::cout << partida.fazJogada(0, 0, 'X')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(0, 1, 'O')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(0, 2, 'X')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(1, 0, 'O')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(1, 1, 'O')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(1, 2, 'X')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(2, 0, 'O')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(2, 1, 'X')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();
	std::cout << partida.fazJogada(2, 2, 'X')<< std::endl;
	std::cout << partida.verificaResultado() << std::endl;
	partida.imprimeTabuleiro();

	return 0;	
}

void* client_connection(void*) {
	pthread_exit(NULL);
}