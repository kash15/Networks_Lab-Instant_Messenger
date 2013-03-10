
all:Server/ims Client/imc

#Server

Server/ims: Server/server.o 
	g++ Server/server.o -o Server/ims

Server/server.o: Server/server.cpp Server/message.h
	g++ -c Server/server.cpp -o Server/server.o


#Client

Client/imc: Client/client.o
	g++ Client/client.o -o Client/imc

Client/client.o:Client/client.cpp Client/message.h
	g++ -c Client/client.cpp -o Client/client.o

clean:
	rm -rf Server/*.o Client/*.o Server/ims Client/imc Server/*~ Client/*~ *~
