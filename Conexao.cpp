#include <unistd.h>

#include "Conexao.hpp"

using std::string;

void ConexaoTCP::enviaMensagem(string mensagem) {
  write(connfd_, mensagem.c_str(), mensagem.length());
}

TipoConexao ConexaoTCP::tipoConexao() {
	return TCP;
}
