// Compilación: g++ closed_hashing/quadratic_probing.cpp -o closed_hashing/quadratic_probing
// Ejecución: ./closed_hashing/quadratic_probing [num_experiments] [read_count]

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <cmath>

enum Estado { VACIO, OCUPADO, BORRADO };

struct Tweet {
    uint64_t user_id;
    std::string screen_name;
};

struct NodoHashID {
    uint64_t clave;
    int conteo;
    Estado estado;
    NodoHashID() : clave(0), conteo(0), estado(VACIO) {}
};

class QuadraticProbingID {
private:
    std::vector<NodoHashID> tabla;
    int capacidad;
    int elementos;

    size_t hash_id(uint64_t clave) { return clave % capacidad; }

public:
    QuadraticProbingID(int cap) {
        capacidad = cap;
        elementos = 0;
        tabla.resize(capacidad);
    }

    void insert(uint64_t clave) {
        size_t indice = hash_id(clave);
        size_t i = 0;

        // Formula Quadratic Probing: H(k, i) = (h(k) + i^2) mod N
        while (tabla[(indice + i * i) % capacidad].estado == OCUPADO) {
            if (tabla[(indice + i * i) % capacidad].clave == clave) {
                tabla[(indice + i * i) % capacidad].conteo++;
                return;
            }
            i++;
            if (i >= capacidad) {
                std::cerr << "Error: Tabla llena o ciclo infinito en IDs." << std::endl;
                return;
            }
        }

        size_t pos_final = (indice + i * i) % capacidad;
        tabla[pos_final].clave = clave;
        tabla[pos_final].conteo = 1;
        tabla[pos_final].estado = OCUPADO;
        elementos++;
    }

    int getElementosUnicos() const { return elementos; }
    long long getSumaContadores() const {
        long long suma = 0;
        for (const auto& nodo : tabla) {
            if (nodo.estado == OCUPADO) suma += nodo.conteo;
        }
        return suma;
    }

    int size() const { 
        return elementos; 
    }

    int inMemorySize() const {
        size_t size = sizeof(tabla);
        for (const auto& nodo : tabla) {
            size += sizeof(nodo.clave) + sizeof(nodo.conteo) + sizeof(nodo.estado);
        }
        return size;
    }

    void clear() {
        for (auto& nodo : tabla) {
            nodo.estado = VACIO;
            nodo.clave = 0;
            nodo.conteo = 0;
        }
        elementos = 0;
    }
};

struct NodoHashName {
    std::string clave;
    int conteo;
    Estado estado;
    NodoHashName() : clave(""), conteo(0), estado(VACIO) {}
};

class QuadraticProbingName {
private:
    std::vector<NodoHashName> tabla;
    int capacidad;
    int elementos;

    size_t hash_name(const std::string& clave) {
        size_t hash = 0, p = 31, p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % capacidad;
            p_pow = (p_pow * p) % capacidad;
        }
        return hash;
    }

public:
    QuadraticProbingName(int cap) {
        capacidad = cap;
        elementos = 0;
        tabla.resize(capacidad);
    }

    void insert(const std::string& clave) {
        size_t indice = hash_name(clave);
        size_t i = 0;

        // Formula Quadratic Probing: H(k, i) = (h(k) + i^2) mod N
        while (tabla[(indice + i * i) % capacidad].estado == OCUPADO) {
            if (tabla[(indice + i * i) % capacidad].clave == clave) {
                tabla[(indice + i * i) % capacidad].conteo++;
                return;
            }
            i++;
            if (i >= capacidad) {
                std::cerr << "Error: Tabla llena o ciclo infinito en Names." << std::endl;
                return;
            }
        }

        size_t pos_final = (indice + i * i) % capacidad;
        tabla[pos_final].clave = clave;
        tabla[pos_final].conteo = 1;
        tabla[pos_final].estado = OCUPADO;
        elementos++;
    }

    int getElementosUnicos() const { return elementos; }
    long long getSumaContadores() const {
        long long suma = 0;
        for (const auto& nodo : tabla) {
            if (nodo.estado == OCUPADO) suma += nodo.conteo;
        }
        return suma;
    }
    std::string getUsuarioMasActivo(int& max_tweets) const {
        std::string topUsuario = "";
        max_tweets = 0;
        for (const auto& nodo : tabla) {
            if (nodo.estado == OCUPADO && nodo.conteo > max_tweets) {
                max_tweets = nodo.conteo;
                topUsuario = nodo.clave;
            }
        }
        return topUsuario;
    }

    int size() const { 
        return elementos; 
    }

    int inMemorySize() const {
        size_t size = sizeof(tabla);
        for (const auto& nodo : tabla) {
            size += sizeof(nodo.clave) + sizeof(nodo.conteo) + sizeof(nodo.estado);
        }
        return size;
    }

    void clear() {
        for (auto& nodo : tabla) {
            nodo.estado = VACIO;
            nodo.clave = "";
            nodo.conteo = 0;
        }
        elementos = 0;
    }
};

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
        tweets.push_back({std::stoull(linea.substr(0, sep)), linea.substr(sep + 1)});
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
    if (numExperiments < 1) {
        std::cerr << "Error: numExperiments debe ser un valor mayor o igual a 1\n";
        return 1;
    }

    std::vector<Tweet> tweets = leerDataset("usuarios.csv", readCount);
    
    int N = 400009; 
    
    // Tabla hash con user_id como clave
    QuadraticProbingID idTable(N);
    double timeSumIds = 0;
    std::vector<double> timesIds;
    for (int i = 0; i < numExperiments; i++) {
        auto start_id = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            idTable.insert(t.user_id);
        }
        auto end_id = std::chrono::high_resolution_clock::now();
        double tiempo_ids = std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(end_id - start_id).count();
        timeSumIds += tiempo_ids;
        timesIds.push_back(tiempo_ids);
        if (i < numExperiments - 1) {
            idTable.clear(); // Limpiar la tabla para el siguiente experimento
        }
    }
    double meanTimeIds = timeSumIds / numExperiments;
    double stdDeviationIds = standardDeviation(timesIds, meanTimeIds);

    // Tabla hash con user_screen_name como clave
    QuadraticProbingName screenNameTable(N);
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

     // Resultados
    std::cout << "-----------------------------------------------\n";
    std::cout << "Tabla hash con quadratic probing (closed hashing)\n";
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

    return 0;
}