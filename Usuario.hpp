#ifndef _USUARIO_HPP
#define _USUARIO_HPP

#include <string>

#include "Conexao.hpp"

class Usuario {
  public:
  	Usuario(Conexao *conexao) : conexao(conexao) {}
  	void escreve(std::string mensagem);
  private:
  	Conexao *conexao;
  	std::string login;
  	std::string senha;
};

#endif // _USUARIO_HPP
