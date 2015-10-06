#ifndef _CONEXAO_HPP
#define _CONEXAO_HPP

#include <string>

using std::string;

enum TipoConexao {TCP, UDP};

class Conexao {
  public:
    virtual void envia_mensagem(string mensagem) = 0;
    virtual int recebe_mensagem(char* recvline) = 0;
    virtual TipoConexao tipo_conexao() = 0;
};

class ConexaoTCP : public Conexao {
  public:
    ConexaoTCP(int connfd) : connfd_(connfd) {}
    ~ConexaoTCP() {}
    void envia_mensagem(string mensagem);
    int recebe_mensagem(char* recvline);
    TipoConexao tipo_conexao();
  private:
    int connfd_;
};


#endif // _CONEXAO_HPP