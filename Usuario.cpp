#include "Usuario.hpp"

void Usuario::escreve(std::string mensagem) {
	conexao->enviaMensagem(mensagem);
}

void Usuario::atualizaConexao(Conexao *novaConexao) {
	free (conexao);
	conexao = novaConexao;
	set_hora_ultima_conexao();
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

bool Usuario::esta_conectado() {
	return conectado;
}

bool Usuario::esta_em_jogo() {
	return em_jogo;
}

std::string Usuario::get_hora_ultima_conexao()
{
	struct tm* timeinfo;
	char buffer[80];

	timeinfo = localtime (&hora_ultima_conexao);
	strftime (buffer, 80, "%T", timeinfo);

	return std::string (buffer);
}

void Usuario::set_hora_ultima_conexao()
{
  	time (&hora_ultima_conexao);
}