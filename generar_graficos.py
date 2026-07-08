import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Mantenemos el estilo visual bonito
sns.set_theme(style="whitegrid")

def graficar_datos():
    print("Leyendo datos del archivo CSV...")
    nombres_columnas = [
        'cantidad_consultas', 'estructura_de_datos', 'tipo_clave', 
        'repeticiones', 'tiempo_promedio_us', 'desviacion_estandar_us', 'memoria_kb'
    ]
    
    df = pd.read_csv('resultados.csv', sep=';', names=nombres_columnas)

    # Forzamos que sean valores numéricos para evitar cualquier error de tipo
    for col in ['cantidad_consultas', 'tiempo_promedio_us', 'memoria_kb']:
        df[col] = pd.to_numeric(df[col], errors='coerce')

    df_id = df[df['tipo_clave'] == 'user_id']
    df_name = df[df['tipo_clave'] == 'user_screen_name']
    
    # Obtenemos la lista de las 5 estructuras para iterar sobre ellas
    estructuras = df['estructura_de_datos'].unique()

    # --- 1. GRÁFICO DE TIEMPOS (USER_ID) ---
    print("Generando gráfico de tiempos para user_id...")
    plt.figure(figsize=(10, 6))
    for est in estructuras:
        subset = df_id[df_id['estructura_de_datos'] == est]
        # Usamos .values para pasar arreglos puros y evitar el error de Pandas
        plt.plot(subset['cantidad_consultas'].values, subset['tiempo_promedio_us'].values, marker='o', linewidth=2, label=est)
    
    plt.title('Tiempo de Ejecución vs Tweets (Clave: user_id)', fontsize=14)
    plt.xlabel('Cantidad de Tweets Procesados', fontsize=12)
    plt.ylabel('Tiempo (Microsegundos [us])', fontsize=12)
    plt.xticks(range(10000, 190000, 20000))
    plt.legend(title='Estructura')
    plt.tight_layout()
    plt.savefig('grafico_tiempos_id.png', dpi=300)

    # --- 2. GRÁFICO DE TIEMPOS (USER_SCREEN_NAME) ---
    print("Generando gráfico de tiempos para user_screen_name...")
    plt.figure(figsize=(10, 6))
    for est in estructuras:
        subset = df_name[df_name['estructura_de_datos'] == est]
        plt.plot(subset['cantidad_consultas'].values, subset['tiempo_promedio_us'].values, marker='o', linewidth=2, label=est)
        
    plt.title('Tiempo de Ejecución vs Tweets (Clave: user_screen_name)', fontsize=14)
    plt.xlabel('Cantidad de Tweets Procesados', fontsize=12)
    plt.ylabel('Tiempo (Microsegundos [us])', fontsize=12)
    plt.xticks(range(10000, 190000, 20000))
    plt.legend(title='Estructura')
    plt.tight_layout()
    plt.savefig('grafico_tiempos_name.png', dpi=300)

    # --- 3. GRÁFICO DE TAMAÑOS EN MEMORIA (USER_ID) ---
    print("Generando gráfico de tamaños para user_id...")
    plt.figure(figsize=(10, 6))
    for est in estructuras:
        subset = df_id[df_id['estructura_de_datos'] == est]
        plt.plot(subset['cantidad_consultas'].values, subset['memoria_kb'].values, marker='o', linewidth=2, label=est)
        
    plt.title('Consumo de Memoria vs Tweets (Clave: user_id)', fontsize=14)
    plt.xlabel('Cantidad de Tweets Procesados', fontsize=12)
    plt.ylabel('Tamaño en Memoria (KB)', fontsize=12)
    plt.xticks(range(10000, 190000, 20000))
    plt.legend(title='Estructura')
    plt.tight_layout()
    plt.savefig('grafico_memoria_id.png', dpi=300)

    # --- 4. GRÁFICO DE TAMAÑOS EN MEMORIA (USER_SCREEN_NAME) ---
    print("Generando gráfico de tamaños para user_screen_name...")
    plt.figure(figsize=(10, 6))
    for est in estructuras:
        subset = df_name[df_name['estructura_de_datos'] == est]
        plt.plot(subset['cantidad_consultas'].values, subset['memoria_kb'].values, marker='o', linewidth=2, label=est)
        
    plt.title('Consumo de Memoria vs Tweets (Clave: user_screen_name)', fontsize=14)
    plt.xlabel('Cantidad de Tweets Procesados', fontsize=12)
    plt.ylabel('Tamaño en Memoria (KB)', fontsize=12)
    plt.xticks(range(10000, 190000, 20000))
    plt.legend(title='Estructura')
    plt.tight_layout()
    plt.savefig('grafico_memoria_name.png', dpi=300)

    print("¡Gráficos generados con éxito!")

if __name__ == '__main__':
    graficar_datos()