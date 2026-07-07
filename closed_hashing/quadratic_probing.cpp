// Compilación: g++ closed_hashing/quadratic_probing.cpp -o closed_hashing/quadratic_probing
// Ejecución: ./closed_hashing/quadratic_probing [read_count] [num_experimento]

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

class QuadraticProbingID {
private:
    vector<NodoHashID> tabla;
    int capacidad;
    int elementos;

    size_t hash_id(uint64_t clave) { return clave % capacidad; }

public:
    QuadraticProbingID(int cap) {
        capacidad = cap;
        elementos = 0;
        tabla.resize(capacidad);
    }

    void insertar(uint64_t clave) {
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
                cerr << "Error: Tabla llena o ciclo infinito en IDs." << endl;
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
};

struct NodoHashName {
    string clave;
    int conteo;
    Estado estado;
    NodoHashName() : clave(""), conteo(0), estado(VACIO) {}
};

class QuadraticProbingName {
private:
    vector<NodoHashName> tabla;
    int capacidad;
    int elementos;

    size_t hash_name(const string& clave) {
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

    void insertar(const string& clave) {
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
                cerr << "Error: Tabla llena o ciclo infinito en Names." << endl;
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
    
    QuadraticProbingID tabla_ids(N);
    auto start_id = high_resolution_clock::now();
    for (const Tweet& t : tweets) tabla_ids.insertar(t.user_id);
    auto end_id = high_resolution_clock::now();
    double tiempo_ids_ms = duration_cast<duration<double, milli>>(end_id - start_id).count();

    QuadraticProbingName tabla_names(N);
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
    cout << numExperimento << ";quadratic_probing;user_id;" << readCount << ";" << tiempo_ids_ms << "\n";
    cout << numExperimento << ";quadratic_probing;user_screen_name;" << readCount << ";" << tiempo_names_ms << "\n";

    return 0;
}