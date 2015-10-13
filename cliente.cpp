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
#include <iostream>
#include <condition_variable>
#include <mutex>

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

int recebeu_resposta = 0;
std::condition_variable mensagens_cv;
std::mutex mtx;

std::priority_queue<Mensagem, std::vector<Mensagem>, ComparaMensagem> mensagens;


void recebe_mensagens_servidor(Conexao *conexao);

void interacao_usuario(Conexao *conexao);

bool respostas_para_receber() { 
    return recebeu_resposta != 0; 
}

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
        {
            std::thread thread_recebe_mensagens_servidor_tcp (recebe_mensagens_servidor, conexao_tcp);      
            std::thread thread_interacao_usuario_tcp (interacao_usuario, conexao_tcp);
            thread_interacao_usuario_tcp.join();
        }
    }

    
    return 0;
}

void recebe_mensagens_servidor(Conexao *conexao) {
    int  n;
    char recvline[MAXLINE + 1];
    std::string comando, arg1, arg2;

    while ((n = conexao->recebe_mensagem(recvline)) > 0) {
        recvline[n] = 0;
        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(std::string(recvline), resultado, rgx);
        comando = resultado[1];
        
        // Prioridade menor = menos importante = final da fila
        if (comando == "REQUEST") {
            mensagens.push(Mensagem(string(recvline), 0));
        } 
        // Prioridade maior = mais importante  = começo da fila
        else if (comando == "REPLY") {
            mensagens.push(Mensagem(string(recvline), 2));
            recebeu_resposta = 1;
            mensagens_cv.notify_one();
            printf("Recebemos reply : %s\n", recvline);
        }

    }
    if (n < 0)
        fprintf(stderr,"read error :(\n");
}

void interacao_usuario(Conexao *conexao) {

    int opcao;
    bool logado = false;
    bool cadastrando = false;
    bool quer_sair = false;
    std::string aux1, aux2, aux3, output;
    // char str1[31], str2[31], str3[31];
    printf ("Olá senhor usuário, gostaria de jogar um jogo? :v\n MUAHAHA \n =) \n");


    printf ("Antes de executar qualquer ação, você deve estar logado.\n");
    while (!quer_sair){
        while (!logado && !quer_sair) {
            printf ("Digite: \n  1 para fazer login\n  2 para criar um novo usuário\n  3 para sair do programa\n");
            scanf ("%d", &opcao);
            aux1 = "";
            aux2 = "";
            aux3 = "";
            if (opcao == 1) {
                printf("Digite seu login: \n");
                std::cin >> aux1;
                printf("Digite sua senha: \n");
                std::cin >> aux2;
                if (!aux1.empty() && !aux2.empty()) {
                    output = "LOGIN " + aux1 + " " + aux2;

                    recebeu_resposta = 0;
                    conexao->envia_mensagem(output);

                    std::unique_lock<std::mutex> lck(mtx);
                    mensagens_cv.wait(lck, respostas_para_receber);

                    Mensagem msg(mensagens.top());
                    aux3 = msg.conteudo;
                    mensagens.pop(); 

                    std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
                    std::smatch resultado;
                    std::regex_search(aux3, resultado, rgx);
                    std::string comando = resultado[1];
                    std::string arg1 = resultado[2];
                    std::string arg2 = resultado[3];
              

                    if (arg1 == "000") {
                        printf("Logado com sucesso como usuário %s.\n", arg2.c_str());
                        logado = true;     
                    }
                    else if (arg1 == "001") {
                        printf("Senha incorreta para usuário %s, tente novamente.\n", arg2.c_str());
                        logado = false;
                    }
                    else if (arg1 == "002") {
                        printf("Usuário %s não existe!\n", arg2.c_str());
                        logado = false;
                    }
                } else { printf ("Formato incorreto. Tente novamente.\n"); }
            } else if (opcao == 2) {
                cadastrando = true;
                while (cadastrando) {
                    aux1 = "";
                    aux2 = "";
                    aux3 = "";
                    printf("Criando novo usuário. Digite um login: \n");
                    std::cin >> aux1;
                    // std::cin >> aux1 >> aux2; // scanf("%30s", aux1);
                    // if(!aux2.empty()) {
                    //     printf ("Excedeu número de parâmetros, tente novamente.\n");
                    //     continue;
                    // }
                    printf("Digita a senha: \n");
                    std::cin >> aux2;
                    // std::cin >> aux2 >> aux3; // scanf("%30s", aux2);
                    // if (!aux3.empty()) {
                    //     printf("Não insira espaços na senha! Tente novamente.\n");
                    //     continue;
                    // }
                    printf("Digite novamente a senha: \n");
                    std::cin >> aux3; // scanf("%30s", aux3);

                    if (aux2 != aux3) {
                        printf("Confirmação de senha falhou, repita o processo.\n\n");
                        continue;
                    } 
                    else {
                        // Mandou o cadastro pro servidor... espera resposta.
                        output = "NEWUSR " + aux1 + " " + aux2;

                        recebeu_resposta = 0;
                        conexao->envia_mensagem(output);

                        std::unique_lock<std::mutex> lck(mtx);
                        mensagens_cv.wait(lck, respostas_para_receber);

                        Mensagem msg(mensagens.top());
                        aux3 = msg.conteudo;
                        mensagens.pop(); 

                        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
                        std::smatch resultado;
                        std::regex_search(aux3, resultado, rgx);
                        std::string comando = resultado[1];
                        std::string arg1 = resultado[2];
                        std::string arg2 = resultado[3];

                        if (arg1 == "010") {
                            // Se o cadastro deu certo :
                            cadastrando = false;
                            printf("Seja bem vindo! Você está conectado como %s.\n", arg2.c_str());
                            logado = true;                            
                        }
                        else if (arg1 == "011") {
                            printf("Usuário %s já existe.\n", arg2.c_str());
                            cadastrando = true;
                            logado = false;
                        }
                    }
                }
            } else if (opcao == 3) {
                quer_sair = true;
            } 
            else {
                printf("Comando inválido.");
            }
        
        }

        // while (logado && !quer_sair) {       }
    }
    // while (mensagens.empty()) {}
}

