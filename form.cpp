#include <iostream>
#include <string>
#include <limits>

using namespace std;

struct Persona {
    string nombre;
    string dni;
    int edad;
    string telefono;
    string direccion;
};

int main() {
    Persona p;

    cout << "=== FORMULARIO DE REGISTRO ===\n\n";

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Nombre: ";
    getline(cin, p.nombre);

    cout << "DNI: ";
    getline(cin, p.dni);

    cout << "Edad: ";
    cin >> p.edad;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Telefono: ";
    getline(cin, p.telefono);

    cout << "Direccion: ";
    getline(cin, p.direccion);

    // REPORTE
    cout << "\n\n====================================\n";
    cout << "        REPORTE DE PERSONA\n";
    cout << "====================================\n";
    cout << "Nombre     : " << p.nombre << "\n";
    cout << "DNI        : " << p.dni << "\n";
    cout << "Edad       : " << p.edad << "\n";
    cout << "Telefono   : " << p.telefono << "\n";
    cout << "Direccion  : " << p.direccion << "\n";
    cout << "====================================\n";

    return 0;
}