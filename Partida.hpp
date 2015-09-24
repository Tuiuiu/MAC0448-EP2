#ifndef _PARTIDA_HPP
#define _PARTIDA_HPP

#include "Usuario.hpp"

enum jogada    {VALIDA, COMANDO_INVALIDO, POSICAO_OCUPADA, POSICAO_INEXISTENTE, AGUARDE_SUA_VEZ};
enum resultado {VITORIA_X, VITORIA_O, VELHA, NAO_ACABOU};


class Partida {
  public:
  	jogada 	  fazJogada(int x, int y, char comando);
  	resultado verificaResultado();
  	void 	  imprimeTabuleiro();
  	Partida();
  private:
  	char tabuleiro[3][3]; // Se for um espaço, está vazio, caso contrário, 
  						  // haverá um X ou O no espaço correspondente
  	char ultimoJogador;
  	/*Usuario jogadorX;
  	Usuario jogadorY;*/
};

#endif // _PARTIDA_HPP