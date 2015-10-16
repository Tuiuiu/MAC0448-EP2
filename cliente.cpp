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
#include <atomic>

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
std::mutex mtx2;

std::priority_queue<Mensagem, std::vector<Mensagem>, ComparaMensagem> mensagens;
std::vector<std::string> convites;

char tabuleiro[3][3];
bool fim_partida = true;



void recebe_mensagens_servidor(ConexaoPtr conexao);

void interacao_usuario(ConexaoPtr conexao);

bool efetuar_login (ConexaoPtr conexao);

void efetuar_logout (ConexaoPtr conexao);

bool efetuar_cadastro (ConexaoPtr conexao);

void listar_jogadores(ConexaoPtr conexao);

void enviar_convite(ConexaoPtr conexao);

void ver_convites(ConexaoPtr conexao);

void joga_jogo(ConexaoPtr conexao);

void comandos_ingame(ConexaoPtr conexao, char simbolo, std::string tabuleiro_inicial);

void imprime_tabuleiro(char tabuleiro[3][3]);

void retoma_jogo(ConexaoPtr conexao, std::string simbolo, std::string matriz);



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


std::vector<std::string> separa_string(std::string const &string) // separa uma string com \n's em várias (uma por linha) 
{
    std::vector<std::string> resultado;
    std::istringstream iss(string);
    std::string token;

    while (std::getline(iss, token, '\n'))
        resultado.push_back(std::move(token));

    return resultado;
}

void recebe_mensagens_servidor(ConexaoPtr conexao) {
    int  n;
    char mensagem_recebida[MAXLINE + 1];
    std::string comando, arg1, arg2;

    while ((n = conexao->recebe_mensagem(mensagem_recebida)) > 0) {
        mensagem_recebida[n] = 0;

        for (auto recvline : separa_string(mensagem_recebida))
        {
            std::regex rgx("([A-Z]*)\\s+(\\w*)(\\s+(\\w*))?(\\s+(\\w*))?(\\s+(\\w*))?");
            std::smatch resultado;
            std::regex_search(recvline, resultado, rgx);
            comando = resultado[1];
            arg1 = resultado[2];

            // Prioridade maior = mais importante  = começo da fila
            // Prioridade menor = menos importante = final da fila
            int prioridade = 0;

            if (comando == "REPLY")
                prioridade = 3;
            else if (comando == "ANSWER") // ANSWER precisa ter mais prioridade do que o START
                prioridade = 2;
            else if (comando == "REQUEST")
                printf ("Novo convite de %s recebido. Responda ao convite no menu principal.\n", arg1.c_str()); // prioridade continua 0
            else { // START etc.
                prioridade = 1; 
            }

            if (comando == "PLAY") {
                int x, y;
                std::string simbolo,result, args1, args2, args3;
                simbolo = resultado[2];
                args1 = resultado[4];
                args2 = resultado[6];
                args3 = resultado[8];
                x = atoi(args1.c_str());
                y = atoi(args2.c_str());
                result = args3;
                tabuleiro[x][y] = simbolo.c_str()[0];


                if (result == "VITORIA") {
                    printf("Você venceu a partida!\n");
                    fim_partida = true;
                } else if (result == "EMPATE") {
                    printf("Você empatou a partida!\n");
                    fim_partida = true;
                } else if (result == "DERROTA") {
                    printf("Você perdeu a partida!\n");
                    fim_partida = true;
                }

                imprime_tabuleiro(tabuleiro);
                printf("Digite \"JOGADA x y\" para realizar uma jogada.\n");

            } else {
                mensagens.push(Mensagem(recvline, prioridade));            
                recebeu_resposta++;
                mensagens_cv.notify_one();
            }
        }
    }
    if (n < 0)
        fprintf(stderr,"read error :(\n");


    exit(EXIT_FAILURE);
}

void interacao_usuario(ConexaoPtr conexao) {

    int opcao = 0;
    bool logado = false;
    bool quer_sair = false;
    std::string aux1, aux2, aux3, output;

    while (!quer_sair){
        while (!logado && !quer_sair) {

            bool error = false;
            do {
                printf ("Antes de executar qualquer ação, você deve estar logado.\n");
                printf ("Digite:\n  1 para fazer login\n  2 para criar um novo usuário\n  3 para sair do programa\n");
                std::cin >> opcao;
                error = std::cin.fail();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                printf("\033[2J\033[;H"); // clear
            } while (error);

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
                printf ("\n=============== MENU PRINCIPAL ===============\n");
                printf ("Digite:\n  1 para listar jogadores conectados\n  2 para enviar convite de jogo\n  3 para ver os convites recebidos\n  4 para logout\n  5 para sair do programa\n");
                std::cin >> opcao;
                error = std::cin.fail();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                printf("\033[2J\033[;H"); // clear
            } while (error);

            switch (opcao) {
                case 1:
                    listar_jogadores(conexao);
                    break; 
                case 2: 
                    enviar_convite(conexao);
                    break;
                case 3: 
                    ver_convites(conexao);
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
        recebeu_resposta--;

        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)(\\s+(\\w*)\\s+(\\w*))?");
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
        else if (arg1 == "003") {
            std::string simbolo = resultado[5];
            std::string matriz = resultado[6];
            retoma_jogo(conexao, simbolo, matriz);
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
    recebeu_resposta--;

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
    printf("Digita a senha: \n");
    std::cin >> aux2;
    printf("Digite novamente a senha: \n");
    std::cin >> aux3;

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
        recebeu_resposta--;

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
        recebeu_resposta--;

        std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
        std::smatch resultado;
        std::regex_search(aux, resultado, rgx);
        std::string comando = resultado[1];
        std::string arg1 = resultado[2];
        std::string arg2 = resultado[3];

        if (arg1 == "030") // começo da lista
        {
            num_usuarios = atoi(arg2.c_str());
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

    while (num_usuarios > 0)
    {
        std::unique_lock<std::mutex> lck(mtx);
        mensagens_cv.wait(lck, respostas_para_receber);

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

        if (arg1 == "031")
        {
            std::string stringaux = arg2 + "\t" + arg3 + "\t" + arg4 + "\n";
            printf("%s", stringaux.c_str());
        }
        else
        {
            mensagens.push(msg);
            printf("Erro inesperado\n");
            return;
        }

        num_usuarios--;
    }
}

void enviar_convite(ConexaoPtr conexao)
{
    std::string oponente;

    printf("Com qual usuário você deseja jogar?\n");
    std::cin >> oponente;
    conexao->envia_mensagem("REQUEST " + oponente);
    printf("Enviando convite para %s...\n", oponente.c_str());

    recebeu_resposta = 0;
    // espera resposta ao convite
    {
        std::unique_lock<std::mutex> lck(mtx);
        mensagens_cv.wait(lck, respostas_para_receber);
    }

        Mensagem msg(mensagens.top());

        std::string aux = msg.conteudo;
        mensagens.pop();

        recebeu_resposta--;        

    std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
    std::smatch resultado;
    std::regex_search(aux, resultado, rgx);
    std::string comando = resultado[1];
    std::string arg1 = resultado[2];
    std::string arg2 = resultado[3];

    if (comando == "ANSWER" && arg2 == oponente)
    {
        if (arg1 == "S")
        {
            printf("Convite aceito. Aguardando início de partida...");
            joga_jogo(conexao);
        }
        else if (arg1 == "N")
        {
            printf("Convite recusado.\n");
        }
        else
        {
            mensagens.push(msg);
            printf("Erro inesperado\n");
            return;
        }

    }
    else if (comando == "REPLY")
    {
        if (arg1 == "041")
        {
            printf("O usuário %s não existe.\n", arg2.c_str());
        }
        else if (arg1 == "042")
        {
            printf("O usuário %s não está conectado.\n", arg2.c_str());
        }
        else if (arg1 == "043")
        {
            printf("O usuário %s já está em jogo.\n", arg2.c_str());
        }
        else if (arg1 == "044")
        {
            printf("Não é possível jogar com você mesmo.\n");
        }
        else
        {
            mensagens.push(msg);
            printf("Erro inesperado\n");
            return;
        }
    }
    else
    {
        mensagens.push(msg);
        printf("Erro inesperado\n");
        return;
    }
}

void ver_convites(ConexaoPtr conexao)
{
    std::queue<Mensagem> mensagens_nao_requests;

    while (!mensagens.empty())
    {
        std::string conteudo = mensagens.top().conteudo;
        if (conteudo.find("REQUEST") == 0) {
            convites.emplace_back(conteudo.substr(8));
        }
        else {
            mensagens_nao_requests.push(mensagens.top());
        }
        mensagens.pop();
        recebeu_resposta--;
    }

    if (convites.empty())
        printf("Você não tem convites pendentes.\n");
    else
    {
        bool quer_sair = false;

        while (!quer_sair)
        {
            unsigned int opcao1;
            bool error;
            unsigned int num_convites = convites.size();
            do
            {
                printf("Você tem convites dos seguintes usuários:\n");
                for (unsigned int i = 0; i < num_convites; i++)
                    printf("%d) %s\n", i, convites[i].c_str());
                printf("Para responder a um convite, digite o número do usuário que te convidou.\nOu digite %d para voltar ao menu anterior.\n", num_convites);

                std::cin >> opcao1;
                error = std::cin.fail();
                std::cin.clear();
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            } while (error);

            if (opcao1 == num_convites)
                quer_sair = true;
            else if (opcao1 >= 0 && opcao1 < num_convites)
            {  
                int opcao2;
                do
                {
                    printf("Digite 1 para aceitar ou 2 para recusar o convite de %s, ou 3 para cancelar a resposta\n", convites[opcao1].c_str());

                    std::cin >> opcao2;
                    error = std::cin.fail();
                    std::cin.clear();
                    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                } while (error);

                std::string oponente = convites[opcao1];

                switch (opcao2)
                {    
                    case 1:
                        convites.erase(convites.begin() + opcao1);
                        num_convites--;
                        printf("Convite aceito. Aguardando início de partida...\n");
                        conexao->envia_mensagem("ANSWER S " + oponente);
                        quer_sair = true;
                        joga_jogo(conexao);
                        // espera início da partida
                        break;
                    case 2:
                        convites.erase(convites.begin() + opcao1);
                        num_convites--;
                        printf ("Convite recusado\n");
                        conexao->envia_mensagem("ANSWER N " + oponente);
                        quer_sair = true;
                        break;
                    case 3:
                        break;
                    default:
                        printf("Comando inválido\n");
                        break;
                }
            }
        }
    }
    while (!mensagens_nao_requests.empty()) {
        mensagens.push(mensagens_nao_requests.front());
        mensagens_nao_requests.pop();
    } 
}

void joga_jogo(ConexaoPtr conexao) {
    char simbolo;

    {
        std::unique_lock<std::mutex> lck(mtx);
        mensagens_cv.wait(lck, respostas_para_receber);
    }
    Mensagem msg(mensagens.top()); 
    std::string aux = msg.conteudo;
    mensagens.pop();
    recebeu_resposta--;

    std::regex rgx("([A-Z]*)\\s+(\\w*)\\s+(\\w*)");
    std::smatch resultado;
    std::regex_search(aux, resultado, rgx);
    std::string comando = resultado[1];
    std::string arg1 = resultado[2];
    std::string arg2 = resultado[3];

    if (comando == "START") {
        if (arg1 == "X") {
            simbolo = arg1.c_str()[0];

        } else if (arg1 == "O") {
            simbolo = arg1.c_str()[0];
        }

        comandos_ingame(conexao, simbolo, " ");
    }
    else printf("Erro inesperado. Você deveria jogar agora.\n");
}

void comandos_ingame(ConexaoPtr conexao, char simbolo, std::string tabuleiro_inicial) {
    std::string entrada;
    fim_partida = false;
    bool error;

    if (tabuleiro_inicial == " ") {
        for (int i = 0; i < 3; i++){
            for (int j = 0; j < 3; j++) {
                tabuleiro[i][j] = ' ';
            }
        }
    } else {
        int z = 0;
        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                char aux;
                if (tabuleiro_inicial[z] == 'X') aux = 'X';
                else if (tabuleiro_inicial[z] == 'O') aux = 'O';
                else if (tabuleiro_inicial[z] == 'N') aux = ' ';
                else aux = ' ';

                tabuleiro[i][j] = aux;
                z++;
            }
        }
    }
    imprime_tabuleiro(tabuleiro);
    do {
        printf("Digite \"JOGADA x y\" para realizar uma jogada.\n");
        std::getline(std::cin, entrada);
        error = std::cin.fail();

        if (entrada.find("JOGADA") == 0) {
            std::regex rgx("([A-Z]*)\\s+(\\w*)(\\s+(\\w*))?");
            std::smatch resultado, resposta;
            std::regex_search(entrada, resultado, rgx);
            std::string arg1 = resultado[2];
            std::string arg2 = resultado[3];
            std::string simbolo_str(1, simbolo);

            std::string output = "PLAY " + arg1 + " " + arg2; 
            conexao->envia_mensagem(output);
            
            std::unique_lock<std::mutex> lck(mtx);
            mensagens_cv.wait(lck, respostas_para_receber);
            
            Mensagem msg(mensagens.top());
            std::string aux = msg.conteudo;
            mensagens.pop();
            recebeu_resposta--;

            std::regex_search(aux, resposta, rgx);
            std::string comando = resposta[1];
            std::string codigo = resposta[2];
            std::string result = resposta[4];

            if (comando == "REPLY") {
                if (codigo == "060") {
                    int x, y;
                    x = atoi(arg1.c_str());
                    y = atoi(arg2.c_str());
                    tabuleiro[x][y] = simbolo;
                    imprime_tabuleiro(tabuleiro);
                    if (result == "VITORIA") {
                        printf("Você venceu a partida!\n");
                        fim_partida = true;
                    } else if (result == "EMPATE") {
                        printf("Você empatou a partida!\n");
                        fim_partida = true;
                    } else if (result == "DERROTA") {
                        printf("Você perdeu a partida!\n");
                        fim_partida = true;
                    } else if (result == "CONTINUA"){

                    } else {
                        printf("Resultado inválido de jogo.\n");
                    }
                } else if (codigo == "061") {
                    printf("Jogada fora dos limites do tabuleiro. Tente outro valor (x e y entre 0 e 2).\n");
                } else if (codigo == "062") {
                    printf("A posição escolhida já encontra-se preenchida. Tente outro valor.\n");
                } else if (codigo == "063") {
                    printf("Jogada inválida. O turno é do oponente.\n");
                } else {

                }
            } 
        } else {
            printf("Comando inválido. A palavra inicial deve ser ou \"JOGADA\" ou \"MSG\".\n");
        }
    } while (error || !fim_partida);

}

void imprime_tabuleiro(char tabuleiro[3][3]) {
    int i = 0;

    printf("= = = = Tabuleiro = = = =\n");
    for (i = 0; i < 2; i++) {
        printf(" %c | %c | %c \n", tabuleiro[i][0], tabuleiro[i][1], tabuleiro[i][2]);
        printf("___________\n");
    }
    printf(" %c | %c | %c \n", tabuleiro[i][0], tabuleiro[i][1], tabuleiro[i][2]);
    printf("= = = = = = = = = = = = =\n");
}

void retoma_jogo(ConexaoPtr conexao, std::string simbolo, std::string matriz) {
    printf("Retomando jogo!\n");
    comandos_ingame(conexao, simbolo.c_str()[0], matriz);
}