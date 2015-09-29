#ifndef _CONEXAO_HPP
#define _CONEXAO_HPP

#include <string>

using std::string;

enum TipoConexao {TCP, UDP};

class Conexao {
  public:
    virtual void enviaMensagem(string mensagem) = 0;
    virtual TipoConexao tipoConexao() = 0;
};

class ConexaoTCP : public Conexao {
  public:
    ConexaoTCP(int connfd) : connfd_(connfd) {}
    void enviaMensagem(string mensagem);
    TipoConexao tipoConexao();
  private:
    int connfd_;
};


#endif // _CONEXAO_HPP