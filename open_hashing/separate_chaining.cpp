// Compilación: g++ open_hashing/separate_chaining.cpp -o open_hashing/separate_chaining
// Ejecución: ./open_hashing/separate_chaining [read_count]

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

class HashTableById {
    struct Node {
        uint64_t key;
        int value;
        Node* next;
    };

    int nodes_count = 0;
    int capacity; 
    std::vector<Node*> buckets;
    
    int hash(uint64_t key) {
        return key % capacity;
    }

public:
    HashTableById(int cap) : capacity(cap), buckets(cap, nullptr) {}

    void insert(uint64_t key) {
        int index = hash(key);
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                current->value++;
                return;
            }
            current = current->next;
        }
        Node* newNode = new Node{key, 1, buckets[index]};
        buckets[index] = newNode;
        nodes_count++;
    }

    bool get(uint64_t key) {
        int index = hash(key);
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        return false;
    }

    int size() const {
        return nodes_count;
    }
};

class HashTableByScreenName {
    struct Node {
        std::string key;
        int value;
        Node* next;
    };

    int nodes_count = 0;
    int capacity;
    std::vector<Node*> buckets;
    
    int hash(const std::string& key) {
        std::hash<std::string> hasher;
        return hasher(key) % capacity;
    }

public:
    HashTableByScreenName(int cap) : capacity(cap), buckets(cap, nullptr) {}

    void insert(const std::string& key) {
        int index = hash(key);
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                current->value++;
                return;
                }
            current = current->next;
        }
        Node* newNode = new Node{key, 1, buckets[index]};
        buckets[index] = newNode;
        nodes_count++;
    }

    bool get(const std::string& key) {
        int index = hash(key);
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                return current->value;
            }
            current = current->next;
        }
        return false;
    }

    int size() const {
        return nodes_count;
    }
};

// Un tweet reducido a sus dos claves de interes.
struct Tweet {
    uint64_t user_id;
    std::string screen_name;
};

// Carga el dataset completo en memoria. Se hace antes de cualquier medicion
// de tiempo para que la lectura de disco no contamine los experimentos.
std::vector<Tweet> leerDataset(const std::string& ruta, int readCount) {
    std::ifstream archivo(ruta);
    if (!archivo) {
        std::cerr << "Error: no se pudo abrir " << ruta << "\n";
        std::exit(1);
    }
    std::vector<Tweet> tweets;
    tweets.reserve(readCount);
    std::string linea;
    int count = 0;
    while (std::getline(archivo, linea) && count < readCount) {
        size_t sep = linea.find(';');
        tweets.push_back({std::stoull(linea.substr(0, sep)),
                          linea.substr(sep + 1)});
        count++;
    }
    return tweets;
}

int main(int argc, char* argv[]) {
    int readCount = (argc > 1) ? std::stoi(argv[1]) : 183361;
    if (readCount < 1 || readCount > 183361) {
        std::cerr << "Error: readCount debe ser un valor entre 1 y 183361\n";
        return 1;
    }
    std::vector<Tweet> tweets = leerDataset("./usuarios.csv", readCount);
    std::cout << "Tweets leidos: " << tweets.size() << "\n\n";

    // --- Conteo usando user_id como clave ---
    // Esquema del enunciado: if (k esta en H) H[k]++; else H[k] = 1;
    HashTableById porId(int(tweets.size() * 0.8));
    for (const Tweet& t : tweets) {
        porId.insert(t.user_id);
    }

    // --- Conteo usando user_screen_name como clave ---
    HashTableByScreenName porScreenName(int(tweets.size() * 0.8));
    for (const Tweet& t : tweets) {
        porScreenName.insert(t.screen_name);
    }
    
    // --- Validacion: ambas tablas deben coincidir ---
    // long long totalId = 0, totalScreen = 0;
    // for (const auto& [clave, cuenta] : porId) totalId += cuenta;
    // for (const auto& [clave, cuenta] : porScreenName) totalScreen += cuenta;

    std::cout << "Usuarios unicos (por user_id)     : " << porId.size() << "\n";
    std::cout << "Usuarios unicos (por screen_name) : " << porScreenName.size() << "\n";
    // std::cout << "Suma de contadores (por user_id)  : " << totalId << "\n";
    // std::cout << "Suma de contadores (screen_name)  : " << totalScreen << "\n\n";


    // --- Usuario con mas tweets, como muestra del resultado ---
    // std::string topUsuario;
    // int topCuenta = 0;
    // for (const auto& [nombre, cuenta] : porScreenName) {
    //     if (cuenta > topCuenta) {
    //         topCuenta = cuenta;
    //         topUsuario = nombre;
    //     }
    // }
    // std::cout << "Usuario mas activo: @" << topUsuario
    //           << " con " << topCuenta << " tweets\n";
    return 0;
}