#include "Usuario.hpp"

void Usuario::escreveParaUsuario(std::string mensagem) {
	conexao->enviaMensagem(mensagem);
}