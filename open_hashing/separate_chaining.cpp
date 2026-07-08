// Compilación: g++ open_hashing/separate_chaining.cpp -o open_hashing/separate_chaining
// Ejecución: ./open_hashing/separate_chaining [read_count]

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>

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

    int inMemorySize() const {
        return sizeof(HashTableById) + sizeof(Node*) * capacity + sizeof(Node) * nodes_count;
    }

    int getValuesCount() const {
        int count = 0;
        for (const auto& bucket : buckets) {
            Node* current = bucket;
            while (current) {
                count += current->value;
                current = current->next;
            }
        }
        return count;
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

    int inMemorySize() const {
        return sizeof(HashTableById) + sizeof(Node*) * capacity + sizeof(Node) * nodes_count;
    }

    int getValuesCount() const {
        int count = 0;
        for (const auto& bucket : buckets) {
            Node* current = bucket;
            while (current) {
                count += current->value;
                current = current->next;
            }
        }
        return count;
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

    

    // Tabla hash con user_id como clave
    HashTableById idTable(int(tweets.size() * 0.8));
    auto start_id = std::chrono::high_resolution_clock::now();
    for (const Tweet& t : tweets) {
        idTable.insert(t.user_id);
    }
    auto end_id = std::chrono::high_resolution_clock::now();
    double tiempo_ids_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_id - start_id).count();

    // Tabla hash con screen_name como clave
    HashTableByScreenName screenNameTable(int(tweets.size() * 0.8));
    auto start_name = std::chrono::high_resolution_clock::now();
    for (const Tweet& t : tweets) {
        screenNameTable.insert(t.screen_name);
    }
    auto end_name = std::chrono::high_resolution_clock::now();
    double tiempo_names_ms = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end_name - start_name).count();
    
    // Resultados
    std::cout << "Tweets leidos: " << tweets.size() << "\n\n";
    std::cout << "Usuarios unicos (por user_id)     : " << idTable.size() << "\n";
    std::cout << "Usuarios unicos (por screen_name) : " << screenNameTable.size() << "\n";
    std::cout << "Suma de contadores (por user_id)  : " << idTable.getValuesCount() << "\n";
    std::cout << "Suma de contadores (screen_name)  : " << screenNameTable.getValuesCount() << "\n\n";
    std::cout << "Tamaño en memoria (por user_id)     : " << idTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tamaño en memoria (por screen_name) : " << screenNameTable.inMemorySize() / 1024 << " KB\n\n";
    std::cout << "Tiempo de insercion (por user_id)     : " << tiempo_ids_ms << " ms\n";
    std::cout << "Tiempo de insercion (por screen_name) : " << tiempo_names_ms << " ms\n\n";

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