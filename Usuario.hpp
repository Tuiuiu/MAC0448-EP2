#ifndef _USUARIO_HPP
#define _USUARIO_HPP

#include <string>

#include "Conexao.hpp"

class Partida;
using PartidaPtr = std::shared_ptr<Partida>;

class Usuario {
  public:
  	Usuario(ConexaoPtr conexao, std::string login_arg, std::string senha_arg) 
  		   : conexao(conexao),conectado(true),em_jogo(false) 
  		   { login = login_arg; senha = senha_arg; set_hora_ultima_conexao(); }
  	void escreve(std::string mensagem);
  	void atualiza_conexao(ConexaoPtr novaConexao);
  	bool esta_conectado();
  	bool esta_em_jogo();
  	void conecta();
  	void desconecta();
  	void entra_jogo();
  	void sai_jogo();
  	std::string get_login();
  	bool confere_senha(std::string senha_arg);
  	std::string get_hora_ultima_conexao();
    PartidaPtr get_partida();
    void set_partida(PartidaPtr nova_partida);

  private:
  	ConexaoPtr conexao;
  	std::string login;
  	std::string senha;
  	bool conectado;
  	bool em_jogo;
  	time_t hora_ultima_conexao;
    PartidaPtr partida;
  	
  	void set_hora_ultima_conexao();
};

using UsuarioPtr = std::shared_ptr<Usuario>;

#endif // _USUARIO_HPP
