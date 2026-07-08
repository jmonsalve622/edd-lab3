// Compilación: g++ closed_hashing/double_hashing.cpp -o closed_hashing/double_hashing
// Ejecución: ./closed_hashing/double_hashing [read_count] [num_experimento]

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <cmath>

using namespace std;
using namespace std::chrono;

enum Estado { VACIO, OCUPADO, BORRADO };

struct Tweet {
    uint64_t user_id;
    string screen_name;
};

struct NodoHashID {
    uint64_t clave;
    int conteo;
    Estado estado;
    NodoHashID() : clave(0), conteo(0), estado(VACIO) {}
};

class DoubleHashingID {
private:
    vector<NodoHashID> tabla;
    int capacidad;
    int elementos;
    int primo_menor;

    size_t hash1(uint64_t clave) { return clave % capacidad; }
    
    size_t hash2(uint64_t clave) { return primo_menor - (clave % primo_menor); }

public:
    DoubleHashingID(int cap) {
        capacidad = cap;
        elementos = 0;
        primo_menor = 399989;
        tabla.resize(capacidad);
    }

    void insert(uint64_t clave) {
        size_t h1 = hash1(clave);
        size_t h2 = hash2(clave);
        size_t i = 0;

        // Fórmula Double Hashing: H(k, i) = (h1(k) + i * h2(k)) mod N
        while (tabla[(h1 + i * h2) % capacidad].estado == OCUPADO) {
            if (tabla[(h1 + i * h2) % capacidad].clave == clave) {
                tabla[(h1 + i * h2) % capacidad].conteo++;
                return;
            }
            i++;
            if (i >= capacidad) {
                cerr << "Error: Tabla llena o ciclo infinito en IDs." << endl;
                return;
            }
        }

        size_t pos_final = (h1 + i * h2) % capacidad;
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
    string clave;
    int conteo;
    Estado estado;
    NodoHashName() : clave(""), conteo(0), estado(VACIO) {}
};

class DoubleHashingName {
private:
    vector<NodoHashName> tabla;
    int capacidad;
    int elementos;
    int primo_menor;

    size_t hash1(const string& clave) {
        size_t hash = 0, p = 31, p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % capacidad;
            p_pow = (p_pow * p) % capacidad;
        }
        return hash;
    }

    size_t hash2(const string& clave) {
        size_t hash = 0, p = 37, p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % primo_menor;
            p_pow = (p_pow * p) % primo_menor;
        }
        return primo_menor - hash;
    }

public:
    DoubleHashingName(int cap) {
        capacidad = cap;
        elementos = 0;
        primo_menor = 399989; 
        tabla.resize(capacidad);
    }

    void insert(const string& clave) {
        size_t h1 = hash1(clave);
        size_t h2 = hash2(clave);
        size_t i = 0;

        // Fórmula Double Hashing: H(k, i) = (h1(k) + i * h2(k)) mod N
        while (tabla[(h1 + i * h2) % capacidad].estado == OCUPADO) {
            if (tabla[(h1 + i * h2) % capacidad].clave == clave) {
                tabla[(h1 + i * h2) % capacidad].conteo++;
                return;
            }
            i++;
            if (i >= capacidad) {
                cerr << "Error: Tabla llena o ciclo infinito en Names." << endl;
                return;
            }
        }

        size_t pos_final = (h1 + i * h2) % capacidad;
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
    string getUsuarioMasActivo(int& max_tweets) const {
        string topUsuario = "";
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
            size += sizeof(nodo.clave) + nodo.clave.capacity() + sizeof(nodo.conteo) + sizeof(nodo.estado);
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

vector<Tweet> leerDataset(const string& ruta, int readCount) {
    ifstream archivo(ruta);
    if (!archivo) {
        cerr << "Error: no se pudo abrir " << ruta << "\n";
        exit(1);
    }
    vector<Tweet> tweets;
    tweets.reserve(readCount);
    string linea;
    int count = 0;
    while (getline(archivo, linea) && count < readCount) {
        size_t sep = linea.find(';');
        tweets.push_back({stoull(linea.substr(0, sep)), linea.substr(sep + 1)});
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

    vector<Tweet> tweets = leerDataset("usuarios.csv", readCount);
    
    int N = 400009; 
    
    // Tabla hash con user_id como clave
    DoubleHashingID idTable(N);
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
    DoubleHashingName screenNameTable(N);
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
    std::cout << "Tabla hash con separate chaining (open hashing)\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Experimentos realizados: " << numExperiments << "\n";
    std::cout << "Tweets leidos: " << tweets.size() << "\n\n";

    std::cout << " --- Tabla hash con user_id como clave ---\n";
    std::cout << "Usuarios unicos: " << idTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << idTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de insercion promedio: " << int(meanTimeIds) << " μs\n";
    std::cout << "Desviacion estandar: " << int(stdDeviationIds) << " μs\n\n";

    std::cout << " --- Tabla hash con user_screen_name como clave ---\n";
    std::cout << "Usuarios unicos: " << screenNameTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << screenNameTable.inMemorySize() / 1024 << " KB\n";
    std::cout << "Tiempo de insercion promedio: " << int(meanTimeNames) << " μs\n";
    std::cout << "Desviacion estandar: " << int(stdDeviationNames)<< " μs\n\n";
}