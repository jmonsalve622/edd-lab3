#!/bin/bash
# Automatizacion de los experimentos: ejecuta las 5 implementaciones de tabla
# hash (3 de hashing cerrado, 1 de hashing abierto y unordered_map) para
# tamanos de entrada crecientes, y acumula las lineas de resultados en un CSV.
#
# Requiere que los 5 programas esten compilados y que exista usuarios.csv
# (generado por preprocesamiento.py).
#
# Uso: ./ejecutar_experimentos.sh

# Archivo donde se acumulan los resultados; se vacia al comenzar.
ARCHIVO_SALIDA="resultados.csv"
> $ARCHIVO_SALIDA

echo "Iniciando pruebas automatizadas (100 repeticiones por cada N)..."

# Tamanos de entrada: de 10.000 a 180.000 tweets en pasos de 10.000.
for tweets in {10000..180000..10000}
do
    echo "Procesando $tweets tweets..."

    # Cada programa recibe [repeticiones] [cantidad de tweets] e imprime,
    # ademas del resumen legible, lineas en formato CSV separado por ";".
    # El grep ";" filtra solo esas lineas hacia el archivo de salida.
    ./closed_hashing/linear_probing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./closed_hashing/quadratic_probing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./closed_hashing/double_hashing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./open_hashing/separate_chaining 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./unordered_map/unordered_map 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
done

# Elimina lineas duplicadas conservando el orden original.
awk '!seen[$0]++ || NR==1' $ARCHIVO_SALIDA > temp.csv && mv temp.csv $ARCHIVO_SALIDA

echo "Datos guardados en $ARCHIVO_SALIDA"
