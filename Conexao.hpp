#ifndef _CONEXAO_HPP
#define _CONEXAO_HPP

#include <memory>
#include <string>

using std::string;

enum TipoConexao {TCP, UDP};

class Conexao {
  public:
    virtual void envia_mensagem(string mensagem) = 0;
    virtual int recebe_mensagem(char* recvline) = 0;
    virtual TipoConexao tipo_conexao() = 0;
};

using ConexaoPtr = std::shared_ptr<Conexao>;

class ConexaoTCP : public Conexao {
  public:
    ConexaoTCP(int connfd) : connfd_(connfd) {}
    ~ConexaoTCP() {}
    void envia_mensagem(string mensagem) override;
    int recebe_mensagem(char* recvline) override;
    TipoConexao tipo_conexao() override;
  private:
    int connfd_;
};


#endif // _CONEXAO_HPP