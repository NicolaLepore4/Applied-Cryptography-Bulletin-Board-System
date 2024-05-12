CXXFLAGS = -std=c++17 -lcrypto
# -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-local-typedefs -Wno-unused-value -Wno-unused-local-typedefs
SERVER = server.cpp
SERVER_EXE = server.out

server: $(SERVER) run-server
	echo "Compiling server ..."
	g++ $(SERVER) -o $(SERVER_EXE) $(CXXFLAGS)
	echo "Server compiled."
	$(MAKE) run-server;
	
run-server:
	if [ -f $(SERVER_EXE) ]; then ./$(SERVER_EXE); else echo "Server not compiled."; fi

clean:	
	clear
	if [ -f $(SERVER_EXE) ]; then rm $(SERVER_EXE); fi
	echo "All files are cleaned."