#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 8888;
const int BUFFER_SIZE = 1024;

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // Создание сокета
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Указание адреса сервера
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Бесконечный цикл для отправки сообщений
    char buffer[BUFFER_SIZE];
    while (true) {
        // Ввод сообщения
        std::cout << "Enter message (type 'exit' to quit; type 'HELP' to show command list): ";
        std::cin.getline(buffer, BUFFER_SIZE);

        // Проверка на команду выхода
        if (strcmp(buffer, "exit") == 0) {
            break;
        }

        // Отправка сообщения серверу
        sendto(sockfd, buffer, strlen(buffer), 0, (sockaddr*)&servaddr, sizeof(servaddr));

        // Получение ответа от сервера
        char received_buffer[BUFFER_SIZE];
        int len = sizeof(servaddr);
        int n = recvfrom(sockfd, received_buffer, BUFFER_SIZE, 0, (sockaddr*)&servaddr, &len);
        if (n == SOCKET_ERROR) {
            std::cerr << "recvfrom failed: " << WSAGetLastError() << std::endl;
        } else {
            received_buffer[n] = '\0';
            std::cout << "Server: " << received_buffer << std::endl;
        }
    }

    // Закрытие сокета и очистка Winsock
    closesocket(sockfd);
    WSACleanup();
    return 0;
}
