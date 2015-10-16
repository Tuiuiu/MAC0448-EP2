CXXFLAGS = -ansi -Wall -pedantic -std=c++11
LDFLAGS = -pthread

CC = g++

servidor: servidor.o Partida.o Usuario.o Conexao.o

cliente: cliente.o Conexao.o

cliente.o: Conexao.hpp

servidor.o: Partida.hpp Usuario.hpp Conexao.hpp

Partida.o: Partida.hpp Usuario.hpp Conexao.hpp

Usuario.o: Usuario.hpp Conexao.hpp

Conexao.o: Conexao.hpp

.PHONY clean:
	rm *.o servidor cliente
