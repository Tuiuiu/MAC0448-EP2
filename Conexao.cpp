#include <unistd.h>

#include "Conexao.hpp"

#define MAXLINE 100

using std::string;

void ConexaoTCP::envia_mensagem(string mensagem) {
  write(connfd_, mensagem.c_str(), mensagem.length());
}

int ConexaoTCP::recebe_mensagem(char* recvline) {
	return read(connfd_, recvline, MAXLINE);
}

TipoConexao ConexaoTCP::tipo_conexao() {
	return TCP;
}