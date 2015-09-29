#include "Usuario.hpp"

void Usuario::escreve(std::string mensagem) {
	conexao->enviaMensagem(mensagem);
}

void Usuario::atualizaConexao(Conexao *novaConexao) {
	free (conexao);
	conexao = novaConexao;
}

void Usuario::conecta() {
	conectado = true;
}

void Usuario::desconecta() {
	conectado = false;
}

void Usuario::entra_jogo() {
	em_jogo = true;
}

void Usuario::sai_jogo() {
	em_jogo = false;
}

std::string Usuario::get_login() {
	return login;
}

bool Usuario::confere_senha(std::string senha_arg) {
	return senha_arg == senha;
}
