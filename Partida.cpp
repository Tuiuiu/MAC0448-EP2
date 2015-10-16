#include <cstdio>

#include "Partida.hpp"

Jogada Partida::fazJogada(int x, int y, char comando) {
	if (x >= 0 && x <= 2 && y >= 0 && y <= 2) {
		if (tabuleiro[x][y] == ' ') {
			if(comando == 'X' || comando == 'O') {
				if (comando == ultimoJogador)
					return AGUARDE_SUA_VEZ;
				else {
					tabuleiro[x][y] = comando;
					ultimoJogador = comando;
					return VALIDA;
				}
			}
			else {
				printf("Comando inválido. Deve ser 'X' ou 'O'\n");
				return COMANDO_INVALIDO;
			} 
		}
		else {
			printf("Posição do tabuleiro já está preenchida.\n");
			return POSICAO_OCUPADA;
		}
	}
	else {
		printf("Posição escolhida está fora do tabuleiro. X e Y devem estar entre 0 e 2\n");
		return POSICAO_INEXISTENTE;
	}
}

Resultado Partida::verificaResultado() {
	for (int i = 0; i <= 2; i++) {
		if (tabuleiro[i][0] == 'X' && tabuleiro[i][1] == 'X' && tabuleiro[i][2] == 'X')
			return VITORIA_X;
		else if (tabuleiro[i][0] == 'O' && tabuleiro[i][1] == 'O' && tabuleiro[i][2] == 'O')
			return VITORIA_O;
		else if (tabuleiro[0][i] == 'X' && tabuleiro[1][i] == 'X' && tabuleiro[2][i] == 'X')
			return VITORIA_X;
		else if (tabuleiro[0][i] == 'O' && tabuleiro[1][i] == 'O' && tabuleiro[2][i] == 'O')
			return VITORIA_O;
	}
	if ((tabuleiro[0][0] == 'X' && tabuleiro[1][1] == 'X' && tabuleiro[2][2] == 'X') 
	 || (tabuleiro[0][2] == 'X' && tabuleiro[1][1] == 'X' && tabuleiro[2][0] == 'X'))
		return VITORIA_X;
	else if ((tabuleiro[0][0] == 'O' && tabuleiro[1][1] == 'O' && tabuleiro[2][2] == 'O') 
	 	  || (tabuleiro[0][2] == 'O' && tabuleiro[1][1] == 'O' && tabuleiro[2][0] == 'O'))
		return VITORIA_O;
	else {

		for (int i = 0; i <= 2; i++)
			for (int j = 0; j <= 2; j++) {
				if (tabuleiro[i][j] == ' ')
					return NAO_ACABOU;
			}
		return VELHA;
	}
}

Partida::Partida(UsuarioPtr jogadorX, UsuarioPtr jogadorO) : ultimoJogador(' '), jogadorX(jogadorX), jogadorO(jogadorO) {
	for (int i = 0; i <= 2; i++) {
		for (int j = 0; j <= 2; j++)
			tabuleiro[i][j] = ' ';
	}


}

void Partida::imprimeTabuleiro() {
	for(int i = 0; i <= 2; i++) {
		printf(" %c | %c | %c \n", tabuleiro[i][0], tabuleiro[i][1], tabuleiro[i][2]);
	}
}

char Partida::simbolo(UsuarioPtr usuario)
{
	if (usuario == jogadorX)
		return 'X';
	if (usuario == jogadorO)
		return 'O';
	return '?';
}

char Partida::get_simbolo_ultimo_jogador()
{
	return ultimoJogador;
}


std::string Partida::tabuleiro_em_string()
{
	std::string resultado = "";

	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
		{
			if (tabuleiro[i][j] == ' ')
				resultado += 'N';
			else
				resultado += tabuleiro[i][j];
		}

	return resultado;
}

UsuarioPtr Partida::get_jogador_X()
{
	return jogadorX;
}

UsuarioPtr Partida::get_jogador_O()
{
	return jogadorO;
}