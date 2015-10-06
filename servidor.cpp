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

std::vector<Usuario*> usuarios_tcp;

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
		
		usuarios_tcp.emplace_back(useraux);*/

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
	bool loginExiste = false;
	bool logado = false;
	std::string comando, arg1, arg2;

	Usuario *usuario;
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
		std::string stringaux;
		/*for(size_t i=0; i<resultado.size(); ++i)
		{
		    std::cout << "Resultado " << i << " é: " << resultado[i] << std::endl;
		}*/

		printf ("Comando: %s\nArg1: %s\nArg2: %s\n", comando.c_str(), arg1.c_str(), arg2.c_str());

		// FAZER O LIST

		if (logado)
		{
			if (comando == "LIST")
			{
				usuario->escreve ("Login\tHora de login\tEstado\n");
				for (auto usuario : usuarios_tcp)
				{
					if (usuario->esta_conectado())
					{
						stringaux = usuario->get_login() + "\t" + usuario->get_hora_ultima_conexao() +"\t";
						usuario->esta_em_jogo() ? stringaux += "Em jogo\n" : stringaux += "Ocioso\n";
						usuario->escreve(stringaux);
					}
				}
			}
			else if (comando == "LOGOUT")
			{
				logado = false;
				usuario->escreve ("Logout feito com sucesso\n");
			}
			else if (comando == "QUIT")
			{
				pthread_exit (NULL);
			}
		}
		else
		{
			if (comando == "LOGIN")
			{
				loginExiste = false;
				if (!arg1.empty() && !arg2.empty()) {
					for (auto user : usuarios_tcp) {
						if (user->get_login() == arg1) {
							if (user->confere_senha(arg2)) {
								ConexaoTCP *conexaoaux = new ConexaoTCP(connfd);
								user->atualiza_conexao(conexaoaux);
								usuario = user;
								loginExiste = true;
								logado = true;
								usuario->conecta();
								stringaux = "Conectado como \'";
								stringaux += arg1;
								stringaux += "\'.\n";
								write(connfd, stringaux.c_str(), stringaux.length());
								break;
							}
							else {
								stringaux = "Senha incorreta\n";
								write(connfd, stringaux.c_str(), stringaux.length());
								loginExiste = true;
								break;
							}
						}
					}
					if (loginExiste == false) {
						stringaux = "Login \'";
						stringaux += arg1; 
						stringaux += "\' não existente\n";
						write(connfd, stringaux.c_str(), stringaux.length());
					}
				}
				else {
					stringaux = "Argumentos para LOGIN não estão corretos. Formato : 'LOGIN usuario senha'\n";
					write(connfd, stringaux.c_str(), stringaux.length());
				}
			}
			else if (comando == "NEWUSR")
			{
				if (!arg1.empty() && !arg2.empty()) {
					bool usuario_ja_existe = false;

					for (auto useraux : usuarios_tcp)
					{
						if (useraux->get_login() == arg1)
						{
							usuario_ja_existe = true;
							break;
						}
					}

					if (usuario_ja_existe)
					{
						stringaux = "Login " + arg1 + " já existe. Escolha outro.\n";
						write(connfd, stringaux.c_str(), stringaux.length());
					}
					else
					{
						usuario =  new Usuario(new ConexaoTCP(connfd), arg1, arg2);
						usuarios_tcp.emplace_back(usuario);
						logado = true;
						stringaux = "Novo usuário criado. Conectado como \'";
						stringaux += arg1;
						stringaux += "\'.\n";
						write(connfd, stringaux.c_str(), stringaux.length());
					}
				}
			}
			else 
			{
				stringaux = "Antes de solicitar qualquer comando, é necessário que esteja logado.\n";
				write(connfd, stringaux.c_str(), stringaux.length());
				stringaux = "Se possui uma conta, digite \'LOGIN usuario senha\' para se conectar.\n";
				write(connfd, stringaux.c_str(), stringaux.length());
				stringaux = "Caso contrario, digite \'NEWUSR usuario senha\' para criar um novo usuário.\n";
				write(connfd, stringaux.c_str(), stringaux.length());
			}
		}

		strcpy (recvline, "");
	}	

	pthread_exit(NULL);
}