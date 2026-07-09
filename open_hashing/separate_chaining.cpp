// Tabla hash con hashing abierto y resolucion de colisiones por encadenamiento
// (separate chaining). Cada posicion de la tabla es la cabeza de una lista
// enlazada que almacena todos los elementos cuyo hash coincide en esa casilla.
//
// Compilación: g++ open_hashing/separate_chaining.cpp -o open_hashing/separate_chaining
// Ejecución: ./open_hashing/separate_chaining [num_experiments] [read_count]

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

// Tabla hash por encadenamiento con user_id (entero) como clave.
class HashTableById {
    // Nodo de la lista enlazada de cada bucket: clave, contador de tweets
    // del usuario y puntero al siguiente nodo de la cadena.
    struct Node {
        uint64_t key;
        int value;
        Node* next;
    };

    int nodes_count = 0;        // cantidad de claves distintas insertadas
    int capacity;               // cantidad de buckets de la tabla
    std::vector<Node*> buckets; // cada bucket apunta a la cabeza de su lista

    // Funcion hash por division: h(k) = k mod capacidad.
    int hash(uint64_t key) {
        return key % capacity;
    }

public:
    // Crea la tabla con todos los buckets vacios (nullptr).
    HashTableById(int cap) : capacity(cap), buckets(cap, nullptr) {}

    // Inserta la clave o incrementa su contador si ya existe.
    // Recorre la lista del bucket correspondiente buscando la clave;
    // si no esta, agrega un nuevo nodo al inicio de la cadena.
    void insert(uint64_t key) {
        int index = hash(key);
        Node* current = buckets[index];
        while (current) {
            if (current->key == key) {
                current->value++; // la clave ya existia: un tweet mas
                return;
            }
            current = current->next;
        }
        // Insercion al inicio de la lista: costo O(1).
        Node* newNode = new Node{key, 1, buckets[index]};
        buckets[index] = newNode;
        nodes_count++;
    }

    // Busca la clave en la lista de su bucket; retorna si fue encontrada.
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

    // Cantidad de claves distintas almacenadas (usuarios unicos).
    int size() const {
        return nodes_count;
    }

    // Estimacion del tamano en memoria: objeto + arreglo de punteros de los
    // buckets + nodos efectivamente creados. Es una aproximacion.
    int inMemorySize() const {
        return sizeof(HashTableById) + sizeof(Node*) * capacity + sizeof(Node) * nodes_count;
    }

    // Suma de todos los contadores; debe coincidir con el total de tweets
    // insertados (se usa como verificacion de correctitud).
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

    // Libera todos los nodos y deja los buckets vacios para repetir
    // el experimento desde cero.
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

// Tabla hash por encadenamiento con user_screen_name (string) como clave.
// Identica en estructura a HashTableById, salvo por el tipo de clave y la
// funcion hash, que delega en std::hash<std::string> de la STL.
class HashTableByScreenName {
    struct Node {
        std::string key;
        int value;
        Node* next;
    };

    int nodes_count = 0;
    int capacity;
    std::vector<Node*> buckets;

    // Hash de la biblioteca estandar para strings, reducido a la capacidad.
    int hash(const std::string& key) {
        std::hash<std::string> hasher;
        return hasher(key) % capacity;
    }

public:
    HashTableByScreenName(int cap) : capacity(cap), buckets(cap, nullptr) {}

    // Misma logica de insercion que en la version por id.
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

    // Estimacion del tamano en memoria (no incluye la capacidad interna
    // reservada por cada std::string).
    int inMemorySize() const {
        return sizeof(HashTableById) + sizeof(Node*) * capacity + sizeof(Node) * nodes_count;
    }

    // Libera todos los nodos y deja los buckets vacios.
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
// Cada linea de usuarios.csv tiene el formato "user_id;screen_name".
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

// Desviacion estandar poblacional de las mediciones de tiempo.
double standardDeviation(const std::vector<double>& data, double mean) {
    double sum = 0.0;
    for (double value : data) {
        sum += (value - mean) * (value - mean);
    }
    return std::sqrt(sum / data.size());
}

int main(int argc, char* argv[]) {
    // Argumentos: cantidad de repeticiones del experimento y cantidad de
    // tweets a procesar (por defecto el dataset completo).
    int numExperiments = (argc > 1) ? std::stoi(argv[1]) : 1;
    int readCount = (argc > 2) ? std::stoi(argv[2]) : 183361;

    if (readCount < 1 || readCount > 183361) {
        std::cerr << "Error: readCount debe ser un valor entre 1 y 183361\n";
        return 1;
    }
    if (numExperiments < 1) {
        std::cerr << "Error: numExperiments debe ser un valor mayor o igual a 1\n";
        return 1;
    }

    std::vector<Tweet> tweets = leerDataset("./usuarios.csv", readCount);

    // ------- Experimento 1: tabla hash con user_id como clave -------
    // La cantidad de buckets se fija en el 80% de los tweets leidos.
    // Se repite la insercion de todos los tweets numExperiments veces,
    // midiendo solo el tiempo del bucle de insercion.
    HashTableById idTable(int(tweets.size() * 0.8));
    double timeSumIds = 0;
    std::vector<double> timesIds;
    for (int i = 0; i < numExperiments; i++) {
        auto start_id = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            idTable.insert(t.user_id);
        }
        auto end_id = std::chrono::high_resolution_clock::now();
        // Duracion de esta repeticion en microsegundos.
        double tiempo_ids = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end_id - start_id).count();
        timeSumIds += tiempo_ids;
        timesIds.push_back(tiempo_ids);
        if (i < numExperiments - 1) {
            idTable.clear(); // Limpiar la tabla para el siguiente experimento
        }
    }
    double meanTimeIds = timeSumIds / numExperiments;
    double stdDeviationIds = standardDeviation(timesIds, meanTimeIds);

    // ------- Experimento 2: tabla hash con user_screen_name como clave -------
    HashTableByScreenName screenNameTable(int(tweets.size() * 0.8));
    double timeSumNames = 0;
    std::vector<double> timesNames;
    for (int i = 0; i < numExperiments; i++) {
        auto start_name = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            screenNameTable.insert(t.screen_name);
        }
        auto end_name = std::chrono::high_resolution_clock::now();
        double tiempo_names = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end_name - start_name).count();
        timeSumNames += tiempo_names;
        timesNames.push_back(tiempo_names);
        if (i < numExperiments - 1) {
            screenNameTable.clear(); // Limpiar la tabla para el siguiente experimento
        }
    }
    double meanTimeNames = timeSumNames / numExperiments;
    double stdDeviationNames = standardDeviation(timesNames, meanTimeNames);

    // ------- Resultados legibles por consola -------
    std::cout << "-----------------------------------------------\n";
    std::cout << "Tabla hash con separate chaining (open hashing)\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Experimentos realizados: " << numExperiments << "\n";
    std::cout << "Tweets leídos: " << tweets.size() << "\n\n";

    std::cout << " --- Tabla hash con user_id como clave ---\n";
    std::cout << "Usuarios únicos: " << idTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << idTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de inserción promedio: " << int(meanTimeIds) << " μs\n";
    std::cout << "Desviación estandar: " << int(stdDeviationIds) << " μs\n\n";

    std::cout << " --- Tabla hash con user_screen_name como clave ---\n";
    std::cout << "Usuarios únicos: " << screenNameTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << screenNameTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de inserción promedio: " << int(meanTimeNames) << " μs\n";
    std::cout << "Desviación estandar: " << int(stdDeviationNames)<< " μs\n\n";

    // --- LÍNEAS PARA EL SCRIPT DE BASH ---
    // Formato CSV: n;estructura;clave;repeticiones;tiempo_prom;desv_est;memoria_kb
    // ejecutar_experimentos.sh filtra estas lineas (grep ";") hacia resultados.csv
    std::cout << readCount << ";separate_chaining;user_id;" << numExperiments << ";"
              << meanTimeIds << ";" << stdDeviationIds << ";" << idTable.inMemorySize() / 1024 << "\n";

    std::cout << readCount << ";separate_chaining;user_screen_name;" << numExperiments << ";"
              << meanTimeNames << ";" << stdDeviationNames << ";" << screenNameTable.inMemorySize() / 1024 << "\n";

    return 0;
}
