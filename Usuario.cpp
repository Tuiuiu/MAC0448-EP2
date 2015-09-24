#include "Usuario.hpp"

void Usuario::escreve(std::string mensagem) {
	conexao->enviaMensagem(mensagem);
}