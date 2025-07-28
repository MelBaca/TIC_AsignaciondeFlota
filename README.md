# MDVSP - Asignación de Flota de Vehículos con Múltiples Depósitos

Este repositorio contiene la implementación de dos modelos matemáticos para resolver el problema de asignación de flota de vehículos con múltiples depósitos (MDVSP, por sus siglas en inglés). El proyecto está organizado en dos carpetas principales:

## 📁 Estructura del Repositorio
├── CODIGO/
│ ├── GeneracionColumnas
├── INSTANCIAS/
│ ├── instancia1_Viajes.csv
│ ├── instancia1_terminales.csv
│ ├── ...
└── README.md

- **`INSTANCIAS/`**: Contiene los archivos `.csv` que representan los datos de entrada utilizados para probar los modelos. Cada archivo define una instancia del MDVSP.

- **`CODIGO/`**: Incluye el código fuente en C++ que implementa los modelos matemáticos y sus cotas. Aquí se encuentran:
  - El **modelo basado en arcos**, que representa el problema mediante una formulación orientada a arcos.
  - El **modelo basado en rutas**, que utiliza una representación por rutas completas asignables a los vehículos.
  - La implementación de **cotas inferiores** para ambos enfoques.

## 🧠 Descripción del Problema

El **MDVSP (Multi-Depot Vehicle Scheduling Problem)** es un problema clásico de optimización en el que se busca asignar vehículos a  viajes minimizando el costo total, considerando que existen múltiples depósitos de donde los vehículos pueden partir y retornar.

Este problema tiene aplicaciones directas en la planificación de flotas en sistemas de transporte público.

## 🛠️ Tecnologías y Herramientas

- **Lenguaje de programación**: C++
- **Entorno de desarrollo**: [Visual Studio 2022](https://visualstudio.microsoft.com/)
- **Solver**: [Gurobi Optimizer](https://www.gurobi.com/)
  - Se configuró correctamente la licencia de Gurobi para su uso con Visual Studio.

## 🚀 Ejecución

1. **Configurar Visual Studio 2022**:
   - Asegúrate de tener instalado Visual Studio con soporte para C++.
   - Configura la licencia de Gurobi y enlaza las bibliotecas necesarias en tu entorno.

2. **Compilar el proyecto**:
   - Abre los archivos `.cpp` dentro de la carpeta `CODIGO/`.
   - Compílalos y ejecútalos desde Visual Studio asegurándote de que Gurobi esté correctamente referenciado.

3. **Probar con instancias**:
   - Los archivos `.csv` en la carpeta `INSTANCIAS/` sirven como entrada para los modelos.
   - Asegúrate de indicar la ruta correcta a las instancias al correr el programa.

## Resultados y Cotas

El modelo de arcos genera soluciones óptimas para el MDVSP, y de los otros modelos se obtiene cotas inferiores. 

---

**Desarrollado por:**  
Gitty Melany Baca Vásconez  
melcogity12@gmail.com
Escuela Politécnica Nacional


