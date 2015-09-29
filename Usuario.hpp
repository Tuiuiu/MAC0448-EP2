#include <string>

#include "Conexao.hpp"

class Usuario {
  public:
  	Usuario(Conexao *conexao) : conexao(conexao) {}
  	void escreveParaUsuario(std::string mensagem);
  private:
  	Conexao *conexao;
  	// std::string login;
  	// std::string senha;
};
