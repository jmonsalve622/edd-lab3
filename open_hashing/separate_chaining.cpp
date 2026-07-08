// Compilación: g++ open_hashing/separate_chaining.cpp -o open_hashing/separate_chaining
// Ejecución: ./open_hashing/separate_chaining [num_experiments] [read_count] 

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

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

    void clear() {
        for (auto& bucket : buckets) {
            Node* current = bucket;
            while (current) {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
            bucket = nullptr;
        }
        nodes_count = 0;
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

    void clear() {
        for (auto& bucket : buckets) {
            Node* current = bucket;
            while (current) {
                Node* toDelete = current;
                current = current->next;
                delete toDelete;
            }
            bucket = nullptr;
        }
        nodes_count = 0;
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

double standardDeviation(const std::vector<double>& data, double mean) {
    double sum = 0.0;
    for (double value : data) {
        sum += (value - mean) * (value - mean);
    }
    return std::sqrt(sum / data.size());
}

int main(int argc, char* argv[]) {
    int numExperiments = (argc > 1) ? std::stoi(argv[1]) : 1;
    int readCount = (argc > 2) ? std::stoi(argv[2]) : 183361;

    if (readCount < 1 || readCount > 183361) {
        std::cerr << "Error: readCount debe ser un valor entre 1 y 183361\n";
        return 1;
    }
    else if (numExperiments < 1) {
        std::cerr << "Error: numExperiments debe ser un valor mayor o igual a 1\n";
        return 1;
    }

    std::vector<Tweet> tweets = leerDataset("./usuarios.csv", readCount);

    // Tabla hash con user_id como clave
    HashTableById idTable(int(tweets.size() * 0.8));
    double timeSumIds = 0;
    std::vector<double> timesIds;
    for (int i = 0; i < numExperiments; i++) {
        auto start_id = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            idTable.insert(t.user_id);
        }
        auto end_id = std::chrono::high_resolution_clock::now();
        double tiempo_ids_ms = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end_id - start_id).count();
        timeSumIds += tiempo_ids_ms;
        timesIds.push_back(tiempo_ids_ms);
        if (i < numExperiments - 1) {
            idTable.clear(); // Limpiar la tabla para el siguiente experimento
        }
    }
    double meanTimeIds = timeSumIds / numExperiments;
    double stdDeviationIds = standardDeviation(timesIds, meanTimeIds);

    // Tabla hash con screen_name como clave
    HashTableByScreenName screenNameTable(int(tweets.size() * 0.8));
    double timeSumNames = 0;
    std::vector<double> timesNames;
    for (int i = 0; i < numExperiments; i++) {
        auto start_name = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            screenNameTable.insert(t.screen_name);
        }
        auto end_name = std::chrono::high_resolution_clock::now();
        double tiempo_names_ms = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end_name - start_name).count();
        timeSumNames += tiempo_names_ms;
        timesNames.push_back(tiempo_names_ms);
        if (i < numExperiments - 1) {
            screenNameTable.clear(); // Limpiar la tabla para el siguiente experimento
        }
    }
    double meanTimeNames = timeSumNames / numExperiments;
    double stdDeviationNames = standardDeviation(timesNames, meanTimeNames);
    
    // Resultados
    std::cout << "Tabla hash con separate chaining (open hashing)\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Experimentos realizados: " << numExperiments << "\n";
    std::cout << "Tweets leidos: " << tweets.size() << "\n\n";

    std::cout << " --- Tabla hash con user_id como clave ---\n";
    std::cout << "Usuarios unicos: " << idTable.size() << "\n";
    std::cout << "Suma de contadores: " << idTable.getValuesCount() << "\n";
    std::cout << "Tamaño en memoria: " << idTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de insercion promedio: " << int(meanTimeIds) << " μs\n";
    std::cout << "Desviacion estandar: " << int(stdDeviationIds) << " μs\n\n";

    std::cout << " --- Tabla hash con user_screen_name como clave ---\n";
    std::cout << "Usuarios unicos: " << screenNameTable.size() << "\n";
    std::cout << "Suma de contadores: " << screenNameTable.getValuesCount() << "\n";
    std::cout << "Tamaño en memoria: " << screenNameTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de insercion promedio: " << int(meanTimeNames) << " μs\n";
    std::cout << "Desviacion estandar: " << int(stdDeviationNames)<< " μs\n\n";

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