#ifndef _CONEXAO_HPP
#define _CONEXAO_HPP

#include <string>

class Conexao {
  public:
    virtual void enviaMensagem(std::string mensagem) = 0;
};

class ConexaoTCP : public Conexao {
  public:
    ConexaoTCP(int connfd) : connfd_(connfd) {}
    void enviaMensagem(std::string mensagem);
  private:
    int connfd_;
};


#endif // _CONEXAO_HPP