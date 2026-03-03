#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#ifndef PORT
#define PORT (80)
#endif


#ifndef BUFFER_SIZE
#define BUFFER_SIZE (4 * 1024 * 1024)
#endif

#ifndef READ_BUFFER_SIZE
#define READ_BUFFER_SIZE (4096)
#endif

const int port = PORT;
const size_t buffer_size = BUFFER_SIZE;
const size_t read_buffer_size = READ_BUFFER_SIZE;

// Ignore SIGPIPE to handle broken pipes gracefully
void ignore_sigpipe() {
    signal(SIGPIPE, SIG_IGN);
}

// Function to handle a single client connection
void handle_client(int client_sock) {
    char read_buf[read_buffer_size];
    bool headers_read = false;
    std::string request;

    // Minimal HTTP request parsing: read until we have headers (\r\n\r\n)
    while (!headers_read) {
        ssize_t bytes_read = recv(client_sock, read_buf, read_buffer_size, 0);
        if (bytes_read <= 0) {
            close(client_sock);
            return;
        }
        request.append(read_buf, bytes_read);

        // Check for end of headers
        size_t pos = request.find("\r\n\r\n");
        if (pos != std::string::npos) {
            headers_read = true;
            // We ignore the request details; assume it's a GET
        }
    }

    // Send response headers
    const char* headers = "HTTP/1.1 200 OK\r\n"
                          "Content-Type: application/octet-stream\r\n"
                          "Cache-Control: no-store, no-cache, must-revalidate\r\n"
                          "\r\n";
    send(client_sock, headers, strlen(headers), 0);

    // Prepare zero buffer
    static const char zero_buf[buffer_size]{};
    //memset(zero_buf, 0, buffer_size);

    // Infinite loop sending zeros
    while (true) {
        ssize_t bytes_sent = send(client_sock, zero_buf, buffer_size, 0);
        if (bytes_sent <= 0) {
            break;  // Client disconnected or error
        }
    }

    close(client_sock);
}

int main() {
    ignore_sigpipe();

    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        std::cerr << "Failed to create socket" << std::endl;
        return 1;
    }

    // Allow address reuse
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_sock, (sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Failed to bind to port " << port << std::endl;
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, SOMAXCONN) == -1) {
        std::cerr << "Failed to listen" << std::endl;
        close(server_sock);
        return 1;
    }

    std::cout << "Serving infinite zeros on 0.0.0.0:"<< port <<"..." << std::endl;

    std::vector<std::thread> threads;

    while (true) {
        int client_sock = accept(server_sock, nullptr, nullptr);
        if (client_sock == -1) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Spawn a thread for each client
        threads.emplace_back(handle_client, client_sock);

    }

    // Cleanup (unreachable in infinite loop, but for completeness)
    for (auto& t : threads) {
        if (t.joinable()) t.join();
    }
    close(server_sock);
    return 0;
}
