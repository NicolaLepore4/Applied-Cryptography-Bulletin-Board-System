CXXFLAGS = -std=c++17 -lcrypto
# -Wall -Wextra -Werror -Wno-sign-compare -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable -Wno-unused-function -Wno-unused-but-set-variable -Wno-unused-local-typedefs -Wno-unused-value -Wno-unused-local-typedefs

CLIENT = clientHandler.cpp
CLIENT_EXE = client.out

client: $(CLIENT)
	echo "Compiling client ..."
	g++ $(CLIENT) -o $(CLIENT_EXE) $(CXXFLAGS)
	echo "Client compiled."
	$(MAKE) run-client

run-client:
	if [ -f $(CLIENT_EXE) ]; then ./$(CLIENT_EXE); else echo "Client not compiled."; fi

clean:	
	clear
	if [ -f $(CLIENT_EXE) ]; then rm $(CLIENT_EXE); fi
	echo "All files are cleaned."