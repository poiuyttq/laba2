#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <unordered_map>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    char buffer[BUFFER_SIZE];
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Разрешаем повторное использование порта
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Сервер работает на 0.0.0.0:" << PORT << std::endl;
 

    
    while (1) {
        sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        // 1. ПРИНИМАЕМ СОЕДИНЕНИЕ
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }
        
        std::cout << "✅ Новый клиент подключился: " 
                  << inet_ntoa(client_addr.sin_addr) << ":" 
                  << ntohs(client_addr.sin_port) << std::endl;

        // 2. РАБОТАЕМ С ЭТИМ КЛИЕНТОМ ПОКА ОН НЕ ОТКЛЮЧИТСЯ
        while (1) {
            memset(buffer, 0, BUFFER_SIZE);
            int bytes_read = read(client_fd, buffer, BUFFER_SIZE - 1);

            if (bytes_read > 0) {
                std::cout << "From client " << inet_ntoa(client_addr.sin_addr) 
                          << ": " << buffer << std::endl;
                
                char response[BUFFER_SIZE] = "Hello from server!";
                if (send(client_fd, response, strlen(response), 0) < 0) {
                    perror("send failed");
                    break;
                }
            } else if (bytes_read == 0) {
                std::cout << "Клиент отключился" << std::endl;
                break;
            } else {
                perror("read error");
                break;
            }
        }
        
        // 3. ЗАКРЫВАЕМ СОЕДИНЕНИЕ С КЛИЕНТОМ
        close(client_fd);
        std::cout << "Соединение с клиентом закрыто" << std::endl;
    }
    
    close(server_fd);
    return 0;
}