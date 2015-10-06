/* Código simples de um cliente http que envia um HEAD pro servidor.
 * Não é o código ideal (deveria ter returns ou exits após os erros
 * das funções por exemplo) mas é suficiente para exemplificar o
 * conceito.
 * 
 * RFC do HTTP: http://www.faqs.org/rfcs/rfc2616.html
 *
 * Prof. Daniel Batista em 21/08/2011. Modificado em cima do cliente
 * do daytime da aula 04
 *
 * Bugs? Tente consertar primeiro! Depois me envie email :) batista@ime.usp.br
 */

#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <thread>
#include <regex>
#include <queue>
#include <vector>
#include <cstdlib>

#include "Conexao.hpp"

#define MAXLINE 100

class Mensagem {
  public:
    Mensagem(std::string c, int p) : conteudo(c), prioridade(p) {}
    std::string conteudo;
    int prioridade;
};

struct ComparaMensagem {
    bool operator()(Mensagem const & m1, Mensagem const & m2) {
        return m1.prioridade < m2.prioridade;
    }
};

void recebe_mensagens_servidor(Conexao *conexao);


int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    struct  hostent *hptr;



    if (argc != 4) {
        fprintf(stderr,"Uso: %s <Endereco IP|Nome> <Porta> <TCP|UDP>\n",argv[0]);
        exit(1);
    }

    if ((hptr = gethostbyname(argv[1])) == NULL) {  
        fprintf(stderr,"gethostbyname :(\n");
        exit(1);
    }

    if (string(argv[3]) == "TCP") {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
            fprintf(stderr,"socket error :( \n");
           
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[2])); 

        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
            fprintf(stderr,"inet_pton error for %s :(\n", argv[1]);
           
        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
            fprintf(stderr,"connect error :(\n");

        Conexao *conexao_tcp = new ConexaoTCP(sockfd);
        std::thread recebe_mensagens_tcp (recebe_mensagens_servidor, conexao_tcp);      
    }

    std::thread(interacao_usuario());

    // printf("Bem-vindo a'O Jogo da Velha\n");
       
    return 0;
}

void recebe_mensagens_servidor(Conexao *conexao) {
    int  n;
    char recvline[MAXLINE + 1];
    std::priority_queue<Mensagem, std::vector<Mensagem>, ComparaMensagem> mensagens;
    std::string comando, arg1, arg2;

    while ((n = conexao->recebe_mensagem(recvline)) > 0) {
        recvline[n] = 0;
        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(std::string(recvline), resultado, rgx);
        comando = resultado[1];
        
        if (comando == "REQUEST") {
            mensagens.push(Mensagem(string(recvline), 0));
        } else {
            mensagens.push(Mensagem(string(recvline), 1));
        }

    }
    if (n < 0)
        fprintf(stderr,"read error :(\n");
}

void interacao_usuario() {
    printf ("Olá senhor usuário, gostaria de jogar um jogo? :v\n MUAHAHA \n =) \n");
}

