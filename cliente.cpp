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


void recebe_mensagens_servidor(ConexaoPtr conexao);

void interacao_usuario(ConexaoPtr conexao);

bool efetuar_login (ConexaoPtr conexao);

void efetuar_logout (ConexaoPtr conexao);

bool efetuar_cadastro (ConexaoPtr conexao);

void listar_jogadores(ConexaoPtr conexao);


bool respostas_para_receber() { 
    return recebeu_resposta != 0; 
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    struct  hostent *hptr;
    char enderecoIPServidor[INET_ADDRSTRLEN];



    if (argc != 4) {
        fprintf(stderr,"Uso: %s <Endereco IP|Nome> <Porta> <TCP|UDP>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((hptr = gethostbyname(argv[1])) == NULL) {  
        fprintf(stderr,"gethostbyname :(\n");
        exit(EXIT_FAILURE);
    }

    if (hptr->h_addrtype != AF_INET) {
        fprintf(stderr,"h_addrtype :(\n");
        exit(1);
    }
    if ( (inet_ntop(AF_INET, hptr->h_addr_list[0], enderecoIPServidor, sizeof(enderecoIPServidor))) == NULL) {
        fprintf(stderr,"inet_ntop :(\n");
        exit (1);
    }

    if (string(argv[3]) == "TCP") {
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            fprintf(stderr,"socket error :( \n");
            exit(EXIT_FAILURE);
        }
           
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(atoi(argv[2])); 

        int resultado;
        if ((resultado = inet_pton(AF_INET, enderecoIPServidor, &servaddr.sin_addr)) <= 0)
        {
            fprintf(stderr,"inet_pton error %d for %s :(\n", resultado, argv[1]);
            exit(EXIT_FAILURE);
        }
           
        if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
        {
            fprintf(stderr,"connect error :(\n");
            exit(EXIT_FAILURE);
        }

        ConexaoPtr conexao_tcp = std::make_shared<ConexaoTCP>(sockfd);

        {
            std::thread thread_recebe_mensagens_servidor_tcp (recebe_mensagens_servidor, conexao_tcp);      
            std::thread thread_interacao_usuario_tcp (interacao_usuario, conexao_tcp);
            thread_interacao_usuario_tcp.join();
        }
    }

    
    return 0;
}

void recebe_mensagens_servidor(ConexaoPtr conexao) {
    int  n;
    char recvline[MAXLINE + 1];
    std::string comando, arg1, arg2;

    while ((n = conexao->recebe_mensagem(recvline)) > 0) {
        recvline[n] = 0;
        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(std::string(recvline), resultado, rgx);
        comando = resultado[1];
        arg1 = resultado[2];
        
        // Prioridade menor = menos importante = final da fila
        if (comando == "REQUEST") {
            mensagens.push(Mensagem(string(recvline), 0));
        } 
        // Prioridade maior = mais importante  = começo da fila
        else if (comando == "REPLY") {
            mensagens.push(Mensagem(string(recvline), 1)); 
            recebeu_resposta++;
            mensagens_cv.notify_one();
            //printf("Recebemos reply: %s\n", recvline);
        }

    }
    if (n < 0)
        fprintf(stderr,"read error :(\n");


    exit(EXIT_FAILURE);
}

void interacao_usuario(ConexaoPtr conexao) {

    int opcao = 0;
    bool logado = false;
    //bool cadastrando = false;
    bool quer_sair = false;
    std::string aux1, aux2, aux3, output;
    // char str1[31], str2[31], str3[31];

    printf ("Olá senhor usuário, gostaria de jogar um jogo? :v\n MUAHAHA \n =) \n");
    while (!quer_sair){
        while (!logado && !quer_sair) {

            bool error = false;
            do {
                // printf("\033[2J\033[;H");
                printf ("Antes de executar qualquer ação, você deve estar logado.\n");
                printf ("Digite:\n  1 para fazer login\n  2 para criar um novo usuário\n  3 para sair do programa\n");
                std::cin >> opcao;
                error = std::cin.fail();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (error);
            
            printf("Opção: %d\n", opcao);
            switch (opcao) {
                case 1:
                    logado = efetuar_login(conexao);
                    break;
                case 2:
                    logado = efetuar_cadastro(conexao);
                    break;
                case 3:
                    quer_sair = true;
                    break;
                default:
                    printf("Comando inválido.\n");
            }
        }

        while (logado && !quer_sair) {
            
            bool error;
            do {
                printf ("Digite:\n  1 para listar jogadores conectados\n  2 para novo jogo\n  3 para ver o hall of fame\n  4 para logout\n  5 para sair do programa\n");
                std::cin >> opcao;
                error = std::cin.fail();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (error);

            switch (opcao) {
                case 1:
                    listar_jogadores(conexao);
                    break; 
                case 2: 
                    //
                    break;
                case 3: 
                    //
                    break;
                case 4: 
                    efetuar_logout(conexao);
                    logado = false;
                    break;
                case 5: 
                    efetuar_logout(conexao);
                    quer_sair = true; 
                    break;
                default:
                    printf("Comando inválido.\n");
            }
        }
    }
    // while (mensagens.empty()) {}
}

bool efetuar_login (ConexaoPtr conexao) // devolve true se o login deu certo e false caso contrário
{
    std::string aux1, aux2, aux3;

    aux1 = "";
    aux2 = "";
    aux3 = "";

    printf("Digite seu login: \n");
    std::cin >> aux1;
    printf("Digite sua senha: \n");
    std::cin >> aux2;
    if (!aux1.empty() && !aux2.empty()) {
        std::string output = "LOGIN " + aux1 + " " + aux2;

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
            return true;     
        }
        else if (arg1 == "001") {
            printf("Senha incorreta para usuário %s, tente novamente.\n", arg2.c_str());
            return false;
        }
        else if (arg1 == "002") {
            printf("Usuário %s não existe!\n", arg2.c_str());
            return false;
        }
    } else {
        printf ("Formato incorreto. Tente novamente.\n");
        return false;
    }

    printf ("Erro inesperado. Tente novamente.\n");
    return false;
}


void efetuar_logout (ConexaoPtr conexao)
{    
    recebeu_resposta = 0;
    conexao->envia_mensagem("LOGOUT");

    std::unique_lock<std::mutex> lck(mtx);
    mensagens_cv.wait(lck, respostas_para_receber);

    Mensagem msg(mensagens.top());
    std::string aux = msg.conteudo;
    mensagens.pop(); 

    std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
    std::smatch resultado;
    std::regex_search(aux, resultado, rgx);
    std::string comando = resultado[1];
    std::string arg1 = resultado[2];
    std::string arg2 = resultado[3];
  

    if (arg1 == "020")
    {
        printf("Logout de %s efetuado com sucesso.\n", arg2.c_str());
        return;
    }
    else
    {
        mensagens.push(msg);
        printf ("Erro inesperado.\n");
        return;
    }
}

bool efetuar_cadastro (ConexaoPtr conexao) // devolve true se conseguiu cadastrar corretamente
{
    std::string aux1 = "";
    std::string aux2 = "";
    std::string aux3 = "";
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
        return false;
    } 
    else {
        // Mandou o cadastro pro servidor... espera resposta.
        std::string output = "NEWUSR " + aux1 + " " + aux2;

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
            printf("Seja bem vindo! Você está conectado como %s.\n", arg2.c_str());
            return true;                            
        }
        else if (arg1 == "011") {
            printf("Usuário %s já existe.\n", arg2.c_str());
            return false;
        }
    }

    printf ("Erro inesperado. Tente novamente.\n");
    return false;
}

void listar_jogadores(ConexaoPtr conexao)
{
    int num_usuarios;
    recebeu_resposta = 0;
    conexao->envia_mensagem("PREPARE_LIST");

    { 
        std::unique_lock<std::mutex> lck(mtx);
        mensagens_cv.wait(lck, respostas_para_receber);

        Mensagem msg(mensagens.top());
        std::string aux = msg.conteudo;
        mensagens.pop();


        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(aux, resultado, rgx);
        std::string comando = resultado[1];
        std::string arg1 = resultado[2];
        std::string arg2 = resultado[3];

        if (arg1 == "030") // começo da lista
        {
            num_usuarios = atoi(arg2.c_str());
            printf("num_usuarios = %d\n", num_usuarios);
        }
        else
        {
            mensagens.push(msg);
            printf ("Erro inesperado.\n");
            return;
        }
    }

    recebeu_resposta = 0;

    printf ("Login\tHora de login\tEstado\n");

    conexao->envia_mensagem("LIST");

    //printf("Antes do while\n");
    while (num_usuarios > 0)
    {
        //printf("Antes do mutex\n");
        std::unique_lock<std::mutex> lck(mtx);
        //printf("Antes de esperar\n");
        mensagens_cv.wait(lck, respostas_para_receber);
        //printf("Depois de esperar\n");

        Mensagem msg(mensagens.top());
        std::string aux = msg.conteudo;
        mensagens.pop();

        recebeu_resposta--;

        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)\\s+([0-9:]*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(aux, resultado, rgx);
        std::string comando = resultado[1];
        std::string arg1 = resultado[2];
        std::string arg2 = resultado[3];
        std::string arg3 = resultado[4];
        std::string arg4 = resultado[5];

        //printf("Aux : %s\n", aux.c_str());
        //printf("arg1 : %s, arg2 : %s, arg3 : %s, arg4 : %s\n", arg1.c_str(), arg2.c_str(), arg3.c_str(), arg4.c_str());

        if (arg1 == "031")
        {
            std::string stringaux = arg2 + "\t" + arg3 + "\t" + arg4 + "\n";
            printf("%s", stringaux.c_str());
        }
        else
        {
            mensagens.push(msg);
            printf("Erro inesperadoALDJWLFA\n");
            return;
        }

        num_usuarios--;
    }
}
