#ifndef _PARTIDA_HPP
#define _PARTIDA_HPP

#include <memory>

#include "Usuario.hpp"

enum jogada    {VALIDA, COMANDO_INVALIDO, POSICAO_OCUPADA, POSICAO_INEXISTENTE, AGUARDE_SUA_VEZ};
enum resultado {VITORIA_X, VITORIA_O, VELHA, NAO_ACABOU};


class Partida {
  public:
  	jogada 	  fazJogada(int x, int y, char comando);
  	resultado verificaResultado();
  	void 	  imprimeTabuleiro();
  	Partida(UsuarioPtr jogadorX, UsuarioPtr jogadorY);
  private:
  	char tabuleiro[3][3]; // Se for um espaço, está vazio, caso contrário, 
  						  // haverá um X ou O no espaço correspondente
  	char ultimoJogador;
  	UsuarioPtr jogadorX;
  	UsuarioPtr jogadorY;
};

using PartidaPtr = std::shared_ptr<Partida>;

#endif // _PARTIDA_HPP