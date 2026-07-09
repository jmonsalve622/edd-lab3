// Tabla hash con hashing cerrado y resolucion de colisiones por doble hashing
// (double hashing). El paso de sondeo depende de una segunda funcion hash:
// H(k, i) = (h1(k) + i * h2(k)) mod N, de modo que cada clave recorre la tabla
// con una secuencia distinta, eliminando el agrupamiento primario y secundario.
//
// Compilación: g++ closed_hashing/double_hashing.cpp -o closed_hashing/double_hashing
// Ejecución: ./closed_hashing/double_hashing [num_experimento] [read_count]

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cstdint>
#include <cmath>

// Estado de cada casilla de la tabla. BORRADO se define para soportar
// eliminacion perezosa, aunque en este experimento no se eliminan elementos.
enum Estado { VACIO, OCUPADO, BORRADO };

// Un tweet reducido a sus dos claves de interes.
struct Tweet {
    uint64_t user_id;
    std::string screen_name;
};

// Casilla de la tabla para claves numericas: clave, contador de tweets
// del usuario y estado de la casilla.
struct NodoHashID {
    uint64_t clave;
    int conteo;
    Estado estado;
    NodoHashID() : clave(0), conteo(0), estado(VACIO) {}
};

// Tabla hash de doble hashing con user_id (entero) como clave.
class DoubleHashingID {
private:
    std::vector<NodoHashID> tabla;  // arreglo de casillas de tamano fijo
    int capacidad;                  // N: cantidad total de casillas
    int elementos;                  // cantidad de claves distintas insertadas
    int primo_menor;                // primo menor que N, usado por hash2

    // Primera funcion hash (posicion inicial): h1(k) = k mod N.
    size_t hash1(uint64_t clave) { return clave % capacidad; }

    // Segunda funcion hash (tamano del salto). La forma primo - (k mod primo)
    // garantiza un resultado en [1, primo], es decir, un paso nunca nulo.
    size_t hash2(uint64_t clave) { return primo_menor - (clave % primo_menor); }

public:
    // Crea la tabla con todas las casillas en estado VACIO.
    DoubleHashingID(int cap) {
        capacidad = cap;
        elementos = 0;
        primo_menor = 399989; // primo inmediatamente menor a la capacidad (400009)
        tabla.resize(capacidad);
    }

    // Inserta la clave o incrementa su contador si ya existe. La secuencia
    // de sondeo avanza en saltos de h2(k) casillas a partir de h1(k).
    void insert(uint64_t clave) {
        size_t h1 = hash1(clave);
        size_t h2 = hash2(clave);
        size_t i = 0;

        // Fórmula Double Hashing: H(k, i) = (h1(k) + i * h2(k)) mod N
        while (tabla[(h1 + i * h2) % capacidad].estado == OCUPADO) {
            // La clave ya estaba en la tabla: solo se suma un tweet mas.
            if (tabla[(h1 + i * h2) % capacidad].clave == clave) {
                tabla[(h1 + i * h2) % capacidad].conteo++;
                return;
            }
            i++;
            // Se acota la cantidad de intentos para evitar ciclos infinitos.
            if (i >= capacidad) {
                std::cerr << "Error: Tabla llena o ciclo infinito en IDs." << std::endl;
                return;
            }
        }

        // Casilla libre encontrada: se inserta la clave con contador en 1.
        size_t pos_final = (h1 + i * h2) % capacidad;
        tabla[pos_final].clave = clave;
        tabla[pos_final].conteo = 1;
        tabla[pos_final].estado = OCUPADO;
        elementos++;
    }

    // Cantidad de claves distintas almacenadas (usuarios unicos).
    int getElementosUnicos() const { return elementos; }

    // Suma de todos los contadores; debe coincidir con el total de tweets
    // insertados (se usa como verificacion de correctitud).
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

    // Estimacion del tamano en memoria: suma el tamano de los campos de cada
    // casilla. Es una aproximacion, no la memoria real del proceso.
    int inMemorySize() const {
        size_t size = sizeof(tabla);
        for (const auto& nodo : tabla) {
            size += sizeof(nodo.clave) + sizeof(nodo.conteo) + sizeof(nodo.estado);
        }
        return size;
    }

    // Reinicia todas las casillas para repetir el experimento desde cero.
    void clear() {
        for (auto& nodo : tabla) {
            nodo.estado = VACIO;
            nodo.clave = 0;
            nodo.conteo = 0;
        }
        elementos = 0;
    }
};

// Casilla de la tabla para claves de tipo string (user_screen_name).
struct NodoHashName {
    std::string clave;
    int conteo;
    Estado estado;
    NodoHashName() : clave(""), conteo(0), estado(VACIO) {}
};

// Tabla hash de doble hashing con user_screen_name (string) como clave.
// Usa dos hashes polinomiales independientes (bases 31 y 37).
class DoubleHashingName {
private:
    std::vector<NodoHashName> tabla;
    int capacidad;
    int elementos;
    int primo_menor;

    // Primera funcion hash: polinomial con base p = 31, modulo la capacidad.
    size_t hash1(const std::string& clave) {
        size_t hash = 0, p = 31, p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % capacidad;
            p_pow = (p_pow * p) % capacidad;
        }
        return hash;
    }

    // Segunda funcion hash: polinomial con base p = 37, modulo el primo menor.
    // El resultado primo - hash queda en [1, primo], evitando un paso nulo.
    size_t hash2(const std::string& clave) {
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

    // Misma logica de insercion que en la version por id: sondeo con saltos
    // de h2(k) casillas hasta hallar la clave existente o una casilla libre.
    void insert(const std::string& clave) {
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
                std::cerr << "Error: Tabla llena o ciclo infinito en Names." << std::endl;
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

    // Recorre la tabla y devuelve el usuario con mayor cantidad de tweets.
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

    // Estimacion del tamano en memoria. A diferencia de las otras variantes
    // de hashing cerrado, aqui tambien se suma la capacidad interna reservada
    // por cada std::string (clave.capacity()), por lo que el valor reportado
    // es mayor aunque la estructura sea equivalente.
    int inMemorySize() const {
        size_t size = sizeof(tabla);
        for (const auto& nodo : tabla) {
            size += sizeof(nodo.clave) + nodo.clave.capacity() + sizeof(nodo.conteo) + sizeof(nodo.estado);
        }
        return size;
    }

    // Reinicia todas las casillas para repetir el experimento desde cero.
    void clear() {
        for (auto& nodo : tabla) {
            nodo.estado = VACIO;
            nodo.clave = "";
            nodo.conteo = 0;
        }
        elementos = 0;
    }
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
        tweets.push_back({std::stoull(linea.substr(0, sep)), linea.substr(sep + 1)});
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

    std::vector<Tweet> tweets = leerDataset("usuarios.csv", readCount);

    // Capacidad fija de la tabla: numero primo mayor que el total de tweets,
    // lo que evita redimensionar y mantiene bajo el factor de carga.
    int N = 400009;

    // ------- Experimento 1: tabla hash con user_id como clave -------
    // Se repite la insercion de todos los tweets numExperiments veces,
    // midiendo solo el tiempo del bucle de insercion.
    DoubleHashingID idTable(N);
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

    // ------- Resultados legibles por consola -------
    std::cout << "-----------------------------------------------\n";
    std::cout << "Tabla hash con double hashing (closed hashing)\n";
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
    std::cout << readCount << ";double_hashing;user_id;" << numExperiments << ";"
              << meanTimeIds << ";" << stdDeviationIds << ";" << idTable.inMemorySize() / 1024 << "\n";

    std::cout << readCount << ";double_hashing;user_screen_name;" << numExperiments << ";"
              << meanTimeNames << ";" << stdDeviationNames << ";" << screenNameTable.inMemorySize() / 1024 << "\n";

    return 0;
}
