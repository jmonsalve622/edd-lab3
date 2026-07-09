# Laboratorio 3 — Estructuras de Datos: Tablas Hash

Estudio experimental que compara el rendimiento de cinco implementaciones de
tablas hash, utilizadas para contabilizar la cantidad de tweets publicados por
cada usuario en un dataset real de Twitter (elecciones australianas de 2019,
`#auspol`). Cada estructura se evalúa con dos tipos de clave:

- **`user_id`**: identificador numérico del usuario (entero de 64 bits).
- **`user_screen_name`**: nombre de usuario (cadena de caracteres).

## Estructuras evaluadas

| Estructura | Tipo | Archivo |
|---|---|---|
| Separate Chaining | Hashing abierto (encadenamiento) | `open_hashing/separate_chaining.cpp` |
| Linear Probing | Hashing cerrado | `closed_hashing/linear_probing.cpp` |
| Quadratic Probing | Hashing cerrado | `closed_hashing/quadratic_probing.cpp` |
| Double Hashing | Hashing cerrado | `closed_hashing/double_hashing.cpp` |
| `std::unordered_map` | Referencia STL | `unordered_map/unordered_map.cpp` |

Las variantes de hashing cerrado usan una tabla fija de 400.009 casillas
(número primo) y hash polinomial para las claves de tipo string. Separate
chaining dimensiona su tabla al 80% de los registros procesados y usa
`std::hash<std::string>`.

## Requisitos

- `g++` (C++11 o superior)
- Python 3 con `pandas`, `matplotlib` y `seaborn` (solo para los gráficos)
- Bash (para el script de automatización)
- El dataset `auspol2019.csv` en la raíz del repositorio

## Uso

### 1. Preprocesar el dataset

Extrae las columnas `user_id` y `user_screen_name`, descarta filas malformadas
y genera `usuarios.csv` (183.361 tweets válidos, 45.861 usuarios únicos):

```bash
python preprocesamiento.py
```

### 2. Compilar las implementaciones

```bash
g++ closed_hashing/linear_probing.cpp -o closed_hashing/linear_probing
g++ closed_hashing/quadratic_probing.cpp -o closed_hashing/quadratic_probing
g++ closed_hashing/double_hashing.cpp -o closed_hashing/double_hashing
g++ open_hashing/separate_chaining.cpp -o open_hashing/separate_chaining
g++ unordered_map/unordered_map.cpp -o unordered_map/unordered_map
```

### 3. Ejecutar un experimento individual

Cada programa recibe la cantidad de repeticiones y la cantidad de tweets a
procesar, e imprime el tiempo promedio de inserción (μs), la desviación
estándar y una estimación del tamaño en memoria:

```bash
./closed_hashing/linear_probing 100 180000
```

### 4. Ejecutar todos los experimentos

Corre las 5 implementaciones con 100 repeticiones para tamaños de entrada
desde 10.000 hasta 180.000 tweets (pasos de 10.000) y acumula los resultados
en `resultados.csv`:

```bash
./ejecutar_experimentos.sh
```

### 5. Generar los gráficos

Produce 4 gráficos PNG (tiempo y memoria, para cada tipo de clave) a partir
de `resultados.csv`:

```bash
python generar_graficos.py
```

## Estructura del repositorio

```
├── auspol2019.csv          # Dataset original (tweets #auspol 2019)
├── preprocesamiento.py     # Limpieza del dataset -> usuarios.csv
├── usuarios.csv            # Dataset preprocesado (user_id;screen_name)
├── closed_hashing/         # Linear, quadratic y double hashing
├── open_hashing/           # Separate chaining
├── unordered_map/          # Referencia con la STL
├── ejecutar_experimentos.sh# Automatización de los experimentos
├── resultados.csv          # Resultados experimentales (formato ;)
├── generar_graficos.py     # Generación de gráficos desde resultados.csv
└── grafico_*.png           # Gráficos de tiempo y memoria
```

## Resultados

Los resultados completos están en `resultados.csv` con el formato
`n;estructura;clave;repeticiones;tiempo_promedio_us;desviacion_us;memoria_kb`,
y los gráficos comparativos en los archivos `grafico_*.png`. En síntesis:
con claves numéricas el hashing cerrado (quadratic y linear probing) obtiene
los menores tiempos, mientras que con claves de tipo string el mejor
desempeño corresponde a separate chaining.
