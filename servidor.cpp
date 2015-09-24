/* Standard headers */
#include <cstdio> 
#include <iostream>
#include <vector>
#include <errno.h>
#include <strings.h>
#include <cstring>
#include <string>
#include <regex>


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
#include "Usuario.hpp"


/* Macros */
#define LISTENQ 1
#define MAX_CONNECTIONS 100
#define MAXLINE 4096

// UHUL
void* client_connection(void*);

std::vector<Usuario> usuarios;

int main (int argc, char **argv) {

	int listenfd, connfd;
	int number_of_connections = 0;
	/* Informações sobre o socket (endereço e porta) ficam nesta struct */
	struct sockaddr_in servaddr;

	char string[100];

	pthread_t aux;
	std::vector<pthread_t> TCPThreads;

	//Conexao *conexaoaux;


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

	while (true) {
		if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
			perror("accept :(\n");
			exit(5);
		}


		/*conexaoaux = new ConexaoTCP(connfd);

		Usuario* useraux(conexaoaux);
		
		usuarios.emplace_back(useraux);*/

		if (pthread_create(&aux, NULL, client_connection, (void *) &connfd))
		{
			printf ("Erro na criação da thread %d.\n", number_of_connections);
			exit (EXIT_FAILURE);
		}
		 
		TCPThreads.emplace_back(aux);

		sprintf(string, "Conexão estabelecida\n");
		write(connfd, string, strlen(string));  
		// break; // TEM QUE TIRAR ISSO DAQUI DEPOIS!!!
	}

	return 0;	
}

void* client_connection(void* entrada) {
	int* aux = (int*) entrada;
	int connfd = *(aux);
	//Usuario* usuario = (Usuario*) entrada;
	//int x, y;
	//char simbolo;
	//bool logado = false;
	std::string comando, arg1, arg2;

	Partida partida;

	/* Armazena linhas recebidas do cliente */
	char    recvline[MAXLINE + 1];
	/* Armazena o tamanho da string lida do cliente */
	ssize_t  n;

	while ((n=read(connfd, recvline, MAXLINE)) > 0) {
		/*sscanf(recvline, "%d %d %c", &x, &y, &simbolo);
		partida.fazJogada(x,y,simbolo);
		std::cout << partida.verificaResultado() << std::endl;
		partida.imprimeTabuleiro();*/

		recvline[n] = '\0';

		printf ("Recvline: %s\n", recvline);

		std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
		std::smatch resultado;
		std::regex_search(std::string(recvline), resultado, rgx);
		comando = resultado[1];
		arg1 = resultado[2];
		arg2 = resultado[3];
		/*for(size_t i=0; i<resultado.size(); ++i)
		{
		    std::cout << "Resultado " << i << " é: " << resultado[i] << std::endl;
		}*/

		printf ("Comando: %s\nArg1: %s\nArg2: %s\n", comando.c_str(), arg1.c_str(), arg2.c_str());

		// FAZER O LIST

		/*if (logado)
		{
			if (strcmp (recvline, "LIST") == 0)
			{
				usuario->escreve ("Login\tHora de login\tEstado\n");
				for (auto usuario : usuarios)
				{
					usuario->escreve ();
				}
			}
		}
		else
		{

		}*/

		strcpy (recvline, "");
	}	

	pthread_exit(NULL);
}