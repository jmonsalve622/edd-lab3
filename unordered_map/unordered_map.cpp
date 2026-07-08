// Complición: g++ unordered_map/unordered_map.cpp -o unordered_map/unordered_map
// Ejecución: ./unordered_map/unordered_map [num_experiments] [read_count]

#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>
#include <chrono>

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

int inMemorySize(const std::unordered_map<uint64_t, int>& table) {
    size_t size = sizeof(table);
    for (const auto& pair : table) {
        size += sizeof(pair.first) + sizeof(pair.second);
    }
    return size;
}

int inMemorySize(const std::unordered_map<std::string, int>& table) {
    size_t size = sizeof(table);
    for (const auto& pair : table) {
        size += sizeof(pair.first) + pair.first.capacity() + sizeof(pair.second);
    }
    return size;
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

    std::vector<Tweet> tweets = leerDataset("./usuarios.csv", readCount);

    // Tabla hash con user_id como clave
    std::unordered_map<uint64_t, int> idTable;
    double timeSumIds = 0;
    std::vector<double> timesIds;
    for (int i = 0; i < numExperiments; i++) {
        auto start_id = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            auto it = idTable.find(t.user_id);
            if (it != idTable.end()) {
                it->second = it->second + 1;
            } else {
                idTable[t.user_id] = 1;
            }
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
    std::unordered_map<std::string, int> screenNameTable;
    double timeSumNames = 0;
    std::vector<double> timesNames;
    for (int i = 0; i < numExperiments; i++) {
        auto start_name = std::chrono::high_resolution_clock::now();
        for (const Tweet& t : tweets) {
            auto it = screenNameTable.find(t.screen_name);
            if (it != screenNameTable.end()) {
                it->second = it->second + 1;
            } else {
                screenNameTable[t.screen_name] = 1;
            }
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
    std::cout << "Tabla hash con unordered_map\n";
    std::cout << "-----------------------------------------------\n";
    std::cout << "Experimentos realizados: " << numExperiments << "\n";
    std::cout << "Tweets leídos: " << tweets.size() << "\n\n";

    std::cout << " --- Tabla hash con user_id como clave ---\n";
    std::cout << "Usuarios únicos: " << idTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << inMemorySize(idTable) / 1024 << " KB\n";
    std::cout << "Tiempo de inserción promedio: " << int(meanTimeIds) << " μs\n";
    std::cout << "Desviación estandar: " << int(stdDeviationIds) << " μs\n\n";

    std::cout << " --- Tabla hash con user_screen_name como clave ---\n";
    std::cout << "Usuarios únicos: " << screenNameTable.size() << "\n";
    std::cout << "Tamaño en memoria: " << inMemorySize(screenNameTable) / 1024 << " KB\n";
    std::cout << "Tiempo de inserción promedio: " << int(meanTimeNames) << " μs\n";
    std::cout << "Desviación estandar: " << int(stdDeviationNames)<< " μs\n\n";

    // --- LÍNEAS PARA EL SCRIPT DE BASH ---
    std::cout << readCount << ";unordered_map;user_id;" << numExperiments << ";" 
              << meanTimeIds << ";" << stdDeviationIds << ";" << inMemorySize(idTable) / 1024 << "\n";
              
    std::cout << readCount << ";unordered_map;user_screen_name;" << numExperiments << ";" 
              << meanTimeNames << ";" << stdDeviationNames << ";" << inMemorySize(screenNameTable) / 1024 << "\n";

    return 0;
}
