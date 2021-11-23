# Multithreaded-Network-Calculator
Calculator server that listens for client TCP connections, reads a sequence of expressions, and evaluates each expression.
Program takes a single command line argument specifiying TCP port. The server program listens for incoming connections on the specified port, and then communicates with the client in the same way that the calcInteractive program does. Chat_with_client function updated to use in server implementation. Recognizes special quit command (which causes the server to end the session with the currently-connected client), and server chat_with_client function salso recognizes a shutdown command, which causes the server process to exit.

Multiple threads are used to handle client connections.
Synchronization is used to protect shared data so that expression evaluations are atomic.

