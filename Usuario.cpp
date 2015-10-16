#include "Usuario.hpp"

void Usuario::escreve(std::string mensagem) {
	conexao->envia_mensagem(mensagem);
}

void Usuario::atualiza_conexao(ConexaoPtr novaConexao) {
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
	partida = nullptr;
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

PartidaPtr Usuario::get_partida()
{
	return partida;
}

void Usuario::set_partida(PartidaPtr nova_partida)
{
	partida = nova_partida;
	em_jogo = true;
}

UsuarioPtr Usuario::adversario()
{
	if (!em_jogo)
		return nullptr;
	else
	{
		if (partida->get_jogador_X() == shared_from_this())
			return partida->get_jogador_O();
		else if (partida->get_jogador_O() == shared_from_this())
			return partida->get_jogador_X();
		else
		{
			printf ("Erro inesperado\n");
			return nullptr;
		}
	}
}

char Usuario::simbolo()
{
	if (esta_em_jogo())
		return partida->simbolo(shared_from_this());
	else
		return '?';
}