#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 9000
#define DATA_FILE "/var/tmp/aesdsocketdata"
#define BUFFER_SIZE 1024

int server_fd = -1;
int client_fd = -1;

void handle_signal(int sig) {
    syslog(LOG_INFO, "Caught signal, exiting");
    if (client_fd != -1) close(client_fd);
    if (server_fd != -1) close(server_fd);
    remove(DATA_FILE);
    closelog();
    exit(0);
}

int main(int argc, char *argv[]) {
    bool daemon_mode = false;
    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        daemon_mode = true;
    }

    openlog("aesdsocket", LOG_PID, LOG_USER);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        syslog(LOG_ERR, "Socket creation failed: %s", strerror(errno));
        return -1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        syslog(LOG_ERR, "Bind failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    if (daemon_mode) {
        pid_t pid = fork();
        if (pid < 0) return -1;
        if (pid > 0) exit(0);
        setsid();
        chdir("/");
        int dev_null = open("/dev/null", O_RDWR);
        dup2(dev_null, STDIN_FILENO);
        dup2(dev_null, STDOUT_FILENO);
        dup2(dev_null, STDERR_FILENO);
        close(dev_null);
    }

    if (listen(server_fd, 10) < 0) {
        syslog(LOG_ERR, "Listen failed: %s", strerror(errno));
        close(server_fd);
        return -1;
    }

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t addrlen = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) continue;

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, sizeof(client_ip));
        syslog(LOG_INFO, "Accepted connection from %s", client_ip);

        int file_fd = open(DATA_FILE, O_RDWR | O_CREAT | O_APPEND, 0644);
        char buffer[BUFFER_SIZE];
        ssize_t bytes_received;

        while ((bytes_received = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
            write(file_fd, buffer, bytes_received);
            if (memchr(buffer, '\n', bytes_received)) break;
        }

        lseek(file_fd, 0, SEEK_SET);
        while ((bytes_received = read(file_fd, buffer, sizeof(buffer))) > 0) {
            send(client_fd, buffer, bytes_received, 0);
        }

        close(file_fd);
        close(client_fd);
        client_fd = -1;
        syslog(LOG_INFO, "Closed connection from %s", client_ip);
    }

    return 0;
}