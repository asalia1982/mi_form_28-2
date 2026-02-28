// server.cpp (multiplataforma: Windows + Linux/Docker)

#include <iostream>
#include <string>
#include <limits>
#include <sstream>

struct Persona {
    std::string nombre;
    std::string dni;
    int edad;
    std::string telefono;
    std::string direccion;
};

static void capturarPersona(Persona& p) {
    std::cout << "=== FORMULARIO DE REGISTRO (SERVER) ===\n\n";

    std::cout << "Nombre: ";
    std::getline(std::cin, p.nombre);

    std::cout << "DNI: ";
    std::getline(std::cin, p.dni);

    std::cout << "Edad: ";
    while (!(std::cin >> p.edad)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Edad invalida. Ingrese un numero: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "Telefono: ";
    std::getline(std::cin, p.telefono);

    std::cout << "Direccion: ";
    std::getline(std::cin, p.direccion);
}

static std::string generarReporte(const Persona& p, const char* NL) {
    std::ostringstream out;
    out << "====================================" << NL;
    out << "        REPORTE DE PERSONA" << NL;
    out << "====================================" << NL;
    out << "Nombre     : " << p.nombre << NL;
    out << "DNI        : " << p.dni << NL;
    out << "Edad       : " << p.edad << NL;
    out << "Telefono   : " << p.telefono << NL;
    out << "Direccion  : " << p.direccion << NL;
    out << "====================================" << NL;
    return out.str();
}

#ifdef _WIN32
// ---------------- WINDOWS ----------------
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0601
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc >= 2) port = std::stoi(argv[1]);

    WSADATA wsaData{};
    int wsa = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (wsa != 0) {
        std::cerr << "WSAStartup fallo. Codigo: " << wsa << "\n";
        return 1;
    }

    SOCKET server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        std::cerr << "socket() fallo. WSAGetLastError: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    BOOL opt = TRUE;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((u_short)port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "bind() fallo. WSAGetLastError: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    if (listen(server_fd, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "listen() fallo. WSAGetLastError: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Servidor escuchando en el puerto " << port << "...\n";
    std::cout << "Esperando conexion de un cliente...\n\n";

    sockaddr_in client_addr{};
    int client_len = sizeof(client_addr);
    SOCKET client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);

    if (client_fd == INVALID_SOCKET) {
        std::cerr << "accept() fallo. WSAGetLastError: " << WSAGetLastError() << "\n";
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    char ipbuf[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, sizeof(ipbuf));
    std::cout << "Cliente conectado desde " << ipbuf << ":" << ntohs(client_addr.sin_port) << "\n\n";

    Persona p;
    capturarPersona(p);

    std::string reporte = generarReporte(p, "\r\n");
    std::cout << "\n--- REPORTE GENERADO ---\n" << reporte << "\n";

    int total = 0;
    int len = (int)reporte.size();
    while (total < len) {
        int sent = send(client_fd, reporte.c_str() + total, len - total, 0);
        if (sent == SOCKET_ERROR) {
            std::cerr << "send() fallo. WSAGetLastError: " << WSAGetLastError() << "\n";
            break;
        }
        total += sent;
    }

    closesocket(client_fd);
    closesocket(server_fd);
    WSACleanup();

    std::cout << "Reporte enviado. Conexion cerrada.\n";
    return 0;
}

#else
// ---------------- LINUX / DOCKER ----------------
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int port = 8080;
    if (argc >= 2) port = std::stoi(argv[1]);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind"); close(server_fd); return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen"); close(server_fd); return 1;
    }

    std::cout << "Servidor escuchando en el puerto " << port << "...\n";
    std::cout << "Esperando conexion de un cliente...\n\n";

    sockaddr_in client_addr{};
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) { perror("accept"); close(server_fd); return 1; }

    char ipbuf[INET_ADDRSTRLEN]{};
    inet_ntop(AF_INET, &client_addr.sin_addr, ipbuf, sizeof(ipbuf));
    std::cout << "Cliente conectado desde " << ipbuf << ":" << ntohs(client_addr.sin_port) << "\n\n";

    Persona p;
    capturarPersona(p);

    std::string reporte = generarReporte(p, "\n");
    std::cout << "\n--- REPORTE GENERADO ---\n" << reporte << "\n";

    if (send(client_fd, reporte.c_str(), reporte.size(), 0) < 0) perror("send");

    close(client_fd);
    close(server_fd);

    std::cout << "Reporte enviado. Conexion cerrada.\n";
    return 0;
}
#endif