
all: bankingClient bankingServer

bankingClient: 
	gcc bankingClient.c -o bankingClient
bankingServer: 
	gcc -lpthread bankingServer.c -o bankingServer

clean: 
	rm -rf bankingClient bankingServer

     
