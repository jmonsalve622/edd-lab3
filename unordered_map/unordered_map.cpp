
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

// Un tweet reducido a sus dos claves de interes.
struct Tweet {
    uint64_t user_id;
    std::string screen_name;
};

// Carga el dataset completo en memoria. Se hace antes de cualquier medicion
// de tiempo para que la lectura de disco no contamine los experimentos.
std::vector<Tweet> leerDataset(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo) {
        std::cerr << "Error: no se pudo abrir " << ruta << "\n";
        std::exit(1);
    }
    std::vector<Tweet> tweets;
    tweets.reserve(190000);  // evita realocaciones (sabemos que hay ~183k)
    std::string linea;
    while (std::getline(archivo, linea)) {
        size_t sep = linea.find(';');
        tweets.push_back({std::stoull(linea.substr(0, sep)),
                          linea.substr(sep + 1)});
    }
    return tweets;
}

int main(int argc, char* argv[]) {
    std::string ruta = (argc > 1) ? argv[1] : "../usuarios.csv";
    std::vector<Tweet> tweets = leerDataset(ruta);
    std::cout << "Tweets leidos: " << tweets.size() << "\n\n";

    // --- Conteo usando user_id como clave ---
    // Esquema del enunciado: if (k esta en H) H[k]++; else H[k] = 1;
    std::unordered_map<uint64_t, int> porId;
    for (const Tweet& t : tweets) {
        auto it = porId.find(t.user_id);
        if (it != porId.end()) {
            it->second = it->second + 1;
        } else {
            porId[t.user_id] = 1;
        }
    }

    // --- Conteo usando user_screen_name como clave ---
    std::unordered_map<std::string, int> porScreenName;
    for (const Tweet& t : tweets) {
        auto it = porScreenName.find(t.screen_name);
        if (it != porScreenName.end()) {
            it->second = it->second + 1;
        } else {
            porScreenName[t.screen_name] = 1;
        }
    }

    // --- Validacion: ambas tablas deben coincidir ---
    long long totalId = 0, totalScreen = 0;
    for (const auto& [clave, cuenta] : porId) totalId += cuenta;
    for (const auto& [clave, cuenta] : porScreenName) totalScreen += cuenta;

    std::cout << "Usuarios unicos (por user_id)     : " << porId.size() << "\n";
    std::cout << "Usuarios unicos (por screen_name) : " << porScreenName.size() << "\n";
    std::cout << "Suma de contadores (por user_id)  : " << totalId << "\n";
    std::cout << "Suma de contadores (screen_name)  : " << totalScreen << "\n\n";

    // --- Usuario con mas tweets, como muestra del resultado ---
    std::string topUsuario;
    int topCuenta = 0;
    for (const auto& [nombre, cuenta] : porScreenName) {
        if (cuenta > topCuenta) {
            topCuenta = cuenta;
            topUsuario = nombre;
        }
    }
    std::cout << "Usuario mas activo: @" << topUsuario
              << " con " << topCuenta << " tweets\n";
    return 0;
}
