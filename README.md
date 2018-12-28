# TCPbank

HOW TO USE

In c terminal:
-"make all" to make bankingServer and bankingClient.
-"make bankingServer" for bankingServer. 
-"make bankingClient" for bankingClient.
-"make clean" to clean bankingServer and bankingClient.

-"./bankingServer <PORT>" 
	to start server. Make sure <PORT> is greater than 8000.
-"./bankingClient <MACHINE> <PORT>"
        to start client. Make sure <MACHINE> AND <PORT> is the location of the server.

======================================================================================
DESIGN
  - Everytime there is a new client, a new thread is created.
  - All account information are stored in a global hashtable "bank".
  - All accounts that are created and are printed for system diagonstic are placed in a global array "allAccounts".
  - A binary semaphore "sem" is initized and used everytime "bank" is used (either for "create","serve","deposit"...etc).
  - Every 15 seconds, "allAccounts" is printed out for the system diagonstic. The semaphore is used to lock all the accounts.
  - When a client enters "quit", their thread in the server is join()'ed.
