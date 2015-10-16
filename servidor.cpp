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

void comando_request(UsuarioPtr usuario, std::string nome_oponente);

void comando_answer(UsuarioPtr usuario, std::string resposta, std::string oponente);

void comando_play(UsuarioPtr usuario, std::string x, std::string y);

void comando_finish(UsuarioPtr usuario);

int main (int argc, char **argv) {

	int listenfd, connfd;
	/* Informações sobre o socket (endereço e porta) ficam nesta struct */
	struct sockaddr_in servaddr;

	//char string[100];

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

	printf ("[Servidor no ar. Aguardando conexoes na porta %s]\n",argv[1]);
	printf ("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");

	while (true) {
		if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
			perror("accept :(\n");
			exit(5);
		}


		ConexaoPtr conexaoaux = std::make_shared<ConexaoTCP>(connfd);

		/* UsuarioPtr useraux(conexaoaux);
		
		usuarios_tcp.emplace_back(useraux);*/
		 
		TCPThreads.emplace_back(client_connection, conexaoaux);

		//sprintf(string, "Conexão estabelecida\n");
		//conexaoaux->envia_mensagem(string); // write(connfd, string, strlen(string));
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
	PartidaPtr partida;

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
		std::string recvline_string(recvline);

		std::regex rgx("([A-Z_]*)(\\s+(\\w*))?(\\s+(\\w*))?");
		std::smatch resultado;
		std::regex_search(recvline_string, resultado, rgx);
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
			if (usuario->esta_em_jogo())
			{
				if (comando == "PLAY")
				{
					comando_play(usuario, arg1, arg2);
				}
				else if (comando == "FINISH")
				{
					comando_finish(usuario);
				}
				else if (comando == "LOGOUT")
				{
					logado = false;
					comando_logout(usuario);
				}
				else
					conexao->envia_mensagem("REPLY 099"); // comando inválido
			}
			else // quando não está em jogo
			{
				if (comando == "PREPARE_LIST")
				{	
					lista_foi_preparada = true;
					comando_prepare_list(copia_lista_usuarios, usuario);
				}
				else if (comando == "LIST")
				{
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
				else if (comando == "REQUEST")
				{
					comando_request(usuario, arg1);
				}
				else if (comando == "ANSWER")
				{
					comando_answer(usuario, arg1, arg2);
				}
				else
					conexao->envia_mensagem("REPLY 099"); // comando inválido
			}
		}
		else // comandos quando não está logado
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
				conexao->envia_mensagem("REPLY 099"); // comando inválido
			}
		}
		strcpy (recvline, "");
	}	

	//if (logado)
	//	usuario->desconecta();
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
			aux->esta_em_jogo() ? stringaux += "Jogando\n" : stringaux += "Ocioso\n";
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
					std::string stringaux;
					if (user->esta_em_jogo())
					{
						stringaux = "REPLY 003 ";
						stringaux += login + " " + user->simbolo() + " " + user->get_partida()->tabuleiro_em_string();
					} 
					else
					{
						stringaux = "REPLY 000 ";
						stringaux += login;
					}
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

void comando_request(UsuarioPtr usuario, std::string nome_oponente) {
	if (usuario->get_login() == nome_oponente)
		usuario->escreve("REPLY 044 " + nome_oponente); // não pode jogar com ele mesmo
	else
	{ 
		for (auto oponente : usuarios_tcp) {
			if (oponente->get_login() == nome_oponente)
			{
				if (oponente->esta_conectado())
				{
					if (!oponente->esta_em_jogo())
					{
						// oponente está disponível, envia o convite para ele
						oponente->escreve("REQUEST " + usuario->get_login());
					}
					else
					{
						usuario->escreve("REPLY 043 " + nome_oponente); // oponente já está em jogo
					}
				}
				else
				{
					usuario->escreve("REPLY 042 " + nome_oponente); // oponente não está conectado
				}
				return;
			}
		}

		usuario->escreve("REPLY 041 " + nome_oponente); // oponente não existe
	}
}

void comando_answer(UsuarioPtr usuario, std::string resposta, std::string nome_oponente)
{
	for (auto oponente : usuarios_tcp) {
		if (oponente->get_login() == nome_oponente)
		{
			if (oponente->esta_conectado())
			{
				if (!oponente->esta_em_jogo())
				{
					oponente->escreve("ANSWER " + resposta + " " + usuario->get_login());
					if (resposta == "S")
					{
						// oponente está disponível e o convite foi aceito, começa a partida
						PartidaPtr partida = std::make_shared<Partida>(usuario, oponente);
						usuario->set_partida(partida);
						oponente->set_partida(partida);
						usuario->escreve("START X " + oponente->get_login());
						oponente->escreve("START O " + usuario->get_login());
					}
				}
				else
				{
					usuario->escreve("REPLY 051 " + nome_oponente); // oponente já está em jogo
				}
			}
			else
			{
				usuario->escreve("REPLY 052 " + nome_oponente); // oponente não está conectado
			}
			return;
		}
	}
} 

void comando_play(UsuarioPtr usuario, std::string x_str, std::string y_str)
{
	PartidaPtr partida = usuario->get_partida();
	if (partida->get_simbolo_ultimo_jogador() == partida->simbolo(usuario))
	{
		usuario->escreve("REPLY 063"); // não é a sua vez
	}
	else
	{
		bool invalido = false;
		if (!x_str.empty() && !y_str.empty())
		{
			int x = atoi(x_str.c_str());
			int y = atoi(y_str.c_str());

			switch(partida->fazJogada(x, y, partida->simbolo(usuario)))
			{
				case VALIDA:
				{
					Resultado resultado_generico = partida->verificaResultado();
					std::string resultado_usuario, resultado_adversario;

					switch(resultado_generico)
					{
						case VELHA:
							resultado_usuario = resultado_adversario = "EMPATE";
							break;
						case NAO_ACABOU:
							resultado_usuario = resultado_adversario = "CONTINUA";
							break;
						case VITORIA_X:
							if (usuario->simbolo() == 'X')
							{
								resultado_usuario = "VITORIA";
								resultado_adversario = "DERROTA";
							}
							else
							{
								resultado_usuario = "DERROTA";
								resultado_adversario = "VITORIA";
							}
							break;
						case VITORIA_O:
							if (usuario->simbolo() == 'O')
							{
								resultado_usuario = "VITORIA";
								resultado_adversario = "DERROTA";
							}
							else
							{
								resultado_usuario = "DERROTA";
								resultado_adversario = "VITORIA";
							}
							break;
							
					}
					usuario->escreve("REPLY 060 " + resultado_usuario);
					usuario->adversario()->escreve("PLAY " + x_str + " " + y_str + " " + resultado_adversario);
					break;
				}
				case POSICAO_INEXISTENTE:
					usuario->escreve("REPLY 061");
					break;
				case POSICAO_OCUPADA:
					usuario->escreve("REPLY 062");
					break;
				case AGUARDE_SUA_VEZ:
					usuario->escreve("REPLY 063");
					break;
				case COMANDO_INVALIDO:
					invalido = true;
					break;
				default:
					invalido = true;
					break;
			}
		}
		else
			invalido = true;

		if (invalido)
			printf("Comando inválido\n");
	}
}

void comando_finish(UsuarioPtr usuario)
{
	usuario->adversario()->sai_jogo();
	usuario->sai_jogo();
}