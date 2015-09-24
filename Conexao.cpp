#include <unistd.h>

#include "Conexao.hpp"

void ConexaoTCP::enviaMensagem(std::string mensagem) {
  write(connfd_, mensagem.c_str(), mensagem.length());
}
