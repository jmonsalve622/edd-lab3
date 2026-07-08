#!/bin/bash

ARCHIVO_SALIDA="resultados.csv"
> $ARCHIVO_SALIDA

echo "Iniciando pruebas automatizadas (100 repeticiones por cada N)..."

for tweets in {10000..180000..10000}
do
    echo "Procesando $tweets tweets..."
    
    ./closed_hashing/linear_probing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./closed_hashing/quadratic_probing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./closed_hashing/double_hashing 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./open_hashing/separate_chaining 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
    ./unordered_map/unordered_map 100 $tweets | grep ";" >> $ARCHIVO_SALIDA
done

awk '!seen[$0]++ || NR==1' $ARCHIVO_SALIDA > temp.csv && mv temp.csv $ARCHIVO_SALIDA

echo "Datos guardados en $ARCHIVO_SALIDA"