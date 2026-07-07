// Compilación: g++ closed_hashing/double_hashing.cpp -o closed_hashing/double_hashing
// Ejecución: ./closed_hashing/double_hashing [read_count] [num_experimento]

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <chrono>
#include <cstdint>

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

    void insertar(uint64_t clave) {
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

    void insertar(const string& clave) {
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

int main(int argc, char* argv[]) {
    int readCount = (argc > 1) ? stoi(argv[1]) : 183361;
    int numExperimento = (argc > 2) ? stoi(argv[2]) : 1;

    if (readCount < 1 || readCount > 183361) {
        cerr << "Error: readCount debe ser un valor entre 1 y 183361\n";
        return 1;
    }

    vector<Tweet> tweets = leerDataset("usuarios.csv", readCount);
    
    int N = 400009; 
    
    DoubleHashingID tabla_ids(N);
    auto start_id = high_resolution_clock::now();
    for (const Tweet& t : tweets) tabla_ids.insertar(t.user_id);
    auto end_id = high_resolution_clock::now();
    double tiempo_ids_ms = duration_cast<duration<double, milli>>(end_id - start_id).count();

    DoubleHashingName tabla_names(N);
    auto start_name = high_resolution_clock::now();
    for (const Tweet& t : tweets) tabla_names.insertar(t.screen_name);
    auto end_name = high_resolution_clock::now();
    double tiempo_names_ms = duration_cast<duration<double, milli>>(end_name - start_name).count();


    cout << "Tweets leidos: " << tweets.size() << "\n\n";
    cout << "Usuarios unicos (por user_id)     : " << tabla_ids.getElementosUnicos() << "\n";
    cout << "Usuarios unicos (por screen_name) : " << tabla_names.getElementosUnicos() << "\n";
    cout << "Suma de contadores (por user_id)  : " << tabla_ids.getSumaContadores() << "\n";
    cout << "Suma de contadores (screen_name)  : " << tabla_names.getSumaContadores() << "\n\n";
    int topCuenta = 0;
    string topUsuario = tabla_names.getUsuarioMasActivo(topCuenta);
    cout << "Usuario mas activo: @" << topUsuario << " con " << topCuenta << " tweets\n\n";

    if (readCount == 10000 && numExperimento == 1) {
        cout << "numero_experimento;dataset;estructura_de_datos;tipo_clave;cantidad_consultas;tiempo_ejecucion_ms\n";
    }
    cout << numExperimento << ";double_hashing;user_id;" << readCount << ";" << tiempo_ids_ms << "\n";
    cout << numExperimento << ";double_hashing;user_screen_name;" << readCount << ";" << tiempo_names_ms << "\n";

    return 0;
}