#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

struct Persona {
    string nombre;
    string dni;
    int edad;
    string telefono;
    string direccion;
};

static string htmlEscape(const string& s) {
    string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            default: out += c; break;
        }
    }
    return out;
}

static string generarReporteHTML(const Persona& p) {
    ostringstream out;
    out << "<!doctype html><html><head><meta charset='utf-8'>"
        << "<title>Reporte</title></head><body style='font-family:monospace;'>"
        << "<h2>REPORTE DE PERSONA</h2><pre>"
        << "====================================\n"
        << "Nombre     : " << htmlEscape(p.nombre) << "\n"
        << "DNI        : " << htmlEscape(p.dni) << "\n"
        << "Edad       : " << p.edad << "\n"
        << "Telefono   : " << htmlEscape(p.telefono) << "\n"
        << "Direccion  : " << htmlEscape(p.direccion) << "\n"
        << "====================================\n"
        << "</pre></body></html>";
    return out.str();
}

static void sendHttp(int client_fd, int code, const string& contentType, const string& body) {
    string status = (code == 200) ? "200 OK" : "404 Not Found";
    ostringstream resp;
    resp << "HTTP/1.1 " << status << "\r\n";
    resp << "Content-Type: " << contentType << "\r\n";
    resp << "Content-Length: " << body.size() << "\r\n";
    resp << "Connection: close\r\n\r\n";
    resp << body;

    string data = resp.str();
    send(client_fd, data.c_str(), data.size(), 0);
}

int main() {
    // Render define PORT
    const char* portEnv = getenv("PORT");
    int port = portEnv ? atoi(portEnv) : 8080;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 128) < 0) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    cerr << "HTTP server escuchando en 0.0.0.0:" << port << "\n";

    // Persona de ejemplo (porque Render no puede pedir teclado)
    Persona p{"Juan Perez", "0801-2000-12345", 22, "9999-9999", "Barrio Centro"};

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) { perror("accept"); continue; }

        char buf[2048];
        int n = recv(client_fd, buf, sizeof(buf) - 1, 0);
        if (n <= 0) { close(client_fd); continue; }
        buf[n] = '\0';

        // parse sencillo: primera línea "GET / HTTP/1.1"
        string req(buf);
        bool isRoot = req.rfind("GET / ", 0) == 0 || req.rfind("GET /HTTP", 0) == 0;

        if (isRoot) {
            string body = generarReporteHTML(p);
            sendHttp(client_fd, 200, "text/html; charset=utf-8", body);
        } else {
            sendHttp(client_fd, 404, "text/plain; charset=utf-8", "404 Not Found");
        }

        close(client_fd);
    }

    // unreachable
    // close(server_fd);
    return 0;
}
