// Tabla hash con hashing cerrado y resolucion de colisiones por sondeo lineal
// (linear probing). Todos los elementos se almacenan directamente en la tabla;
// ante una colision se prueba la casilla siguiente: H(k, i) = (h(k) + i) mod N.
//
// Compilación: g++ closed_hashing/linear_probing.cpp -o closed_hashing/linear_probing
// Ejecución: ./closed_hashing/linear_probing [num_experiments] [read_count]

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

// Casilla de la tabla para claves numericas: guarda la clave (user_id),
// el contador de tweets del usuario y el estado de la casilla.
struct NodoHashID {
    uint64_t clave;
    int conteo;
    Estado estado;
    NodoHashID() : clave(0), conteo(0), estado(VACIO) {}
};

// Tabla hash de sondeo lineal con user_id (entero) como clave.
class LinearProbingID {
private:
    std::vector<NodoHashID> tabla;  // arreglo de casillas de tamano fijo
    int capacidad;                  // N: cantidad total de casillas
    int elementos;                  // cantidad de claves distintas insertadas

    // Funcion hash por division: h(k) = k mod N.
    size_t hash_id(uint64_t clave) {
        return clave % capacidad;
    }

public:
    // Crea la tabla con todas las casillas en estado VACIO.
    LinearProbingID(int cap) {
        capacidad = cap;
        elementos = 0;
        tabla.resize(capacidad);
    }

    // Inserta la clave o incrementa su contador si ya existe.
    // Recorre la secuencia de sondeo lineal hasta encontrar la clave
    // (colision con la misma clave) o una casilla libre.
    void insert(uint64_t clave) {
        size_t indice = hash_id(clave);
        size_t i = 0;

        // Avanza de a una casilla mientras encuentre posiciones ocupadas.
        while (tabla[(indice + i) % capacidad].estado == OCUPADO) {
            // La clave ya estaba en la tabla: solo se suma un tweet mas.
            if (tabla[(indice + i) % capacidad].clave == clave) {
                tabla[(indice + i) % capacidad].conteo++;
                return;
            }
            i++;
            if (i == capacidad) return; // tabla llena: no hay donde insertar
        }

        // Casilla libre encontrada: se inserta la clave con contador en 1.
        size_t pos_final = (indice + i) % capacidad;
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

// Tabla hash de sondeo lineal con user_screen_name (string) como clave.
// Identica en estructura a LinearProbingID, salvo por la funcion hash.
class LinearProbingName {
private:
    std::vector<NodoHashName> tabla;
    int capacidad;
    int elementos;

    // Hash polinomial con base p = 31: trata la cadena como un numero en
    // base 31 y lo reduce modulo la capacidad de la tabla.
    size_t hash_name(const std::string& clave) {
        size_t hash = 0;
        size_t p = 31;
        size_t p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % capacidad;
            p_pow = (p_pow * p) % capacidad;
        }
        return hash;
    }

public:
    LinearProbingName(int cap) {
        capacidad = cap;
        elementos = 0;
        tabla.resize(capacidad);
    }

    // Misma logica de insercion que en la version por id: sondeo lineal
    // hasta hallar la clave existente o la primera casilla libre.
    void insert(const std::string& clave) {
        size_t indice = hash_name(clave);
        size_t i = 0;

        while (tabla[(indice + i) % capacidad].estado == OCUPADO) {
            if (tabla[(indice + i) % capacidad].clave == clave) {
                tabla[(indice + i) % capacidad].conteo++;
                return;
            }
            i++;
            if (i == capacidad) return;
        }

        size_t pos_final = (indice + i) % capacidad;
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

    // Estimacion del tamano en memoria (no incluye la capacidad interna
    // reservada por cada std::string).
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
    LinearProbingID idTable(N);
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
    LinearProbingName screenNameTable(N);
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
    std::cout << "Tabla hash con linear probing (closed hashing)\n";
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
    std::cout << readCount << ";linear_probing;user_id;" << numExperiments << ";"
              << meanTimeIds << ";" << stdDeviationIds << ";" << idTable.inMemorySize() / 1024 << "\n";

    std::cout << readCount << ";linear_probing;user_screen_name;" << numExperiments << ";"
              << meanTimeNames << ";" << stdDeviationNames << ";" << screenNameTable.inMemorySize() / 1024 << "\n";

    return 0;
}
