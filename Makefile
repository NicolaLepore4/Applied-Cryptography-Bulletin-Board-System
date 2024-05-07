CXXFLAGS = -std=c++17 -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-local-typedefs -Wno-unused-value -Wno-unused-local-typedefs
SERVER = server/server.cpp
SERVER_EXE = server/server.out

CLIENT = client/client.cpp
CLIENT_EXE = client/client.out

server: $(SERVER) run-server
	echo "Compiling server ..."
	g++ $(SERVER) -o $(SERVER_EXE) $(CXXFLAGS)
	echo "Server compiled."
	$(MAKE) run-server;
	

client: $(CLIENT) run-client
	echo "Compiling client ..."
	g++ $(CLIENT) -o $(CLIENT_EXE) $(CXXFLAGS)
	echo "Client compiled."
	$(MAKE) run-client;

run-server:
	if [ -f $(SERVER_EXE) ]; then ./$(SERVER_EXE); else echo "Server not compiled."; fi

run-client:
	if [ -f $(CLIENT_EXE) ]; then ./$(CLIENT_EXE); else echo "Client not compiled."; fi
clean:	
	clear
	if [ -f $(SERVER_EXE) ]; then rm $(SERVER_EXE); fi
	if [ -f $(CLIENT_EXE) ]; then rm $(CLIENT_EXE); fi
	echo "All files are cleaned."