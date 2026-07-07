#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>

using namespace std;
using namespace std::chrono;


enum Estado { VACIO, OCUPADO, BORRADO };

struct NodoHash {
    string clave;
    int conteo;
    Estado estado;

    NodoHash() : clave(""), conteo(0), estado(VACIO) {}
};

class LinearProbing {
private:
    vector<NodoHash> tabla;
    int capacidad;
    int elementos;
    bool usar_hash_id;

    // Funcion para sacar user_id en resultados
    size_t hash_id(const string& clave) {
        unsigned long long id = stoull(clave);
        return id % capacidad;
    }

    // Funcion para sacar user_name en resultados
    size_t hash_name(const string& clave) {
        size_t hash = 0;
        size_t p = 31;
        size_t p_pow = 1;
        for (char c : clave) {
            hash = (hash + (c - 'a' + 1) * p_pow) % capacidad;
            p_pow = (p_pow * p) % capacidad;
        }
        return hash;
    }

    // Funcion hash
    size_t hash(const string& clave) {
        if (usar_hash_id) return hash_id(clave);
        return hash_name(clave);
    }

public:
    LinearProbing(int cap, bool es_id) {
        capacidad = cap;
        elementos = 0;
        usar_hash_id = es_id;
        tabla.resize(capacidad);
    }

    // Insercion y conteo de los tweets
    void insertar(const string& clave) {
        size_t indice = hash(clave);
        size_t i = 0;

        while (tabla[(indice + i) % capacidad].estado == OCUPADO) {
            if (tabla[(indice + i) % capacidad].clave == clave) {
                tabla[(indice + i) % capacidad].conteo++;
                return;
            }

            i++;
            
            if (i == capacidad) {
                cerr << "Error: La tabla Hash está llena." << endl;
                return;
            }
        }

        size_t pos_final = (indice + i) % capacidad;
        tabla[pos_final].clave = clave;
        tabla[pos_final].conteo = 1;
        tabla[pos_final].estado = OCUPADO;
        elementos++;
    }
};


// Main principal de linear_probing.cpp
int main() {
    string archivo_csv = "../usuarios.csv";
    ifstream archivo(archivo_csv);

    if (!archivo.is_open()) {
        cerr << "Error al abrir el archivo: " << archivo_csv << endl;
        return 1;
    }

    int N = 400009; 
    LinearProbing tabla_ids(N, true);
    LinearProbing tabla_names(N, false);

    string linea, user_id, screen_name;
    int num_tweets_procesados = 0;

    double tiempo_ids_ms = 0.0;
    double tiempo_names_ms = 0.0;

    cout << "numero de tweets, tipo de clave, tiempo en ms" << endl;

    // Printeo de resultados de numero de tweets, la clave y el tiempo de ejecucion
    while (getline(archivo, linea)) {
        stringstream ss(linea);
        getline(ss, user_id, ';');
        getline(ss, screen_name, ';');

        auto start_id = high_resolution_clock::now();
        tabla_ids.insertar(user_id);
        auto end_id = high_resolution_clock::now();
        tiempo_ids_ms += duration_cast<duration<double, milli>>(end_id - start_id).count();

        auto start_name = high_resolution_clock::now();
        tabla_names.insertar(screen_name);
        auto end_name = high_resolution_clock::now();
        tiempo_names_ms += duration_cast<duration<double, milli>>(end_name - start_name).count();

        num_tweets_procesados++;

        if (num_tweets_procesados % 10000 == 0) {
            cout << num_tweets_procesados << ";user_id;" << tiempo_ids_ms << endl;
            cout << num_tweets_procesados << ";user_name;" << tiempo_names_ms << endl;
        }
    }

    archivo.close();
    return 0;
}