# Preprocesamiento del dataset auspol2019.csv
#
# Extrae solo las columnas necesarias para el experimento (user_id y
# user_screen_name) y genera un archivo plano "usuarios.csv" con una linea
# por tweet, en formato:
#
#     user_id;user_screen_name
#
# Esto permite que los programas en C++ lean el dataset con un getline
# simple, sin necesidad de un parser CSV que maneje campos entre comillas
# con saltos de linea internos.
#
# Se descartan las filas malformadas (aquellas que no tienen exactamente
# 11 campos), producto de saltos de linea sin escapar en user_description.
#
# Uso: python preprocesamiento.py

import csv

ENTRADA = "auspol2019.csv"
SALIDA = "usuarios.csv"

# Indices de las columnas de interes en el CSV original
COL_USER_ID = 5
COL_SCREEN_NAME = 7
NUM_COLUMNAS = 11


def main():
    validos = 0
    descartados = 0
    ids_unicos = set()

    with open(ENTRADA, encoding="utf-8", newline="") as f_in, \
         open(SALIDA, "w", encoding="utf-8", newline="\n") as f_out:
        lector = csv.reader(f_in)
        next(lector)  # saltar encabezado

        for fila in lector:
            if len(fila) != NUM_COLUMNAS:
                descartados += 1
                continue
            user_id = fila[COL_USER_ID].strip()
            screen_name = fila[COL_SCREEN_NAME].strip()
            # user_id debe ser numerico y screen_name no vacio
            if not user_id.isdigit() or not screen_name:
                descartados += 1
                continue
            f_out.write(f"{user_id};{screen_name}\n")
            validos += 1
            ids_unicos.add(user_id)

    print(f"Tweets validos escritos : {validos}")
    print(f"Filas descartadas       : {descartados}")
    print(f"Usuarios unicos         : {len(ids_unicos)}")
    print(f"Archivo generado        : {SALIDA}")


if __name__ == "__main__":
    main()
