#ifndef _PARTIDA_HPP
#define _PARTIDA_HPP

#include <memory>

#include "Usuario.hpp"

enum Jogada    {VALIDA, COMANDO_INVALIDO, POSICAO_OCUPADA, POSICAO_INEXISTENTE, AGUARDE_SUA_VEZ};
enum Resultado {VITORIA_X, VITORIA_O, VELHA, NAO_ACABOU};


class Partida {
  public:
  	Jogada 	  fazJogada(int x, int y, char comando);
  	Resultado verificaResultado();
  	void 	  imprimeTabuleiro();
  	Partida(UsuarioPtr jogadorX, UsuarioPtr jogadorO);
    char simbolo(UsuarioPtr usuario);
    char get_simbolo_ultimo_jogador();
    std::string tabuleiro_em_string();
    //UsuarioPtr get_ultimo_jogador();
    UsuarioPtr get_jogador_X();
    UsuarioPtr get_jogador_O();

  private:
  	char tabuleiro[3][3]; // Se for um espaço, está vazio, caso contrário, 
  						  // haverá um X ou O no espaço correspondente
  	char ultimoJogador;
  	UsuarioPtr jogadorX;
  	UsuarioPtr jogadorO;
};

using PartidaPtr = std::shared_ptr<Partida>;

#endif // _PARTIDA_HPP