#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define BACKLOG_SIZE 10
#define SERVER_CONN_READ_BUFFER 30000

static volatile sig_atomic_t shouldExit = 0;
static int serverSocketFD;

static void signalHandler(int signal) {
    printf("received signal: %d\n", signal);
    shouldExit = 1;
    if (shutdown(serverSocketFD, SHUT_RDWR) < 0 ) {
        perror("failed to shutdown server listening socket");
    }
    close(serverSocketFD);
}

int main() {
    // start signal catch to ensure proper cleanup
    if (signal(SIGINT, signalHandler) == SIG_ERR) {
        perror("failed to attach signal handler");
        exit(EXIT_FAILURE);
    }
    puts("starting server...");
    serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFD < 0) {
        perror("failed to create socket");
        exit(EXIT_FAILURE);
    }

    // using the {} is the same as doing
    //      memset(&address, 0, sizeof address);
    // if we had a pointer instead, we could do this
    //      *address = (sockaddr_in){0}
    struct sockaddr_in address = {
            .sin_family = AF_INET,

            // htonl converts a long integer (e.g. address) to a network representation
            .sin_addr = htonl(INADDR_ANY),

            // htons converts a short integer (e.g. port) to a network representation
            .sin_port = htons(PORT),
    };

    if (bind(serverSocketFD, (struct sockaddr *)&address, sizeof address) < 0) {
        perror("failed to bind address");
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocketFD, BACKLOG_SIZE) < 0) {
        perror("failed to listen on socket");
        exit(EXIT_FAILURE);
    }

    while (!shouldExit) {
        puts("\nWaiting for new connection...");

        struct sockaddr_in incomingAddress = {};
        socklen_t incomingAddressLength = {0};

        int newSocket = accept(serverSocketFD, (struct sockaddr*)&incomingAddress, &incomingAddressLength);
        if (newSocket < 0 ) {
            perror("failed to accept connection");
            exit(EXIT_FAILURE);
        }

        char buffer[SERVER_CONN_READ_BUFFER] = {0};
        long valuesRead = read(newSocket, buffer, SERVER_CONN_READ_BUFFER);
        if (valuesRead < 0) {
            perror("failed read from connecting socket");
            close(newSocket);
            continue;
        }
        printf("READ %ld bytes:\n%s\n", valuesRead, buffer);

        char *res =
                "HTTP/1.1 200 OK\n"
                "Content-Type: text/plain\n"
                "Content-Length: 18\n" // you would want to make this dynamic, but this is just for testing
                "Server: scratch-c-server\n"
                "\n"
                "Hello from server!"; // if this line is updated, need to update content length. should use dynamic string instead

        ssize_t bytesWritten = write(newSocket, res, strlen(res));
        if (bytesWritten < 0) {
            perror("failed to write to connecting socket");
            close(newSocket);
            continue;
        }
        printf("WRITE %zdB\n", bytesWritten);

        close(newSocket);
    }

    puts("closing server...");
    return EXIT_SUCCESS;
}
