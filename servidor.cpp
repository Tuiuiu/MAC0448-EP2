/* Standard headers */
#include <cstdio> 
#include <iostream>
#include <vector>
#include <errno.h>
#include <strings.h>
#include <cstring>
#include <string>
#include <regex>
#include <thread>
#include <memory>


/* Linux headers */
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

/* Network headers */
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>


#include "Partida.hpp"
#include "Usuario.hpp"


/* Macros */
#define LISTENQ 1
#define MAX_CONNECTIONS 100
#define MAXLINE 4096

void client_connection(ConexaoPtr conexao);

std::vector<UsuarioPtr> usuarios_tcp;

void comando_prepare_list(std::vector<UsuarioPtr> &copia_lista_usuarios, UsuarioPtr usuario);

void comando_list(std::vector<UsuarioPtr> &copia_lista_usuarios, UsuarioPtr usuario);

void comando_logout(UsuarioPtr usuario);

UsuarioPtr comando_login(ConexaoPtr conexao, std::string login, std::string senha);

UsuarioPtr comando_newusr(ConexaoPtr conexao, std::string login, std::string senha);

int main (int argc, char **argv) {

	int listenfd, connfd;
	/* Informações sobre o socket (endereço e porta) ficam nesta struct */
	struct sockaddr_in servaddr;

	char string[100];

	std::vector<std::thread> TCPThreads;

	//ConexaoPtr conexaoaux;


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


		ConexaoPtr conexaoaux = std::make_shared<ConexaoTCP>(connfd);

		/* UsuarioPtr useraux(conexaoaux);
		
		usuarios_tcp.emplace_back(useraux);*/
		 
		TCPThreads.emplace_back(client_connection, conexaoaux);

		sprintf(string, "Conexão estabelecida\n");
		conexaoaux->envia_mensagem(string); // write(connfd, string, strlen(string));
	}

	return 0;	
}

void client_connection(ConexaoPtr conexao) {
	// int* aux = (int*) entrada;
	// int connfd = *(aux);
	//UsuarioPtr usuario = (UsuarioPtr) entrada;
	//int x, y;
	//char simbolo;
	bool logado = false;
	bool lista_foi_preparada = false;
	std::vector<UsuarioPtr> copia_lista_usuarios;
	
	std::string comando, arg1, arg2;

	UsuarioPtr usuario;
	Partida partida;

	/* Armazena linhas recebidas do cliente */
	char    recvline[MAXLINE + 1];
	/* Armazena o tamanho da string lida do cliente */
	ssize_t  n;

	while ((n=conexao->recebe_mensagem(recvline)) > 0) {
		/*sscanf(recvline, "%d %d %c", &x, &y, &simbolo);
		partida.fazJogada(x,y,simbolo);
		std::cout << partida.verificaResultado() << std::endl;
		partida.imprimeTabuleiro();*/

		recvline[n] = '\0';

		printf ("Recvline: %s\n", recvline);

		std::regex rgx("([A-Z_]*)(\\s+(\\w*))?(\\s+(\\w*))?");
		std::smatch resultado;
		std::regex_search(std::string(recvline), resultado, rgx);
		comando = resultado[1];
		arg1 = resultado[3];
		arg2 = resultado[5];
		std::string stringaux;
		/*for(size_t i=0; i<resultado.size(); ++i)
		{
		    std::cout << "Resultado " << i << " é: " << resultado[i] << std::endl;
		}*/

		printf ("Comando: %s\nArg1: %s\nArg2: %s\n", comando.c_str(), arg1.c_str(), arg2.c_str());

		if (logado)
		{
			if (comando == "PREPARE_LIST")
			{	
				lista_foi_preparada = true;
				comando_prepare_list(copia_lista_usuarios, usuario);
			}
			else if (comando == "LIST")
			{
				std::cout << "Ta oq essa porra? " << lista_foi_preparada << std::endl;
				if(lista_foi_preparada) {
					comando_list(copia_lista_usuarios, usuario);
					lista_foi_preparada = false;
				}
				else 
					usuario->escreve("REPLY 032");
			}
			else if (comando == "LOGOUT")
			{
				logado = false;
				comando_logout(usuario);
			}
			else if (comando == "QUIT")
			{
				break;
			}
		}
		else
		{
			if (comando == "LOGIN")
			{
				usuario = comando_login(conexao, arg1, arg2);
				if (usuario == nullptr) {
					logado = false;
				}
				else { 
					logado = true; 
				}
			}
			else if (comando == "NEWUSR")
			{
				usuario = comando_newusr(conexao, arg1, arg2);
				if (usuario == nullptr) {
					logado = false;
				}
				else { 
					logado = true; 
				}
			}
			else 
			{
				stringaux = "Antes de solicitar qualquer comando, é necessário que esteja logado.\n";
				conexao->envia_mensagem(stringaux);
				stringaux = "Se possui uma conta, digite \'LOGIN usuario senha\' para se conectar.\n";
				conexao->envia_mensagem(stringaux);
				stringaux = "Caso contrario, digite \'NEWUSR usuario senha\' para criar um novo usuário.\n";
				conexao->envia_mensagem(stringaux);
			}
		}
		strcpy (recvline, "");
	}	
}

void comando_prepare_list(std::vector<UsuarioPtr> &copia_lista_usuarios, UsuarioPtr usuario) {
	//copia_lista_usuarios = usuarios_tcp;
	for (auto user : usuarios_tcp)
	{
		if (user->esta_conectado())
			copia_lista_usuarios.emplace_back(user);
	}

	int tam_lista_usuarios = copia_lista_usuarios.size();
	printf("tam_lista_usuarios = %d\n", tam_lista_usuarios);
	std::string stringaux = "REPLY 030 " + std::to_string(tam_lista_usuarios);
	printf("stringaux = %s\n", stringaux.c_str());
	usuario->escreve(stringaux);
}

void comando_list(std::vector<UsuarioPtr> &copia_lista_usuarios, UsuarioPtr usuario) {
	for (auto bla : copia_lista_usuarios) {
		printf("\n\n\n\n AEHOOOO %s \n\n", bla->get_login().c_str());
	}
	while (!copia_lista_usuarios.empty()){
		UsuarioPtr aux = copia_lista_usuarios.back();
		copia_lista_usuarios.pop_back();
		if (aux->esta_conectado())
		{
			std::string stringaux = "REPLY 031 " + aux->get_login() + " " + aux->get_hora_ultima_conexao() + " ";
			aux->esta_em_jogo() ? stringaux += "Em jogo\n" : stringaux += "Ocioso\n";
			usuario->escreve(stringaux);
		}	
	}
}

void comando_logout(UsuarioPtr usuario) {
	usuario->desconecta();
	usuario->escreve ("REPLY 020 " + usuario->get_login());
}

UsuarioPtr comando_login(ConexaoPtr conexao, std::string login, std::string senha) {
	bool loginExiste = false;
	if (!login.empty() && !senha.empty()) {
		for (auto user : usuarios_tcp) {
			if (user->get_login() == login) {
				if (user->confere_senha(senha)) {
					user->atualiza_conexao(conexao);
					// usuario = user;
					loginExiste = true;
					// logado = true;
					user->conecta();
					//stringaux = "Conectado como \'";
					//stringaux += login;
					//stringaux += "\'.\n";
					std::string stringaux = "REPLY 000 ";
					stringaux += login;
					conexao->envia_mensagem(stringaux);
					return user;
				}
				else {
					std::string stringaux = "REPLY 001 ";
					stringaux += login;
					conexao->envia_mensagem(stringaux);
					loginExiste = true;
					return nullptr;
				}
			}
		}
		if (loginExiste == false) {
			//stringaux = "Login \'";
			//stringaux += login; 
			//stringaux += "\' não existente\n";
			std::string stringaux = "REPLY 002 ";
			stringaux += login;
			conexao->envia_mensagem(stringaux);
			return nullptr;
		}
	}
	return nullptr;
}

UsuarioPtr comando_newusr(ConexaoPtr conexao, std::string login, std::string senha) {
	if (!login.empty() && !senha.empty()) {
		bool usuario_ja_existe = false;

		for (auto useraux : usuarios_tcp)
		{
			if (useraux->get_login() == login)
			{
				usuario_ja_existe = true;
				break;
			}
		}

		if (usuario_ja_existe)
		{
			std::string stringaux = "REPLY 011 " + login;
			conexao->envia_mensagem(stringaux);
			return nullptr;
		}
		else
		{
			UsuarioPtr usuario = std::make_shared<Usuario>(conexao, login, senha);
			usuarios_tcp.emplace_back(usuario);
			// logado = true;
			std::string stringaux = "REPLY 010 ";
			stringaux += login;
			conexao->envia_mensagem(stringaux);
			return usuario;
		}
	}
	return nullptr;
}

