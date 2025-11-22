#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <chrono>
#include <thread>

#define PORT 8080
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
#define sadr struct sockaddr_in

int main() {
    char buffer[BUFFER_SIZE];
    
    // 1. СОЗДАЕМ СОКЕТ ОДИН РАЗ
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    sadr server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("invalid address");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    
    std::string name;
    std::cout << "Введите ваше имя: \n";
    std::cin >> name;
    // 2. ПОДКЛЮЧАЕМСЯ К СЕРВЕРУ ОДИН РАЗ
    std::cout << "Подключаемся к серверу..." << std::endl;
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    std::cout << "✅ Успешно подключились к серверу!" << std::endl;

    // 3. ОБЩАЕМСЯ ПО ЭТОМУ СОЕДИНЕНИЮ
    while(1) {
        std::string str;
        std::cout << "Введите сообщение:\n";
        std::cin >> str;

        
        std::string msg = (name + ": " + str.c_str() + "\n");
        // Отправляем сообщение
        if (send(sockfd, msg.c_str(), msg.size(), 0) < 0) {
            perror("send failed");
            break;  // Не выходим из программы, просто прерываем цикл
        }

        // Получаем ответ
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = read(sockfd, buffer, BUFFER_SIZE - 1);
        if (bytes_received > 0) {
            std::cout << "From server: " << buffer << std::endl;
        } else if (bytes_received == 0) {
            std::cout << "Сервер закрыл соединение" << std::endl;
            break;
        } else {
            perror("read error");
            break;
        }
    }
    
    // 4. ЗАКРЫВАЕМ СОЕДИНЕНИЕ ТОЛЬКО КОГДА ЗАКОНЧИЛИ
    close(sockfd);
    std::cout << "Соединение закрыто" << std::endl;
    return 0;
}