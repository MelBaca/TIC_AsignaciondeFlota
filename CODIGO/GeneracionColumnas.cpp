
#include "gurobi_c++.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <map>
#include <stack>
#include <queue>
#include <limits>
#include <iomanip>
#include <set>
#include <cmath>
#include <chrono>

using namespace std;


#define MIN_ESPERA 3
#define COSTO_FIJO_RUTA 10
#define INF numeric_limits<double>::infinity()

// FUNCIÓN PARA PASAR DE UN STRING A MINUTOS
int horaTextoAMinutos(const string& hora_str) {
    int horas = 0, minutos = 0, segundos = 0;
    char sep1, sep2;
    stringstream ss(hora_str);
    ss >> horas >> sep1 >> minutos;
    if (ss.peek() == ':') {
        ss >> sep2 >> segundos;
    }
    return horas * 60 + minutos;
}

// CLASE ARCO
class Arco {
private:
    int origen;
    int destino;
    int deposito; 
public:
    // Constructor
    Arco(int i, int j, int k) : origen(i), destino(j), deposito(k) {}

    // Getters
    int getOrigen() const { return origen; }
    int getDestino() const { return destino; }
    int getDeposito() const { return deposito; }

    // Sobrecarga del operador == para comparar arcos
    bool operator==(const Arco& otro) const {
        return origen == otro.origen && destino == otro.destino && deposito == otro.deposito;
    }

    // Función para imprimir valores del objeto
    void imprimir() const {
        cout << "Arco [Origen: " << origen << ", Destino: " << destino
            << ", Deposito: " << deposito << "]" << endl;
    }

    bool operator<(const Arco& other) const {
        if (origen != other.origen) return origen < other.origen;
        if (destino != other.destino) return destino < other.destino;
        return deposito < other.deposito;
    }
};

// Especialización de la función hash para la clase Arco
namespace std {
    template <>
    struct hash<Arco> {
        size_t operator()(const Arco& a) const {
            return hash<int>()(a.getOrigen()) ^
                (hash<int>()(a.getDestino()) << 1) ^
                (hash<int>()(a.getDeposito()) << 2);
        }
    };
}

// Sobrecarga del operador << para la clase Arco
ostream& operator<<(ostream& os, const Arco& arco) {
    os << "(" << arco.getOrigen() << "," << arco.getDestino() << "," << arco.getDeposito() << ")";
    return os;
}

// Sobrecarga del operador << para list<Arco>
ostream& operator<<(ostream& os, const list<Arco>& lista_arcos) {
    os << "[";
    bool primero = true;
    for (const auto& arco : lista_arcos) {
        if (!primero) os << ", ";
        os << arco;
        primero = false;
    }
    os << "]";
    return os;
}

// Sobrecarga del operador << para vector<list<Arco>>
ostream& operator<<(ostream& os, const vector<list<Arco>>& vec_listas) {
    os << "{\n";
    for (size_t i = 0; i < vec_listas.size(); i++) {
        os << "  Nodo " << (i + 1) << ": " << vec_listas[i]; // +1 porque los nodos van de 1 a n+2d
        if (i < vec_listas.size() - 1) os << ",";
        os << "\n";
    }
    os << "}";
    return os;
}

// Sobrecarga del operador << para unordered_map<Arco, double>
ostream& operator<<(ostream& os, const unordered_map<Arco, double>& mapa_costos) {
    os << "{\n";
    bool primero = true;
    for (const auto& par : mapa_costos) {
        if (!primero) os << ",";
        os << "  " << par.first << " : " << par.second;
        primero = false;
    }
    os << "\n}";
    return os;
}

// CLASE TERMINAL
class Terminal {
private:
    int id_terminal;
    string nombre;
    int num_buses;

public:
    //Constructor: función para inicializar los atributos del objeto.
    Terminal(int id, string n, int buses) : id_terminal(id), nombre(n), num_buses(buses) {}

    // Para obener los valores del objeto
    // const al final es para no modificar el objeto
    int getId() const { return id_terminal; }
    string getNombre() const { return nombre; }
    int getNumBuses() const { return num_buses; }

    //Void : esta función no devuelve ningún valor
    void imprimir() const {
        cout << "Terminal [ID: " << id_terminal << ", Nombre: " << nombre
            << ", Buses: " << num_buses << "]" << endl;
    }
};


// CLASE VIAJE
class Viaje {
private:
    int id_viaje;
    int id_terminal_salida;
    int id_terminal_llegada;
    int tiempo_inicio;
    int tiempo_fin;
    string nom_circuito;

public:
    Viaje(int id, int salida, int llegada, int t_inicio, int t_fin, string circuito)
        : id_viaje(id), id_terminal_salida(salida), id_terminal_llegada(llegada),
        tiempo_inicio(t_inicio), tiempo_fin(t_fin), nom_circuito(circuito) {
    }

    int getId() const { return id_viaje; }
    int getTerminalSalida() const { return id_terminal_salida; }
    int getTerminalLlegada() const { return id_terminal_llegada; }
    int getTiempoInicio() const { return tiempo_inicio; }
    int getTiempoFin() const { return tiempo_fin; }
    string getCircuito() const { return nom_circuito; }

    
    void imprimir() const {
        cout << "Viaje [ID: " << id_viaje
            << ", Salida: " << id_terminal_salida
            << ", Llegada: " << id_terminal_llegada
            << ", T.Inicio: " << tiempo_inicio
            << ", T.Fin: " << tiempo_fin
            << ", Circuito: " << nom_circuito << "]" << endl;
    }
    /////////////////////////////// CAMBIÉ /////////////////////////////////////////////////
    // Función para usar < con el objeto viajes, Viaje& es para acceder directamente al objeto original y no hacer una copia
    bool operator<(const Viaje& otro) const {
        if (tiempo_inicio != otro.tiempo_inicio) {
            return tiempo_inicio < otro.tiempo_inicio;
        }
        return id_viaje < otro.id_viaje;  // Desempate
    }

    // Operador << para imprimir con cout
    friend ostream& operator<<(ostream& os, const Viaje& viaje) {
        os << "Viaje[" << viaje.id_viaje << ": "
            << viaje.id_terminal_salida << "->" << viaje.id_terminal_llegada
            << " (" << viaje.tiempo_inicio << "-" << viaje.tiempo_fin << ") "
            << viaje.nom_circuito << "]";
        return os;
    }
};


// CLASE RESULTADO DE CAMINO MÁS CORTO
class ResultadoCamino {
private:
    double distancia;
    list<Arco> camino;
    bool existe_camino;

public:
    ResultadoCamino() : distancia(INF), existe_camino(false) {}

    ResultadoCamino(double dist, const list<Arco>& path)
        : distancia(dist), camino(path), existe_camino(true) {
    }

    double getDistancia() const { return distancia; }
    list<Arco> getCamino() const { return camino; }
    bool existeCamino() const { return existe_camino; }

    void imprimir() const {
        if (!existe_camino) {
            cout << "No existe camino" << endl;
            return;
        }
        //cout << "Distancia: " << distancia << endl;
        cout << "Camino: ";
        bool primero = true;
        for (const auto& arco : camino) {
            if (!primero) cout << " , ";
            cout << arco.getOrigen();
            primero = false;
        }
        if (!camino.empty()) {
            cout << " , " << camino.back().getDestino();
        }
        cout << endl;
    }
};

//CLASE MULTIGRAFO
class Multigrafo_vsp {
private:
    int n;  //num viajes
    int d;  // num depósitos

    vector < list < Arco > > arcos_sal;  // (i,j,k)
    vector < list < Arco > > arcos_ent;  // (i,j,k)

    // Matriz de costos c_primal[i,j,k]
    unordered_map<Arco, double> c_primal;
    unordered_map<Arco, double> c_original;

    vector<Terminal> terminales;
    vector<Viaje> viajes;

public:
    // Constructor
    Multigrafo_vsp(int num_viajes, int num_depositos) : n(num_viajes), d(num_depositos) {
        // Llama a una funci�n
        inicializarEstructuras();
    }

    int getn() const { return n; }
    int getd() const { return d; }
    const vector<list<Arco>>& getArcos_sal() const { return arcos_sal; }
    const vector<list<Arco>>& getArcos_ent() const { return arcos_ent; }
    const unordered_map<Arco, double>& getC_primal() const { return c_primal; }
    const unordered_map<Arco, double>& getC_original() const { return c_original; }
    const vector<Terminal>& getTerminales() const { return terminales; }
    const vector<Viaje>& getViajes() const { return viajes; }

    void inicializarEstructuras() {
        // Crear arreglos de arcos entrantes y salientes para cada nodo
        arcos_sal.clear();
        arcos_ent.clear();
        arcos_sal.resize(n + 2 * d);
        arcos_ent.resize(n + 2 * d);

    }

    void leerTerminales(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            throw runtime_error("No se pudo abrir el archivo: " + archivo);
        }

        string linea;
        // Leer la primera línea (encabezados)
        getline(file, linea);

        // Leer cada línea de datos
        int linea_num = 1;
        while (getline(file, linea)) {
            try {
                linea_num++;
                stringstream ss(linea); //permite leer la línea campo por campo separado por comas.
                string valor;

                // Leer id_terminal (int)
                getline(ss, valor, ',');
                //cout << "valor de id_terminal: " << valor << endl;
                int id = stoi(valor); //convierte una cadena de texto (string) a un número entero (int).

                // Leer nombre (string)
                getline(ss, valor, ',');
                string nombre = valor;

                // Leer num_buses (int)
                getline(ss, valor, ',');
                //cout << "valor de num_buses: " << valor << endl;
                int buses = stoi(valor);

                terminales.push_back(Terminal(id, nombre, buses));
            }
            catch (const exception& e) {
                cerr << "Error al procesar línea " << linea_num << ": " << linea << endl;
                cerr << "Motivo: " << e.what() << endl;
                throw;
            }
        }

        file.close();
        cout << "Se han cargado " << terminales.size() << " terminales." << endl;
        d = terminales.size(); 
    }

    void leerViajes(const string& archivo) {
        ifstream file(archivo);
        if (!file.is_open()) {
            cerr << "Error al abrir el archivo de viajes: " << archivo << endl;
            return;
        }

        string linea;
  
        getline(file, linea);

        int linea_num = 1;
      
        while (getline(file, linea)) {
            try {
                linea_num++;
                stringstream ss(linea);
                string valor;

                getline(ss, valor, ',');
                int id = stoi(valor);

                getline(ss, valor, ',');
                int salida = stoi(valor);
                //cout << "→ salida: " << salida << endl;

                getline(ss, valor, ',');
                int llegada = stoi(valor);
                //cout << "→ llegada: " << llegada << endl;

                getline(ss, valor, ',');
                //cout << "→ inicio: " << valor << endl;
                int t_inicio = horaTextoAMinutos(valor);

                getline(ss, valor, ',');
                //cout << "→ fin: " << valor << endl;
                int t_fin = horaTextoAMinutos(valor);

                getline(ss, valor, ',');
                string circuito = valor;
                //cout << "→ circuito: " << circuito << endl;

                viajes.push_back(Viaje(id, salida, llegada, t_inicio, t_fin, circuito));
            }
            catch (const exception& e) {
                cerr << "Error en línea " << linea_num << ": " << linea << endl;
                cerr << "Motivo: " << e.what() << endl;
                throw;
            }
        }

        file.close();
        cout << "Se han cargado " << viajes.size() << " viajes." << endl;

        // Ordenar los viajes por tiempo de inicio
        sort(viajes.begin(), viajes.end());

        n = static_cast<int>(viajes.size()); 
        inicializarEstructuras(); // Re-inicializar con el nuevo n
    }

    void imprimirMultigrafo() const {
        cout << "\n=== MULTIGRAFO VSP ===" << endl;
        cout << "Numero de viajes: " << n << endl;
        cout << "Numero de depósitos: " << d << endl;
        cout << "Numeracion de nodos:" << endl;
        cout << "- Depositos salida: {1, ..., " << d << "}" << endl;
        cout << "- Viajes: {" << (d + 1) << ", ..., " << (d + n) << "}" << endl;
        cout << "- Depositos llegada: {" << (n + d + 1) << ", ..., " << (n + 2 * d) << "}" << endl;

        cout << "\n--- ARCOS DE SALIDA ---" << endl;
        cout << arcos_sal << endl;

        cout << "\n--- ARCOS DE ENTRADA ---" << endl;
        cout << arcos_ent << endl;

        cout << "\n--- COSTOS PRIMALES ---" << endl;
        cout << c_primal << endl;
    }

    void construirArcos() {
        // Limpiar arcos existentes
        for (auto& lista : arcos_sal) lista.clear();
        for (auto& lista : arcos_ent) lista.clear();
        c_primal.clear();
        c_original.clear();

        // Verificar que tenemos datos
        if (terminales.empty() || viajes.empty()) {
            throw runtime_error("Debe cargar terminales y viajes antes de construir arcos");
        }

        // Definir tiempos de traslado entre depósitos y delta
        vector<double> tiempos_traslado = {11.40,11,5.71,7.84,13.06,9.74}; 
      
        // Procesar cada terminal por separado
        for (int terminal_idx = 0; terminal_idx < d; terminal_idx++) {
            int terminal_id = terminales[terminal_idx].getId();
            construirArcosPorTerminal(terminal_idx, terminal_id);
        }

        if (d > 1) {
            construirArcosEntreDepositos(tiempos_traslado);
        }

        cout << "Arcos construidos para el multigrafo VSP." << endl;
        cout << "Total de arcos creados: " << c_primal.size() << endl;
    }

    void construirArcosPorTerminal(int terminal_idx, int terminal_id) {
        // NUMERACIÓN CORRECTA:
        // - Depósitos salida: {1, ..., d}
        // - Viajes: {d+1, ..., d+n}
        // - Depósitos llegada: {n+d+1, ..., n+2d}

        int nodo_deposito_salida = terminal_idx + 1;           // Nodo en {1, ..., d}
        int nodo_deposito_llegada = n + d + terminal_idx + 1;  // Nodo en {n+d+1, ..., n+2d}

        vector<int> viajes_salen, viajes_llegan;

        for (int j = 0; j < n; j++) {
            int terminal_salida_viaje = viajes[j].getTerminalSalida();
            int terminal_llegada_viaje = viajes[j].getTerminalLlegada();

            if (terminal_salida_viaje == terminal_id) {
                viajes_salen.push_back(j);
            }
            if (terminal_llegada_viaje == terminal_id) {
                viajes_llegan.push_back(j);
            }
        }

        // 1. Crear arcos desde depósito de salida a viajes que salen de este
        for (int j : viajes_salen) {
            int nodo_viaje_j = d + j + 1;  
            Arco a(nodo_deposito_salida, nodo_viaje_j, nodo_deposito_salida);

            if (nodo_deposito_salida - 1 < arcos_sal.size() &&
                nodo_viaje_j - 1 < arcos_ent.size()) {
                arcos_sal[nodo_deposito_salida - 1].push_back(a);
                arcos_ent[nodo_viaje_j - 1].push_back(a);
                c_primal[a] = COSTO_FIJO_RUTA;
                c_original[a] = COSTO_FIJO_RUTA;
            }
        }

        // 2. Crear arcos desde viajes hacia depósito de llegada
        for (int j : viajes_llegan) {
            int nodo_viaje_j = d + j + 1;  
            Arco a(nodo_viaje_j, nodo_deposito_llegada, nodo_deposito_salida);

            if (nodo_viaje_j - 1 < arcos_sal.size() &&
                nodo_deposito_llegada - 1 < arcos_ent.size()) {
                arcos_sal[nodo_viaje_j - 1].push_back(a);
                arcos_ent[nodo_deposito_llegada - 1].push_back(a);
                c_primal[a] = 0;
                c_original[a] = 0;
            }
        }

        // Crear arcos entre viajes de esta terminal PARA TODOS LOS DEPÓSITOS
        for (int i : viajes_llegan) {
            int nodo_viaje_i = d + i + 1;

            for (int j : viajes_salen) {
                int nodo_viaje_j = d + j + 1;

                if (i == j) continue; // No crear arco de un viaje a sí mismo

                bool tiempoCompatible = viajes[j].getTiempoInicio() >=
                    viajes[i].getTiempoFin() + MIN_ESPERA;

                if (tiempoCompatible) {
                    double costo = static_cast<double>(
                        viajes[j].getTiempoInicio() - viajes[i].getTiempoFin()
                        );

                    // CREAR ARCO PARA CADA DEPÓSITO
                    for (int dep = 0; dep < d; dep++) {
                        int nodo_deposito_origen = dep + 1; // Depósitos van de 1 a d
                        Arco a(nodo_viaje_i, nodo_viaje_j, nodo_deposito_origen);

                        if (nodo_viaje_i - 1 < arcos_sal.size() &&
                            nodo_viaje_j - 1 < arcos_ent.size()) {
                            arcos_sal[nodo_viaje_i - 1].push_back(a);
                            arcos_ent[nodo_viaje_j - 1].push_back(a);
                            c_primal[a] = costo;
                            c_original[a] = costo;

                           // cout << "Arco mismo terminal: (" << nodo_viaje_i << ","
                            //    << nodo_viaje_j << "," << nodo_deposito_origen
                             //   << ") con costo " << costo << endl;
                        }
                    }
                }
            }
        }
    }

    void construirArcosEntreDepositos(const vector<double>& tiempos_traslado) {

        // Función auxiliar para obtener índice correcto del vector tiempos_traslado
        auto obtenerIndiceTiempoTraslado = [](int dep_i, int dep_j, int total_depositos) -> int {
            if (dep_i > dep_j) {
                int temp = dep_i;
                dep_i = dep_j;
                dep_j = temp;
            }

            int indice = 0;
            for (int i = 0; i < dep_i; i++) {
                indice += (total_depositos - 1 - i);
            }
            indice += (dep_j - dep_i - 1);
            return indice;
            };

        // Iterar sobre todos los pares de depósitos
        for (int dep_i = 0; dep_i < d; dep_i++) {
            for (int dep_j = 0; dep_j < d; dep_j++) {
                if (dep_i == dep_j) continue; // Saltar mismo depósito

                int terminal_id_i = terminales[dep_i].getId();
                int terminal_id_j = terminales[dep_j].getId();

                // Obtener tiempo de traslado usando el índice correcto
                int indice_tiempo = obtenerIndiceTiempoTraslado(dep_i, dep_j, d);
                double tiempo_traslado = tiempos_traslado[indice_tiempo];

                //cout << "Procesando arcos desde depósito " << dep_i << " hacia depósito "
                 //   << dep_j << " (tiempo traslado: " << tiempo_traslado
                 //   << ", índice: " << indice_tiempo << ")" << endl;

                // Obtener viajes que LLEGAN al depósito i
                vector<int> viajes_llegan_dep_i;
                for (int v = 0; v < n; v++) {
                    if (viajes[v].getTerminalLlegada() == terminal_id_i) {
                        viajes_llegan_dep_i.push_back(v);
                    }
                }

                // Obtener viajes que SALEN del depósito j
                vector<int> viajes_salen_dep_j;
                for (int v = 0; v < n; v++) {
                    if (viajes[v].getTerminalSalida() == terminal_id_j) {
                        viajes_salen_dep_j.push_back(v);
                    }
                }

                // Crear arcos inter-depósito PARA TODOS LOS DEPÓSITOS
                for (int viaje_i : viajes_llegan_dep_i) {
                    for (int viaje_j : viajes_salen_dep_j) {
                        if (viaje_i == viaje_j) continue;

                        // Verificar compatibilidad temporal
                        bool tiempoCompatible = viajes[viaje_j].getTiempoInicio() >=
                            viajes[viaje_i].getTiempoFin() + tiempo_traslado + MIN_ESPERA;

                        if (tiempoCompatible) {
                            int nodo_viaje_i = d + viaje_i + 1;
                            int nodo_viaje_j = d + viaje_j + 1;

                            // Calcular costo
                            double diferencia_tiempo = viajes[viaje_j].getTiempoInicio() - viajes[viaje_i].getTiempoFin();
                            double costo = diferencia_tiempo ;

                            // CREAR ARCO PARA CADA DEPÓSITO
                            for (int dep = 0; dep < d; dep++) {
                                int nodo_deposito_origen = dep + 1; // Depósitos van de 1 a d
                                Arco a(nodo_viaje_i, nodo_viaje_j, nodo_deposito_origen);

                                if (nodo_viaje_i - 1 < arcos_sal.size() &&
                                    nodo_viaje_j - 1 < arcos_ent.size()) {

                                    arcos_sal[nodo_viaje_i - 1].push_back(a);
                                    arcos_ent[nodo_viaje_j - 1].push_back(a);
                                    c_primal[a] = costo;
                                    c_original[a] = costo;

                                   // cout << "Arco inter-depósito: (" << nodo_viaje_i << ","
                                    //    << nodo_viaje_j << "," << nodo_deposito_origen
                                     //   << ") con costo " << costo << endl;
                                }
                            }

                          //  cout << "  Arcos creados: viaje " << viaje_i
                          //      << " (llega a dep " << dep_i << " en t=" << viajes[viaje_i].getTiempoFin()
                          //      << ") -> viaje " << viaje_j
                          //      << " (sale de dep " << dep_j << " en t=" << viajes[viaje_j].getTiempoInicio()
                          //      << ") para todos los depósitos" << endl;
            
                        }
                    }
                }
            }
        }
    }
    
    // Función auxiliar para obtener tiempo de traslado
    double obtenerTiempoTraslado(int dep_origen, int dep_destino, const vector<double>& tiempos_traslado) {
        
        // Mapear pares de depósitos a índice en el vector
        if (dep_origen == 0 && dep_destino == 1) return tiempos_traslado[0]; // 0->1
        if (dep_origen == 1 && dep_destino == 0) return tiempos_traslado[0]; // 1->0 (mismo tiempo)
        if (dep_origen == 0 && dep_destino == 2) return tiempos_traslado[1]; // 0->2
        if (dep_origen == 2 && dep_destino == 0) return tiempos_traslado[1]; // 2->0
        if (dep_origen == 1 && dep_destino == 2) return tiempos_traslado[2]; // 1->2
        if (dep_origen == 2 && dep_destino == 1) return tiempos_traslado[2]; // 2->1

        // Valor por defecto si no se encuentra el par
        return tiempos_traslado[0];
    }


    void actualizarCostosConDuales(const map<int, double>& alpha, const map<int, double>& beta) {
        //cout << "=== INICIO ACTUALIZACIÓN DE COSTOS ===" << endl;
        //cout << "Tamaño de alpha: " << alpha.size() << endl;
        //cout << "Tamaño de beta: " << beta.size() << endl;
        //cout << "Tamaño inicial de c_original: " << c_original.size() << endl;

        int costos_actualizados = 0;
        int arcos_procesados = 0;
        int arcos_viaje = 0;
        int arcos_deposito = 0;
        int arcos_otros = 0;
        int alpha_no_encontrados = 0;
        int beta_no_encontrados = 0;
        int betas_cero = 0;
        int alphas_cero = 0;

        //cout << "\n=== PROCESANDO ARCOS ===" << endl;

        // Recorrer todos los arcos
        for (int nodo = 0; nodo < arcos_sal.size(); nodo++) {
            for (auto& arco : arcos_sal[nodo]) {
                arcos_procesados++;
                int i = arco.getOrigen();
                int j = arco.getDestino();
                int k = arco.getDeposito();
                double costo_original = c_original[arco];
                double costo_modificado = costo_original;
                bool debe_actualizar = false;

                // Verificar si es un viaje
                if (i >= d + 1 && i <= d + n) {
                    arcos_viaje++;
                    auto it_alpha = alpha.find(i);
                    if (it_alpha != alpha.end()) {
                        double alpha_val = it_alpha->second;
                        costo_modificado = costo_original - alpha_val;
                        debe_actualizar = true;

                        if (alpha_val == 0.0) {
                            alphas_cero++;
                           // cout << "VIAJE - Arco (" << i << "," << j << "," << k << "): "
                           //     << costo_original << " - 0 = " << costo_modificado << endl;
                        }
                        else {
                            //cout << "VIAJE - Arco (" << i << "," << j << "," << k << "): "
                            //    << costo_original << " - " << alpha_val
                            //    << " = " << costo_modificado << endl;
                        }
                    }
                    else {
                        alpha_no_encontrados++;
                        cout << "WARNING: No se encontro alpha[" << i << "]" << endl;
                    }
                }
                // Verificar si es un depósito de salida
                else if (i >= 1 && i <= d) {
                    arcos_deposito++;
                    auto it_beta = beta.find(i);
                    if (it_beta != beta.end()) {
                        double beta_val = it_beta->second;
                        costo_modificado = costo_original + beta_val;
                        debe_actualizar = true;

                        if (beta_val == 0.0) {
                            betas_cero++;
                            //cout << "DEPÓSITO - Arco (" << i << "," << j << "," << k << "): "
                            //    << costo_original << " + 0 = " << costo_modificado << endl;
                        }
                        else {
                           // cout << "DEPÓSITO - Arco (" << i << "," << j << "," << k << "): "
                           //     << costo_original << " + " << beta_val
                           //     << " = " << costo_modificado << endl;
                        }
                    }
                    else {
                        beta_no_encontrados++;
                        cout << "WARNING: No se encontro beta[" << i << "]" << endl;
                    }
                }
                else {
                    arcos_otros++;
                    //cout << "OTRO - Arco (" << i << "," << j << "," << k << "): mantiene " << costo_original << endl;
                }

                // Actualizar SIEMPRE que debe_actualizar sea true
                if (debe_actualizar) {
                    c_primal[arco] = costo_modificado;
                    costos_actualizados++;
                }
            }
        }

        //cout << "\n=== RESUMEN DE ACTUALIZACIÓN ===" << endl;
        //cout << "Arcos procesados: " << arcos_procesados << endl;
        //cout << "Arcos de viaje: " << arcos_viaje << endl;
        //cout << "Arcos de depósito: " << arcos_deposito << endl;
        //cout << "Arcos de otros tipos: " << arcos_otros << endl;
        //cout << "Alpha no encontrados: " << alpha_no_encontrados << endl;
        //cout << "Beta no encontrados: " << beta_no_encontrados << endl;
        //cout << "Alphas con valor 0: " << alphas_cero << endl;
        //cout << "Betas con valor 0: " << betas_cero << endl;
        //cout << "Costos actualizados: " << costos_actualizados << endl;

        if (costos_actualizados == (arcos_viaje + arcos_deposito)) {
            cout << " Se actualizaron todos los arcos esperados" << endl;
        }
        else {
            cout << " ERROR: Falta actualizar algunos arcos" << endl;
        }
    }
};


// CLASE MULTIGRAFO EXTENDIDA CON DAG 
class Multigrafo_vsp_con_DAG : public Multigrafo_vsp {
private:
    
    // Función para recontruir un camino
    list<Arco> reconstruirCamino(int origen, int destino, const map<int, int>& pred) const {
        list<Arco> camino;

        if (pred.find(destino) == pred.end() || pred.at(destino) == -1 && origen != destino) {
            return camino;
        }

        int actual = destino;
        while (pred.find(actual) != pred.end() && pred.at(actual) != -1) {
            int predecesor = pred.at(actual);

            // Verificar que el predecesor está en rango
            if (predecesor < 1 || predecesor > getn() + 2 * getd()) {
                cout << "ERROR: Predecesor " << predecesor << " fuera de rango" << endl;
                break;
            }

            // Verificar que el índice del vector es válido
            int indice_vector = predecesor - 1;
            if (indice_vector >= 0 && indice_vector < getArcos_sal().size()) {
                bool arco_encontrado = false;
                for (const auto& arco : getArcos_sal()[indice_vector]) {
                    if (arco.getDestino() == actual) {
                        camino.push_front(arco);
                        arco_encontrado = true;
                        break;
                    }
                }

                if (!arco_encontrado) {
                    cout << "ERROR: No se encontró arco de " << predecesor << " a " << actual << endl;
                    break;
                }
            }

            actual = predecesor;
        }

        return camino;
    }

public:
    Multigrafo_vsp_con_DAG(int num_viajes, int num_depositos)
        : Multigrafo_vsp(num_viajes, num_depositos) {
    }

    // Estructura para almacenar información de columna generada
    struct ColumnaGenerada {
        int deposito_origen;
        int deposito_destino;
        double costo_reducido;
        list<Arco> camino;

        ColumnaGenerada(int origen, int destino, double costo, list<Arco> path)
            : deposito_origen(origen), deposito_destino(destino),
            costo_reducido(costo), camino(path) {
        }
    };

    ///////////////////////////////////////////////////////////////////// primeros k
    
    // Función que encuentra los primeros k caminos negativos
    vector<ResultadoCamino> encontrarPrimerosKCaminosNegativos(int deposito_k, int k_caminos) {
        // Verificar que el depósito existe
        if (deposito_k < 1 || deposito_k > getd()) {
            cerr << "ERROR: Depósito " << deposito_k << " fuera de rango [1, " << getd() << "]" << endl;
            return vector<ResultadoCamino>();
        }

        int nodo_salida = deposito_k;  // Nodos 1 a d
        int nodo_llegada = getn() + getd() + deposito_k;  // Nodos n+d+1 a n+2d

        // Vector para almacenar los caminos negativos encontrados
        vector<ResultadoCamino> caminos_negativos;

        //  Identificar todos los nodos alcanzables desde el depósito
        set<int> nodos_alcanzables;
        queue<int> cola;
        cola.push(nodo_salida);
        nodos_alcanzables.insert(nodo_salida);
        nodos_alcanzables.insert(nodo_llegada);

        while (!cola.empty()) {
            int actual = cola.front();
            cola.pop();

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    // Solo arcos de este depósito
                    if (arco.getDeposito() != deposito_k) continue;

                    // Solo arcos que existen en c_primal
                    if (getC_primal().count(arco)) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.insert(destino).second) {
                            // Si es nodo de viaje, seguir explorando
                            if (destino >= getd() + 1 && destino <= getd() + getn()) {
                                cola.push(destino);
                            }
                        }
                    }
                }
            }
        }

        // Ordenamiento trivial - orden natural (porque i < j siempre)
        vector<int> orden(nodos_alcanzables.begin(), nodos_alcanzables.end());
        sort(orden.begin(), orden.end());

        // Calcular distancias más cortas con verificación continua
        map<int, double> dist;
        map<int, int> pred;

        for (int nodo : orden) {
            dist[nodo] = INF;
            pred[nodo] = -1;
        }
        dist[nodo_salida] = 0.0;

        // Procesar nodos en orden topológico
        for (int actual : orden) {
            if (dist[actual] == INF) continue;

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    if (arco.getDeposito() != deposito_k) continue;

                    auto it = getC_primal().find(arco);
                    if (it != getC_primal().end()) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.count(destino)) {
                            double nueva_dist = dist[actual] + it->second;

                            // Verificar si mejoramos la distancia
                            if (nueva_dist < dist[destino]) {
                                dist[destino] = nueva_dist;
                                pred[destino] = actual;

                                // Si el destino es el depósito de llegada y la distancia es negativa
                                if (destino == nodo_llegada && nueva_dist < -1e-6) {
                                    // Reconstruir y guardar el camino
                                    list<Arco> camino = reconstruirCamino(nodo_salida, nodo_llegada, pred);
                                    caminos_negativos.push_back(ResultadoCamino(nueva_dist, camino));

                                    // Si ya encontramos k caminos, terminar
                                    if (caminos_negativos.size() >= k_caminos) {
                                        return caminos_negativos;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Retornar todos los caminos negativos encontrados (pueden ser menos de k)
        return caminos_negativos;
    }

    // Función para encontrar los primeros k caminos negativos de todos los depósitos
    vector<pair<int, vector<ResultadoCamino>>> encontrarPrimerosKCaminosNegativosTodos(int k_caminos) {
        vector<pair<int, vector<ResultadoCamino>>> resultado;

        for (int deposito = 1; deposito <= getd(); deposito++) {
            try {
                vector<ResultadoCamino> caminos = encontrarPrimerosKCaminosNegativos(deposito, k_caminos);

                if (!caminos.empty()) {
                    resultado.push_back(make_pair(deposito, caminos));
                }
            }
            catch (const exception& e) {
                cout << "ERROR en depósito " << deposito << ": " << e.what() << endl;
                continue;
            }
        }

        return resultado;
    }

    /////////////////////////////////////////////////////////////////////////////////////////// TODOS LOS CAMINOS

    // Función que encuentra TODOS los caminos negativos
    vector<ResultadoCamino> encontrarCaminosNegativos(int deposito_k) {
        // Verificar que el depósito existe
        if (deposito_k < 1 || deposito_k > getd()) {
            cerr << "ERROR: Depósito " << deposito_k << " fuera de rango [1, " << getd() << "]" << endl;
            return vector<ResultadoCamino>();
        }

        int nodo_salida = deposito_k;  // Nodos 1 a d
        int nodo_llegada = getn() + getd() + deposito_k;  // Nodos n+d+1 a n+2d

        vector<ResultadoCamino> caminos_negativos;

        // Identificar todos los nodos alcanzables desde el depósito
        set<int> nodos_alcanzables;
        queue<int> cola;
        cola.push(nodo_salida);
        nodos_alcanzables.insert(nodo_salida);
        nodos_alcanzables.insert(nodo_llegada);

        while (!cola.empty()) {
            int actual = cola.front();
            cola.pop();

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    // Solo arcos de este depósito
                    if (arco.getDeposito() != deposito_k) continue;

                    // Solo arcos que existen en c_primal
                    if (getC_primal().count(arco)) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.insert(destino).second) {
                            // Si es nodo de viaje, seguir explorando
                            if (destino >= getd() + 1 && destino <= getd() + getn()) {
                                cola.push(destino);
                            }
                        }
                    }
                }
            }
        }

        // Ordenamiento trivial - orden natural (porque i < j siempre)
        vector<int> orden(nodos_alcanzables.begin(), nodos_alcanzables.end());
        sort(orden.begin(), orden.end());

        // Calcular distancias más cortas con verificación continua
        map<int, double> dist;
        map<int, int> pred;

        for (int nodo : orden) {
            dist[nodo] = INF;
            pred[nodo] = -1;
        }
        dist[nodo_salida] = 0.0;

        // Procesar nodos en orden topológico
        for (int actual : orden) {
            if (dist[actual] == INF) continue;

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    if (arco.getDeposito() != deposito_k) continue;

                    auto it = getC_primal().find(arco);
                    if (it != getC_primal().end()) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.count(destino)) {
                            double nueva_dist = dist[actual] + it->second;

                            // Verificar si mejoramos la distancia
                            if (nueva_dist < dist[destino]) {
                                dist[destino] = nueva_dist;
                                pred[destino] = actual;

                                // Si el destino es el depósito de llegada y la distancia es negativa
                                if (destino == nodo_llegada && nueva_dist < -1e-6) {
                                    // Reconstruir y guardar el camino
                                    list<Arco> camino = reconstruirCamino(nodo_salida, nodo_llegada, pred);
                                    caminos_negativos.push_back(ResultadoCamino(nueva_dist, camino));

                                    // NO RETORNAMOS AQUÍ - continuamos buscando más caminos
                                }
                            }
                        }
                    }
                }
            }
        }

        // Retornar TODOS los caminos negativos encontrados
        return caminos_negativos;
    }

    // Función para encontrar TODOS los caminos negativos de todos los depósitos
    vector<pair<int, vector<ResultadoCamino>>> encontrarCaminosNegativosTodos() {
        vector<pair<int, vector<ResultadoCamino>>> resultado;

        for (int deposito = 1; deposito <= getd(); deposito++) {
            try {
                vector<ResultadoCamino> caminos = encontrarCaminosNegativos(deposito);

                if (!caminos.empty()) {
                    resultado.push_back(make_pair(deposito, caminos));
                }
            }
            catch (const exception& e) {
                cout << "ERROR en depósito " << deposito << ": " << e.what() << endl;
                continue;
            }
        }

        return resultado;
    }

    //////////////////////////////////////////////////////////////////////////////////// EL MÁS NEGATIVO

    
    ResultadoCamino encontrarPrimerCaminoNegativoUltraRapido(int deposito_k) {
        // Verificar que el depósito existe
        if (deposito_k < 1 || deposito_k > getd()) {
            cerr << "ERROR: Depósito " << deposito_k << " fuera de rango [1, " << getd() << "]" << endl;
            return ResultadoCamino();
        }

        int nodo_salida = deposito_k;  // Nodos 1 a d
        int nodo_llegada = getn() + getd() + deposito_k;  // Nodos n+d+1 a n+2d

        // Identificar todos los nodos alcanzables desde el depósito
        set<int> nodos_alcanzables;
        queue<int> cola;
        cola.push(nodo_salida);
        nodos_alcanzables.insert(nodo_salida);
        nodos_alcanzables.insert(nodo_llegada);

        while (!cola.empty()) {
            int actual = cola.front();
            cola.pop();

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    // Solo arcos de este depósito
                    if (arco.getDeposito() != deposito_k) continue;

                    // Solo arcos que existen en c_primal
                    if (getC_primal().count(arco)) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.insert(destino).second) {
                            // Si es nodo de viaje, seguir explorando
                            if (destino >= getd() + 1 && destino <= getd() + getn()) {
                                cola.push(destino);
                            }
                        }
                    }
                }
            }
        }

        // Ordenamiento trivial - orden natural (porque i < j siempre)
        vector<int> orden(nodos_alcanzables.begin(), nodos_alcanzables.end());
        sort(orden.begin(), orden.end());

        // Calcular distancias más cortas con una sola pasada
        map<int, double> dist;
        map<int, int> pred;

        for (int nodo : orden) {
            dist[nodo] = INF;
            pred[nodo] = -1;
        }
        dist[nodo_salida] = 0.0;

        // Una sola pasada es suficiente gracias al orden topológico garantizado
        for (int actual : orden) {
            if (dist[actual] == INF) continue;

            int indice = actual - 1;
            if (indice >= 0 && indice < getArcos_sal().size()) {
                for (const auto& arco : getArcos_sal()[indice]) {
                    if (arco.getDeposito() != deposito_k) continue;

                    auto it = getC_primal().find(arco);
                    if (it != getC_primal().end()) {
                        int destino = arco.getDestino();
                        if (nodos_alcanzables.count(destino)) {
                            double nueva_dist = dist[actual] + it->second;
                            if (nueva_dist < dist[destino]) {
                                dist[destino] = nueva_dist;
                                pred[destino] = actual;
                            }
                        }
                    }
                }
            }
        }

        // Verificar si hay camino negativo
        if (dist[nodo_llegada] < -1e-6) {
            list<Arco> camino = reconstruirCamino(nodo_salida, nodo_llegada, pred);
            return ResultadoCamino(dist[nodo_llegada], camino);
        }

        return ResultadoCamino();
    }

    // Función optimizada para encontrar todos los caminos negativos de todos los depósitos
    vector<pair<int, ResultadoCamino>> encontrarTodosCaminosNegativosUltraRapido() {
        vector<pair<int, ResultadoCamino>> caminos_encontrados;

        for (int k = 1; k <= getd(); k++) {
            try {
                ResultadoCamino resultado = encontrarPrimerCaminoNegativoUltraRapido(k);

                if (resultado.existeCamino() && resultado.getDistancia() < -1e-6) {
                    caminos_encontrados.push_back(make_pair(k, resultado));
                }
            }
            catch (const exception& e) {
                cout << "ERROR en depósito " << k << ": " << e.what() << endl;
                continue;
            }
        }

        return caminos_encontrados;
    }
};

//Estructura para guardar iteraciones generación
struct IteracionInfo {
    int num_iteracion;
    double tiempo_acumulado_segundos;
    double valor_objetivo;
    int columnas_anadidas_iteracion;
};

// Función para guardar el progreso en CSV
void guardarProgresoIteraciones(const std::vector<IteracionInfo>& historial, const std::string& nombre_archivo) {
    std::ofstream archivo(nombre_archivo);

    if (!archivo.is_open()) {
        std::cout << "ERROR: No se pudo crear el archivo " << nombre_archivo << std::endl;
        return;
    }
    // Escribir headers - MODIFICAR ESTA LÍNEA
    archivo << "Iteracion,Tiempo_Acumulado_Segundos,Valor_Objetivo,Columnas_Anadidas\n";


    // Escribir datos - MODIFICAR ESTA LÍNEA
    for (const auto& info : historial) {
        archivo << info.num_iteracion << ","
            << std::fixed << std::setprecision(2) << info.tiempo_acumulado_segundos << ","
            << std::fixed << std::setprecision(6) << info.valor_objetivo << ","
            << info.columnas_anadidas_iteracion << "\n";  
    }

    archivo.close();
    std::cout << "Historial de iteraciones guardado en: " << nombre_archivo << std::endl;
}

// Función para verificar si la solución es factible para el modelo
bool verificarSolucionFactible(const vector<vector<int>>& rutas_solucion,
    const Multigrafo_vsp& multigrafo,
    bool mostrar_detalles = true) {
    try {
        if (mostrar_detalles) {
            cout << "\n=== VERIFICANDO FACTIBILIDAD DE LA SOLUCIÓN ===" << endl;
        }

        int n = multigrafo.getn();  // número de viajes
        int d = multigrafo.getd();  // número de depósitos
        const unordered_map<Arco, double>& arcos_costos = multigrafo.getC_original();

        if (mostrar_detalles) {
            cout << "Configuración del problema:" << endl;
            cout << "- Número de viajes: " << n << endl;
            cout << "- Número de depósitos: " << d << endl;
            cout << "- Nodos de depósitos salida: 1 a " << d << endl;
            cout << "- Nodos de viajes: " << (d + 1) << " a " << (d + n) << endl;
            cout << "- Nodos de depósitos llegada: " << (n + d + 1) << " a " << (n + 2 * d) << endl;
            cout << "- Total de rutas a verificar: " << rutas_solucion.size() << endl;
        }

        bool solucion_factible = true;
        vector<string> errores;

        
        set<tuple<int, int, int>> arcos_validos; // (origen, destino, deposito)
        for (const auto& par : arcos_costos) {
            const Arco& arco = par.first;
            arcos_validos.insert({ arco.getOrigen(), arco.getDestino(), arco.getDeposito() });
        }

        // Conjunto para verificar que cada viaje aparece exactamente una vez
        set<int> viajes_cubiertos;
        map<int, int> viajes_contador; // viaje -> cuántas veces aparece

        // VERIFICACIÓN RUTA POR RUTA
        for (size_t r = 0; r < rutas_solucion.size(); r++) {
            const vector<int>& ruta = rutas_solucion[r];

            if (mostrar_detalles) {
                cout << "\n--- Verificando Ruta " << (r + 1) << ": ";
                for (size_t i = 0; i < ruta.size(); i++) {
                    cout << ruta[i];
                    if (i < ruta.size() - 1) cout << " -> ";
                }
                cout << " ---" << endl;
            }

            // Verificar que la ruta tenga al menos 3 nodos (depósito_salida -> viaje -> depósito_llegada)
            if (ruta.size() < 3) {
                string error = "Ruta " + to_string(r + 1) + ": Debe tener al menos 3 nodos";
                errores.push_back(error);
                if (mostrar_detalles) cout << "e" << error << endl;
                solucion_factible = false;
                continue;
            }

            int nodo_inicio = ruta[0];
            int nodo_fin = ruta[ruta.size() - 1];

            // ============ VERIFICACIÓN (i): MISMO DEPÓSITO ============
            // Verificar que empiece en depósito de salida (1 a d)
            if (nodo_inicio < 1 || nodo_inicio > d) {
                string error = "Ruta " + to_string(r + 1) + ": No empieza en depósito de salida válido (" + to_string(nodo_inicio) + ")";
                errores.push_back(error);
                if (mostrar_detalles) cout << "e " << error << endl;
                solucion_factible = false;
            }

            // Verificar que termine en depósito de llegada correspondiente
            int deposito_llegada_esperado = n + d + nodo_inicio; // Si sale del depósito k, debe llegar al n+d+k
            if (nodo_fin != deposito_llegada_esperado) {
                string error = "Ruta " + to_string(r + 1) + ": No termina en depósito de llegada correcto";
                error += " (esperado: " + to_string(deposito_llegada_esperado) + ", obtenido: " + to_string(nodo_fin) + ")";
                errores.push_back(error);
                if (mostrar_detalles) cout << "e " << error << endl;
                solucion_factible = false;
            }
            else {
                if (mostrar_detalles) cout << "Empieza y termina en el mismo depósito (" << nodo_inicio << " -> " << nodo_fin << ")" << endl;
            }

            //  VERIFICACIÓN  ARCOS VÁLIDOS
            bool arcos_validos_ruta = true;
            for (size_t i = 0; i < ruta.size() - 1; i++) {
                int origen = ruta[i];
                int destino = ruta[i + 1];

                // El depósito al que pertenece esta ruta
                int deposito_ruta = nodo_inicio;

                // Verificar si existe el arco (origen, destino, deposito_ruta)
                bool arco_existe = arcos_validos.count({ origen, destino, deposito_ruta }) > 0;

                if (!arco_existe) {
                    string error = "Ruta " + to_string(r + 1) + ": Arco (" + to_string(origen) + "," +
                        to_string(destino) + "," + to_string(deposito_ruta) + ") no existe en el grafo";
                    errores.push_back(error);
                    if (mostrar_detalles) cout << "e" << error << endl;
                    solucion_factible = false;
                    arcos_validos_ruta = false;
                }
            }

            if (arcos_validos_ruta && mostrar_detalles) {
                cout << "Todos los arcos existen en el grafo" << endl;
            }

            // VERIFICACIÓN VIAJES ÚNICOS
            // Contar viajes en esta ruta (nodos entre d+1 y d+n)
            vector<int> viajes_en_ruta;
            for (int nodo : ruta) {
                if (nodo >= d + 1 && nodo <= d + n) {
                    viajes_en_ruta.push_back(nodo);
                    viajes_cubiertos.insert(nodo);
                    viajes_contador[nodo]++;
                }
            }

            if (mostrar_detalles) {
                cout << " Viajes en esta ruta: ";
                for (size_t i = 0; i < viajes_en_ruta.size(); i++) {
                    cout << viajes_en_ruta[i];
                    if (i < viajes_en_ruta.size() - 1) cout << ", ";
                }
                cout << " (total: " << viajes_en_ruta.size() << ")" << endl;
            }
        }

        //VERIFICACIONES GLOBALES
        if (mostrar_detalles) {
            cout << "\n=== VERIFICACIONES GLOBALES ===" << endl;
        }

        // Verificar que todos los viajes estén cubiertos exactamente una vez
        bool todos_viajes_cubiertos = true;

        // Verificar viajes faltantes
        vector<int> viajes_faltantes;
        for (int v = d + 1; v <= d + n; v++) {
            if (viajes_cubiertos.find(v) == viajes_cubiertos.end()) {
                viajes_faltantes.push_back(v);
                todos_viajes_cubiertos = false;
                solucion_factible = false;
            }
        }

        // Verificar viajes duplicados
        vector<int> viajes_duplicados;
        for (const auto& par : viajes_contador) {
            if (par.second > 1) {
                viajes_duplicados.push_back(par.first);
                todos_viajes_cubiertos = false;
                solucion_factible = false;
            }
        }

        if (mostrar_detalles) {
            cout << "Total de viajes que deben cubrirse: " << n << endl;
            cout << "Total de viajes cubiertos: " << viajes_cubiertos.size() << endl;

            if (!viajes_faltantes.empty()) {
                cout << " Viajes faltantes: ";
                for (size_t i = 0; i < viajes_faltantes.size(); i++) {
                    cout << viajes_faltantes[i];
                    if (i < viajes_faltantes.size() - 1) cout << ", ";
                }
                cout << endl;
                errores.push_back("Hay " + to_string(viajes_faltantes.size()) + " viajes sin cubrir");
            }

            if (!viajes_duplicados.empty()) {
                cout << " Viajes duplicados: ";
                for (size_t i = 0; i < viajes_duplicados.size(); i++) {
                    cout << viajes_duplicados[i] << " (aparece " << viajes_contador[viajes_duplicados[i]] << " veces)";
                    if (i < viajes_duplicados.size() - 1) cout << ", ";
                }
                cout << endl;
                errores.push_back("Hay " + to_string(viajes_duplicados.size()) + " viajes duplicados");
            }

            if (todos_viajes_cubiertos) {
                cout << " Todos los viajes están cubiertos exactamente una vez" << endl;
            }
        }

        // RESUMEN FINAL
        if (mostrar_detalles) {
            cout << "\n=== RESUMEN DE VERIFICACIÓN ===" << endl;
            if (solucion_factible) {
                cout << " ¡SOLUCIÓN FACTIBLE! Todas las verificaciones pasaron." << endl;
            }
            else {
                cout << " SOLUCIÓN NO FACTIBLE. Se encontraron " << errores.size() << " errores:" << endl;
                for (size_t i = 0; i < errores.size(); i++) {
                    cout << "   " << (i + 1) << ". " << errores[i] << endl;
                }
            }
        }

        return solucion_factible;

    }
    catch (const exception& e) {
        if (mostrar_detalles) {
            cout << " Error durante la verificación: " << e.what() << endl;
        }
        return false;
    }
}

// Función auxiliar para verificar sin mostrar detalles
bool esSolucionFactible(const vector<vector<int>>& rutas_solucion,
    const Multigrafo_vsp& multigrafo) {
    return verificarSolucionFactible(rutas_solucion, multigrafo, false);
}


void resolverModeloArcos(Multigrafo_vsp& multigrafo, const string& nombre_instancia) {
    try {
        cout << "\n=== RESOLVIENDO MODELO DE ARCOS===\n";
        int t = 3600 * 3;
        //string nombre_archivo_csv = "Norte_2_depositos_mod_Arcos.csv";

        int n = multigrafo.getn(); 
        int d = multigrafo.getd();  

        // Crear entorno y modelo de Gurobi
        GRBEnv env_arcos = GRBEnv(true);
        env_arcos.set("LogFile", "modelo_arcos.log");
        env_arcos.start();
        GRBModel modelo_arcos = GRBModel(env_arcos);

        // =============================== VARIABLES DEL MODELO ===================================================
        map<Arco, GRBVar> X;

        // Obtener todos los arcos del multigrafo
        const unordered_map<Arco, double>& arcos_costos = multigrafo.getC_original();

        cout << "Creando variables para " << arcos_costos.size() << " arcos..." << endl;
        auto inicio = std::chrono::high_resolution_clock::now();
        for (const auto& par : arcos_costos) {
            const Arco& arco = par.first;
            int i = arco.getOrigen();
            int j = arco.getDestino();
            int k = arco.getDeposito();

            string nombre_var = "X_" + to_string(i) + "_" + to_string(j) + "_" + to_string(k);
            X[arco] = modelo_arcos.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, nombre_var);
        }

        // ==================================== FUNCIÓN OBJETIVO ================================================
        // Minimizar: Σ C_ijk * X_ijk
        GRBLinExpr objetivo = 0;
        for (const auto& par : arcos_costos) {
            const Arco& arco = par.first;
            double costo = par.second;
            objetivo += costo * X[arco];
        }
        modelo_arcos.setObjective(objetivo, GRB_MINIMIZE);

        // ==================================== RESTRICCIONES ========================================================

        //  Cada viaje debe ser cubierto exactamente una vez
        for (int j = d + 1; j <= d + n; j++) {
            GRBLinExpr suma_cobertura = 0;

            for (const auto& par : arcos_costos) {
                const Arco& arco = par.first;
                if (arco.getDestino() == j) {
                    suma_cobertura += X[arco];
                }
            }

            string nombre_constr = "cobertura_viaje_" + to_string(j);
            modelo_arcos.addConstr(suma_cobertura == 1.0, nombre_constr);
        }

        //Conservación de flujo en cada viaje para cada depósito
        for (int j = d + 1; j <= d + n; j++) {  // Para cada viaje
            for (int k = 1; k <= d; k++) {  // Para cada depósito
                GRBLinExpr flujo_entrante = 0;
                GRBLinExpr flujo_saliente = 0;

                // Buscar arcos entrantes y salientes para el viaje j del depósito k
                for (const auto& par : arcos_costos) {
                    const Arco& arco = par.first;

                    // Solo considerar arcos que pertenecen al depósito k
                    if (arco.getDeposito() == k) {
                        if (arco.getDestino() == j) {  // Arco entrante al viaje j del depósito k
                            flujo_entrante += X[arco];
                        }

                        if (arco.getOrigen() == j) {  // Arco saliente del viaje j del depósito k
                            flujo_saliente += X[arco];
                        }
                    }
                }

                string nombre_constr = "flujo_viaje_" + to_string(j) + "_deposito_" + to_string(k);
                modelo_arcos.addConstr(flujo_entrante == flujo_saliente, nombre_constr);
            }
        }

        // Capacidad de depósitos de salida
        for (int i = 1; i <= d; i++) {
            GRBLinExpr vehiculos_usados = 0;

            // Buscar TODOS los arcos que salen del depósito i
            for (const auto& par : arcos_costos) {
                const Arco& arco = par.first;
                if (arco.getOrigen() == i) {
                    vehiculos_usados += X[arco];
                }
            }

            int capacidad_i = multigrafo.getTerminales()[i - 1].getNumBuses();
            string nombre_constr = "capacidad_deposito_" + to_string(i);
            modelo_arcos.addConstr(vehiculos_usados <= capacidad_i, nombre_constr);
        }

        // ============================================ RESOLVER MODELO ======================================
        //cout << "\nResolviendo modelo de arcos" << endl;
        

        // Configurar parámetros del solver
        modelo_arcos.set(GRB_DoubleParam_TimeLimit, t);
        modelo_arcos.set(GRB_DoubleParam_MIPGap, 0.01);

        modelo_arcos.write("modelo_arcos.lp");
        modelo_arcos.optimize();

        auto fin = std::chrono::high_resolution_clock::now();
        double tiempo = std::chrono::duration<double>(fin - inicio).count();

        // ======================================== ANALIZAR RESULTADOS ========================================
        int num_vars = modelo_arcos.get(GRB_IntAttr_NumVars);
        int num_constrs = modelo_arcos.get(GRB_IntAttr_NumConstrs);
        int status = modelo_arcos.get(GRB_IntAttr_Status);

        double valor_objetivo = 0.0;
        double mejor_cota_inferior = 0.0;
        double mejor_solucion = 0.0;
        double gap_final = 0.0;
        double cota_lp = 0.0;  
        int nodos_branch_cut = 0;
        int vehiculos_totales = 0;
        string estado_str = "DESCONOCIDO";

        // ======================================== CONSTRUCCIÓN DE RUTAS ==========================================
        vector<vector<int>> rutas_solucion;

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            estado_str = (status == GRB_OPTIMAL) ? "OPTIMO" : "LIMITE_TIEMPO";

            try {
                valor_objetivo = modelo_arcos.get(GRB_DoubleAttr_ObjVal);
                mejor_cota_inferior = (status == GRB_OPTIMAL) ? valor_objetivo : modelo_arcos.get(GRB_DoubleAttr_ObjBound);
                mejor_solucion = valor_objetivo;

                if (status == GRB_TIME_LIMIT) {
                    gap_final = modelo_arcos.get(GRB_DoubleAttr_MIPGap);
                }

                try {
                    cota_lp = modelo_arcos.get(GRB_DoubleAttr_ObjBoundC);
                }
                catch (...) {
                    cota_lp = mejor_cota_inferior;
                }

                cout << "Estado: " << ((status == GRB_OPTIMAL) ? "ÓPTIMO" : "LÍMITE DE TIEMPO") << endl;
                cout << "Valor objetivo: " << fixed << setprecision(6) << valor_objetivo << endl;

                // ============ MOSTRAR ARCOS ACTIVOS ============
                //cout << "\n=== ARCOS ACTIVOS EN LA SOLUCIÓN ===" << endl;
                //vector<pair<Arco, double>> arcos_activos;

                //for (auto& par : X) {
                //    double valor = par.second.get(GRB_DoubleAttr_X);
                //    if (valor > 0.001) {  // Mostrar todos los arcos con valor > 0
                //        const Arco& arco = par.first;
                //        arcos_activos.push_back({ arco, valor });
                //        cout << "Arco (" << arco.getOrigen() << "," << arco.getDestino()
                //            << "," << arco.getDeposito() << ") = " << fixed << setprecision(6) << valor << endl;
                //    }
                //}
                //cout << "Total de arcos activos: " << arcos_activos.size() << endl;

                // ============ CONSTRUIR RUTAS A PARTIR DE LA SOLUCIÓN ============
                //cout << "\n=== CONSTRUYENDO RUTAS ===" << endl;

                // Crear estructura para almacenar arcos activos por depósito
                map<int, vector<pair<int, int>>> arcos_por_deposito; 

                for (auto& par : X) {
                    double valor = par.second.get(GRB_DoubleAttr_X);
                    if (valor > 0.99) {  // Arco activo
                        const Arco& arco = par.first;
                        arcos_por_deposito[arco.getDeposito()].push_back({ arco.getOrigen(), arco.getDestino() });
                    }
                }

                // Construir rutas para cada depósito
                for (int deposito = 1; deposito <= d; deposito++) {
                    if (arcos_por_deposito.find(deposito) == arcos_por_deposito.end()) continue;

                    //cout << "\n--- Procesando depósito " << deposito << " ---" << endl;
                    vector<pair<int, int>> arcos_disponibles = arcos_por_deposito[deposito];

                    // Mostrar todos los arcos del depósito
                    //cout << "Arcos disponibles para depósito " << deposito << ":" << endl;
                    for (const auto& arco : arcos_disponibles) {
                        //cout << "  " << arco.first << " -> " << arco.second << endl;
                    }

                    // Encontrar todas las rutas que empiecen desde el depósito
                    while (!arcos_disponibles.empty()) {
                        // Buscar un arco que salga del depósito de origen
                        bool ruta_encontrada = false;

                        for (auto it = arcos_disponibles.begin(); it != arcos_disponibles.end(); ++it) {
                            if (it->first == deposito) {  // Arco que sale del depósito
                                vector<int> ruta;
                                vector<pair<int, int>> arcos_temp = arcos_disponibles;
                                int nodo_actual = it->first;
                                int siguiente = it->second;

                                // Remover el arco inicial
                                arcos_temp.erase(find(arcos_temp.begin(), arcos_temp.end(), *it));

                                ruta.push_back(nodo_actual);
                                nodo_actual = siguiente;

                                // Seguir construyendo la ruta
                                while (true) {
                                    ruta.push_back(nodo_actual);

                                    // Si llegamos a un depósito de destino, terminar
                                    if (nodo_actual >= n + d + 1 && nodo_actual <= n + 2 * d) {
                                        break;
                                    }

                                    // Buscar el siguiente arco
                                    bool siguiente_encontrado = false;
                                    for (auto it2 = arcos_temp.begin(); it2 != arcos_temp.end(); ++it2) {
                                        if (it2->first == nodo_actual) {
                                            siguiente = it2->second;
                                            arcos_temp.erase(it2);
                                            nodo_actual = siguiente;
                                            siguiente_encontrado = true;
                                            break;
                                        }
                                    }

                                    if (!siguiente_encontrado) {
                                        cout << "Advertencia: No se encontró continuación para nodo " << nodo_actual << endl;
                                        break;
                                    }
                                }

                                // Remover los arcos usados de la lista principal
                                for (size_t i = 0; i < ruta.size() - 1; i++) {
                                    pair<int, int> arco_usado = { ruta[i], ruta[i + 1] };
                                    auto pos = find(arcos_disponibles.begin(), arcos_disponibles.end(), arco_usado);
                                    if (pos != arcos_disponibles.end()) {
                                        arcos_disponibles.erase(pos);
                                    }
                                }

                                // Agregar ruta válida
                                if (ruta.size() > 2) {
                                    rutas_solucion.push_back(ruta);
                                    //cout << "Ruta " << rutas_solucion.size() << ": ";
                                    //for (size_t i = 0; i < ruta.size(); i++) {
                                    //    cout << ruta[i];
                                    //    if (i < ruta.size() - 1) cout << " ; ";
                                    //}
                                    cout << endl;
                                }

                                ruta_encontrada = true;
                                break;
                            }
                        }

                        // Si no encontramos más rutas que empiecen desde el depósito, salir
                        if (!ruta_encontrada) {
                            // Pueden quedar arcos internos, intentar procesarlos
                            if (!arcos_disponibles.empty()) {
                                //cout << "Arcos restantes (posiblemente internos):" << endl;
                                for (const auto& arco : arcos_disponibles) {
                                    //cout << "  " << arco.first << " -> " << arco.second << endl;
                                }
                            }
                            break;
                        }
                    }
                }

                // Contar vehículos totales
                for (auto& par : X) {
                    double valor = par.second.get(GRB_DoubleAttr_X);
                    if (valor > 0.99) {  // Arco activo
                        const Arco& arco = par.first;
                        int origen = arco.getOrigen();
                        // Solo contar arcos que salen de depósitos de origen (nodos 1 a d)
                        if (origen >= 1 && origen <= d) {
                            vehiculos_totales++;
                        }
                    }
                }

                cout << "\nTotal de rutas encontradas: " << rutas_solucion.size() << endl;
                cout << "Total de vehiculos utilizados: " << vehiculos_totales << endl;

            }
            catch (const exception& e) {
                cout << "Error al procesar solución: " << e.what() << endl;
            }
        }

        else if (status == GRB_INFEASIBLE) {
            estado_str = "INFACTIBLE";
            cout << "Estado: INFACTIBLE" << endl;

            cout << "Computando IIS..." << endl;
            modelo_arcos.computeIIS();
            modelo_arcos.write("modelo_infactible.ilp");
            cout << "IIS guardado en modelo_infactible.ilp" << endl;
        }
        else {
            estado_str = "OTRO_" + to_string(status);
            cout << "Estado: " << status << endl;
        }

        try {
            nodos_branch_cut = (int)modelo_arcos.get(GRB_DoubleAttr_NodeCount);
        }
        catch (...) {
            nodos_branch_cut = 0;
        }

        bool es_factible = esSolucionFactible(rutas_solucion, multigrafo);
        cout << "1 si es factible, 0 sino : " << es_factible << endl;

        // ============ GUARDAR RESULTADOS ============
        bool archivo_existe = false;
        ifstream test_file(nombre_instancia);
        if (test_file.good()) {
            archivo_existe = true;
        }
        test_file.close();

        ofstream csv_file(nombre_instancia, ios::app);
        if (csv_file.is_open()) {
            if (!archivo_existe) {
                csv_file << "Instancia,NumVars,NumRestr,TiempoResolucion,MejorCota,MejorSolucion,GapFinal,CotaLP,NodosBC,Estado,VehiculosUsados\n";  // MODIFICADO
            }

            csv_file << nombre_instancia << ","
                << num_vars << ","
                << num_constrs << ","
                << fixed << setprecision(2) << tiempo << ","
                << fixed << setprecision(6) << mejor_cota_inferior << ","
                << fixed << setprecision(6) << mejor_solucion << ","
                << fixed << setprecision(6) << gap_final << ","
                << fixed << setprecision(6) << cota_lp << ","  // AÑADIDO
                << nodos_branch_cut << ","
                << estado_str << ","
                << vehiculos_totales << "\n";

            csv_file.close();
            cout << "Resultados guardados" << endl;
        }

        // ============ GUARDAR RUTAS EN CSV SEPARADO ============
        if (!rutas_solucion.empty()) {
            // Crear nombre del archivo de rutas
            string nombre_rutas = nombre_instancia;
            size_t pos = nombre_rutas.find_last_of('.');
            if (pos != string::npos) {
                nombre_rutas = nombre_rutas.substr(0, pos) + "_rutas.csv";
            }
            else {
                nombre_rutas += "_rutas.csv";
            }

            ofstream csv_rutas(nombre_rutas);
            if (csv_rutas.is_open()) {
                // Escribir encabezado
                csv_rutas << "NumRuta,Secuencia,NumNodos,CostoRuta\n";

                // Escribir cada ruta
                for (size_t i = 0; i < rutas_solucion.size(); i++) {
                    const vector<int>& ruta = rutas_solucion[i];

                    // Calcular costo de la ruta
                    double costo_ruta = 0.0;
                    for (size_t j = 0; j < ruta.size() - 1; j++) {
                        int origen = ruta[j];
                        int destino = ruta[j + 1];

                        // Buscar el arco correspondiente y su costo
                        for (const auto& par : arcos_costos) {
                            const Arco& arco = par.first;
                            if (arco.getOrigen() == origen && arco.getDestino() == destino) {
                                costo_ruta += par.second;
                                break;
                            }
                        }
                    }

                    csv_rutas << (i + 1) << ",\"";

                    // Escribir secuencia de nodos
                    for (size_t j = 0; j < ruta.size(); j++) {
                        csv_rutas << ruta[j];
                        if (j < ruta.size() - 1) csv_rutas << ",";
                    }

                    csv_rutas << "\"," << ruta.size() << ","
                        << fixed << setprecision(6) << costo_ruta << "\n";
                }

                csv_rutas.close();
                cout << "Rutas guardadas en: " << nombre_rutas << endl;
            }
            else {
                cout << "Error: No se pudo crear el archivo de rutas" << endl;
            }
        }

    }
    catch (GRBException& e) {
        cerr << "Error de Gurobi: " << e.getMessage() << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}


void resolverModeloAsignacion(Multigrafo_vsp& multigrafo, const string& nombre_instancia) {
    try {
        cout << "\n=== RESOLVIENDO MODELO DE ASIGNACIÓN (COTA INFERIOR MDVSP) ===\n";
        int t = 3600 * 3;

        int n = multigrafo.getn();  
        int d = multigrafo.getd();  

        //cout << "Viajes: " << n << ", Depósitos: " << d << endl;

        // Crear entorno y modelo de Gurobi
        GRBEnv env_asignacion = GRBEnv(true);
        env_asignacion.set("LogFile", "modelo_asignacion.log");
        env_asignacion.start();
        GRBModel modelo_asignacion = GRBModel(env_asignacion);

        // ================================== VARIABLES DEL MODELO ===============================================
        auto inicio = std::chrono::high_resolution_clock::now();
        map<pair<int, int>, GRBVar> X;

        // Crear conjunto de arcos simplificados
        set<pair<int, int>> arcos_simplificados;
        map<pair<int, int>, double> costos_simplificados;

        // Obtener todos los arcos del multigrafo original
        const unordered_map<Arco, double>& arcos_costos = multigrafo.getC_original();

        //cout << "Procesando " << arcos_costos.size() << " arcos originales..." << endl;

        // Simplificar arcos: eliminar la dimensión del depósito
        for (const auto& par : arcos_costos) {
            const Arco& arco = par.first;
            double costo = par.second;

            int origen = arco.getOrigen();
            int destino = arco.getDestino();

            // Crear arco simplificado (i,j)
            pair<int, int> arco_simple = { origen, destino };

            // Agregar arco (no hay duplicados)
            costos_simplificados[arco_simple] = costo;
            arcos_simplificados.insert(arco_simple);
        }

        //cout << "Arcos simplificados: " << arcos_simplificados.size() << endl;

        
        for (const auto& arco : arcos_simplificados) {
            int i = arco.first;
            int j = arco.second;

            string nombre_var = "X_" + to_string(i) + "_" + to_string(j);
            X[arco] = modelo_asignacion.addVar(0.0, 1.0, 0.0, GRB_CONTINUOUS, nombre_var);
        }

        // ============================================= FUNCIÓN OBJETIVO ===============================================
        
        GRBLinExpr objetivo = 0;
        for (const auto& par : costos_simplificados) {
            const pair<int, int>& arco = par.first;
            double costo = par.second;
            objetivo += costo * X[arco];
        }
        modelo_asignacion.setObjective(objetivo, GRB_MINIMIZE);

        // ============================================== CALCULAR CAPACIDAD TOTAL =====================================================
        int capacidad_total = 0;
        for (int k = 1; k <= d; k++) {
            capacidad_total += multigrafo.getTerminales()[k - 1].getNumBuses();
        }
        cout << "Capacidad total de la flota: " << capacidad_total << " vehículos" << endl;

        // =================================================== RESTRICCIONES ====================================

        // Cada viaje debe ser cubierto exactamente una vez
        
        for (int j = d + 1; j <= d + n; j++) {
            GRBLinExpr suma_cobertura = 0;

            for (const auto& arco : arcos_simplificados) {
                if (arco.second == j) {  // Arco que llega al viaje j
                    suma_cobertura += X[arco];
                }
            }

            string nombre_constr = "cobertura_viaje_" + to_string(j);
            modelo_asignacion.addConstr(suma_cobertura == 1.0, nombre_constr);
        }

        // Conservación de flujo SOLO para viajes
       
        for (int j = d + 1; j <= d + n; j++) {
            GRBLinExpr flujo_entrante = 0;
            GRBLinExpr flujo_saliente = 0;

            for (const auto& arco : arcos_simplificados) {
                if (arco.second == j) {  // Arco entrante al viaje j
                    flujo_entrante += X[arco];
                }
                if (arco.first == j) {  // Arco saliente del viaje j
                    flujo_saliente += X[arco];
                }
            }

            string nombre_constr = "flujo_viaje_" + to_string(j);
            modelo_asignacion.addConstr(flujo_entrante == flujo_saliente, nombre_constr);
        }

        // Capacidad por depósito individual
        // Σ_{j} x_{kj} ≤ r_k, ∀k ∈ {1, ..., d}
        for (int k = 1; k <= d; k++) {
            GRBLinExpr vehiculos_deposito_k = 0;

            for (const auto& arco : arcos_simplificados) {
                if (arco.first == k) {  // Arco que sale del depósito k
                    vehiculos_deposito_k += X[arco];
                }
            }

            int capacidad_k = multigrafo.getTerminales()[k - 1].getNumBuses();
            string nombre_constr_k = "capacidad_deposito_" + to_string(k);
            modelo_asignacion.addConstr(vehiculos_deposito_k <= capacidad_k, nombre_constr_k);

            cout << "Depósito " << k << ": capacidad = " << capacidad_k << " vehículos" << endl;
        }

        // ============ RESOLVER MODELO ============
        cout << "\nResolviendo modelo de asignación..." << endl;
        

        // Configurar parámetros del solver
        modelo_asignacion.set(GRB_DoubleParam_TimeLimit, t);
        modelo_asignacion.set(GRB_DoubleParam_MIPGap, 0.01);

        modelo_asignacion.write("modelo_asignacion.lp");
        modelo_asignacion.optimize();

        auto fin = std::chrono::high_resolution_clock::now();
        double tiempo = std::chrono::duration<double>(fin - inicio).count();

        // ============ ANALIZAR RESULTADOS ============
        int num_vars = modelo_asignacion.get(GRB_IntAttr_NumVars);
        int num_constrs = modelo_asignacion.get(GRB_IntAttr_NumConstrs);
        int status = modelo_asignacion.get(GRB_IntAttr_Status);

        double valor_objetivo = 0.0;
        double mejor_cota_inferior = 0.0;
        double mejor_solucion = 0.0;
        double gap_final = 0.0;
        int nodos_branch_cut = 0;
        int vehiculos_totales = 0;
        string estado_str = "DESCONOCIDO";

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            estado_str = (status == GRB_OPTIMAL) ? "OPTIMO" : "LIMITE_TIEMPO";

            try {
                valor_objetivo = modelo_asignacion.get(GRB_DoubleAttr_ObjVal);
                mejor_cota_inferior = (status == GRB_OPTIMAL) ? valor_objetivo : modelo_asignacion.get(GRB_DoubleAttr_ObjBound);
                mejor_solucion = valor_objetivo;

                if (status == GRB_TIME_LIMIT) {
                    gap_final = modelo_asignacion.get(GRB_DoubleAttr_MIPGap);
                }

                cout << "Estado: " << ((status == GRB_OPTIMAL) ? "ÓPTIMO" : "LÍMITE DE TIEMPO") << endl;
                cout << "Valor objetivo (COTA INFERIOR): " << fixed << setprecision(6) << valor_objetivo << endl;

                // Contar vehículos utilizados
                for (const auto& par : X) {
                    double valor = par.second.get(GRB_DoubleAttr_X);
                    if (valor > 0.99) {  // Arco activo
                        const pair<int, int>& arco = par.first;
                        int origen = arco.first;
                        // Solo contar arcos que salen de depósitos (nodos 1 a d)
                        if (origen >= 1 && origen <= d) {
                            vehiculos_totales++;
                        }
                    }
                }

                // Calcular capacidad total para mostrar (ya calculada arriba)
                cout << "Total de vehículos utilizados: " << vehiculos_totales << endl;
                cout << "Capacidad total disponible: " << capacidad_total << endl;

                // Verificar factibilidad básica (capacidad_total ya calculada)
                if (vehiculos_totales <= capacidad_total) {
                    cout << " Respeta la capacidad total de la flota" << endl;
                }
                else {
                    cout << "Excede la capacidad total de la flota" << endl;
                }

                // Mostrar algunos arcos activos para verificación
                cout << "\n=== MUESTRA DE ARCOS ACTIVOS ===" << endl;
                int contador = 0;
                for (const auto& par : X) {
                    double valor = par.second.get(GRB_DoubleAttr_X);
                    if (valor > 0.99 && contador < 10) {
                        const pair<int, int>& arco = par.first;
                        cout << "Arco (" << arco.first << "," << arco.second << ") = " << valor
                            << " (costo: " << costos_simplificados[arco] << ")" << endl;
                        contador++;
                    }
                }

            }
            catch (const exception& e) {
                cout << "Error al procesar solución: " << e.what() << endl;
            }
        }
        else if (status == GRB_INFEASIBLE) {
            estado_str = "INFACTIBLE";
            cout << "Estado: INFACTIBLE" << endl;
            cout << "Esto podría indicar que no hay suficientes vehículos para cubrir todos los viajes." << endl;

            cout << "Computando IIS..." << endl;
            modelo_asignacion.computeIIS();
            modelo_asignacion.write("modelo_asignacion_infactible.ilp");
            cout << "IIS guardado en modelo_asignacion_infactible.ilp" << endl;
        }
        else {
            estado_str = "OTRO_" + to_string(status);
            cout << "Estado: " << status << endl;
        }

        try {
            nodos_branch_cut = (int)modelo_asignacion.get(GRB_DoubleAttr_NodeCount);
        }
        catch (...) {
            nodos_branch_cut = 0;
        }

        // ============ GUARDAR RESULTADOS ============
        // (capacidad_total ya calculada arriba)
        string nombre_archivo_csv = nombre_instancia + "_Asignacion.csv";

        bool archivo_existe = false;
        ifstream test_file(nombre_archivo_csv);
        if (test_file.good()) {
            archivo_existe = true;
        }
        test_file.close();

        ofstream csv_file(nombre_archivo_csv, ios::app);
        if (csv_file.is_open()) {
            if (!archivo_existe) {
                csv_file << "Instancia,NumVars,NumRestr,TiempoResolucion,CotaInferior,MejorSolucion,GapFinal,NodosBC,Estado,VehiculosUsados,CapacidadTotal\n";
            }

            csv_file << nombre_instancia << ","
                << num_vars << ","
                << num_constrs << ","
                << fixed << setprecision(2) << tiempo << ","
                << fixed << setprecision(6) << mejor_cota_inferior << ","
                << fixed << setprecision(6) << mejor_solucion << ","
                << fixed << setprecision(6) << gap_final << ","
                << nodos_branch_cut << ","
                << estado_str << ","
                << vehiculos_totales << ","
                << capacidad_total << "\n";

            csv_file.close();
            cout << "Resultados guardados en: " << nombre_archivo_csv << endl;
        }

        cout << "\n=== RESUMEN DEL MODELO DE ASIGNACIÓN ===" << endl;
        cout << "Variables: " << num_vars << endl;
        cout << "Restricciones: " << num_constrs << endl;
        cout << "Tiempo: " << fixed << setprecision(2) << tiempo << " segundos" << endl;
        cout << "COTA INFERIOR MDVSP: " << fixed << setprecision(6) << mejor_cota_inferior << endl;
        cout << "Capacidad total: " << capacidad_total << " vehículos" << endl;

    }
    catch (GRBException& e) {
        cerr << "Error de Gurobi: " << e.getMessage() << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}


double calcularCaminoMasCorto(const unordered_map<Arco, double>& arcos_costos,
    int origen, int destino, int deposito) {
    // Crear grafo para el depósito específico
    map<int, vector<pair<int, double>>> adj;

    for (const auto& par : arcos_costos) {
        const Arco& arco = par.first;
        if (arco.getDeposito() == deposito) {
            int u = arco.getOrigen();
            int v = arco.getDestino();
            double costo = par.second;
            adj[u].push_back({ v, costo });
        }
    }

    // Algoritmo de Dijkstra
    map<int, double> dist;
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;

    // Inicializar distancias
    for (const auto& nodo_adj : adj) {
        dist[nodo_adj.first] = numeric_limits<double>::infinity();
        for (const auto& vecino : nodo_adj.second) {
            dist[vecino.first] = numeric_limits<double>::infinity();
        }
    }

    if (dist.find(origen) == dist.end()) {
        return numeric_limits<double>::infinity();
    }

    dist[origen] = 0.0;
    pq.push({ 0.0, origen });

    while (!pq.empty()) {
        double d_actual = pq.top().first;
        int u = pq.top().second;
        pq.pop();

        if (d_actual > dist[u]) continue;

        if (adj.find(u) != adj.end()) {
            for (const auto& vecino : adj[u]) {
                int v = vecino.first;
                double peso = vecino.second;
                double nueva_dist = dist[u] + peso;

                if (nueva_dist < dist[v]) {
                    dist[v] = nueva_dist;
                    pq.push({ nueva_dist, v });
                }
            }
        }
    }

    return (dist.find(destino) != dist.end()) ? dist[destino] : numeric_limits<double>::infinity();
}


void resolverCaminoCorto(Multigrafo_vsp& multigrafo, const string& nombre_instancia) {
    try {
        cout << "\n=== RESOLVIENDO COTA DE CAMINO MÁS CORTO (ALGORITMO OPTIMIZADO) ===\n";

        // Obtener datos del problema
        int n = multigrafo.getn();  // número de viajes
        int d = multigrafo.getd();  // número de depósitos

        cout << "Viajes: " << n << ", Depósitos: " << d << endl;
        cout << "Estructura del grafo:" << endl;
        cout << "  Depósitos de salida: {1, ..., " << d << "}" << endl;
        cout << "  Viajes: {" << (d + 1) << ", ..., " << (d + n) << "}" << endl;
        cout << "  Depósitos de llegada: {" << (n + d + 1) << ", ..., " << (n + 2 * d) << "}" << endl;

        auto inicio_total = std::chrono::high_resolution_clock::now();

        // Obtener arcos del multigrafo
        const unordered_map<Arco, double>& arcos_costos = multigrafo.getC_original();

        // Obtener depósitos disponibles
        set<int> todos_depositos;
        for (const auto& par : arcos_costos) {
            todos_depositos.insert(par.first.getDeposito());
        }

        cout << "Depósitos encontrados: ";
        for (int k : todos_depositos) {
            cout << k << " ";
        }
        cout << endl;

        // Variables para resultados
        double mejor_cota = 0.0;
        int mejor_viaje = -1;
        int mejor_deposito = -1;
        vector<tuple<int, double, int>> resultados_por_viaje; // (viaje, SP_v, deposito_critico)

        // ALGORITMO PRINCIPAL: Para cada viaje v ∈ N = {d+1, ..., d+n}
        for (int v = d + 1; v <= d + n; v++) {
            cout << "\n--- Procesando viaje " << v << " (viaje " << (v - d) << ") ---" << endl;

            double mejor_sp_v = numeric_limits<double>::infinity();
            int deposito_critico = -1;

            // 1.1. Para cada depósito k ∈ D
            for (int k : todos_depositos) {
                cout << "  Depósito " << k << ":" << endl;

                // 1.1.1. Calcular camino más corto de k hasta v
                double costo_k_a_v = calcularCaminoMasCorto(arcos_costos, k, v, k);
                cout << "    Camino " << k << " → " << v << ": ";
                if (costo_k_a_v == numeric_limits<double>::infinity()) {
                    cout << "INFINITO" << endl;
                    continue;
                }
                else {
                    cout << fixed << setprecision(4) << costo_k_a_v << endl;
                }

                // 1.1.2. Calcular camino más corto desde v hasta n+d+k (depósito de llegada)
                int deposito_llegada_k = n + d + k;
                double costo_v_a_k_llegada = calcularCaminoMasCorto(arcos_costos, v, deposito_llegada_k, k);
                cout << "    Camino " << v << " → " << deposito_llegada_k << ": ";
                if (costo_v_a_k_llegada == numeric_limits<double>::infinity()) {
                    cout << "INFINITO" << endl;
                    continue;
                }
                else {
                    cout << fixed << setprecision(4) << costo_v_a_k_llegada << endl;
                }

                // 1.1.3. SP(v,k) := suma de los costos de los dos caminos
                double sp_v_k = costo_k_a_v + costo_v_a_k_llegada;
                cout << "    SP(" << v << "," << k << ") = " << fixed << setprecision(6) << sp_v_k << endl;

                // 1.2. SP(v) = min {SP(v,k) : k ∈ D}
                if (sp_v_k < mejor_sp_v) {
                    mejor_sp_v = sp_v_k;
                    deposito_critico = k;
                }
            }

            cout << "  → SP(" << v << ") = " << fixed << setprecision(6) << mejor_sp_v;
            cout << " (depósito crítico: " << deposito_critico << ")" << endl;

            // Guardar resultado para este viaje
            resultados_por_viaje.push_back({ v, mejor_sp_v, deposito_critico });

            // 2. SP = max {SP(v) : v ∈ N}
            if (mejor_sp_v != numeric_limits<double>::infinity() && mejor_sp_v > mejor_cota) {
                mejor_cota = mejor_sp_v;
                mejor_viaje = v;
                mejor_deposito = deposito_critico;
            }
        }

        auto fin_total = std::chrono::high_resolution_clock::now();
        double tiempo_total = std::chrono::duration<double>(fin_total - inicio_total).count();

        // ============ RESULTADOS FINALES ============
        cout << "\n=== RESULTADOS DEL ALGORITMO DE CAMINOS MÁS CORTOS ===" << endl;
        cout << "Cota inferior (SP): " << fixed << setprecision(6) << mejor_cota << endl;
        cout << "Viaje crítico: " << mejor_viaje << " (viaje " << (mejor_viaje - d) << ")" << endl;
        cout << "Depósito crítico: " << mejor_deposito << endl;
        cout << "Tiempo total: " << setprecision(2) << tiempo_total << " segundos" << endl;

        // Mostrar resumen de todos los viajes
        cout << "\nResumen por viaje:" << endl;
        for (const auto& resultado : resultados_por_viaje) {
            int viaje = get<0>(resultado);
            double sp_v = get<1>(resultado);
            int dep_critico = get<2>(resultado);
            cout << "  Viaje " << viaje << " (viaje " << (viaje - d) << "): SP = "
                << fixed << setprecision(6) << sp_v << " (depósito " << dep_critico << ")" << endl;
        }

        // Guardar resultados en CSV
        string nombre_archivo = nombre_instancia + "_CaminoCorto.csv";
        bool archivo_existe = ifstream(nombre_archivo).good();

        ofstream csv(nombre_archivo, ios::app);
        if (csv.is_open()) {
            if (!archivo_existe) {
                csv << "Instancia,CotaInferior,ViajeCritico,DepositoCritico,TiempoTotal,NumViajes,NumDepositos\n";
            }

            csv << nombre_instancia << ","
                << fixed << setprecision(6) << mejor_cota << ","
                << mejor_viaje << ","
                << mejor_deposito << ","
                << setprecision(2) << tiempo_total << ","
                << n << ","
                << d << "\n";

            csv.close();
            cout << "Resultados guardados en: " << nombre_archivo << endl;
        }
        else {
            cerr << "Error: No se pudo abrir el archivo " << nombre_archivo << endl;
        }

        // Opcional: Guardar detalles de todos los viajes
        string nombre_detalle = nombre_instancia + "_CaminoCorto_Detalle.csv";
        ofstream csv_detalle(nombre_detalle);
        if (csv_detalle.is_open()) {
            csv_detalle << "Instancia,Viaje,IndiceViaje,SP_v,DepositoCritico\n";
            for (const auto& resultado : resultados_por_viaje) {
                int viaje = get<0>(resultado);
                double sp_v = get<1>(resultado);
                int dep_critico = get<2>(resultado);
                csv_detalle << nombre_instancia << ","
                    << viaje << ","
                    << (viaje - d) << ","
                    << fixed << setprecision(6) << sp_v << ","
                    << dep_critico << "\n";
            }
            csv_detalle.close();
            cout << "Detalles guardados en: " << nombre_detalle << endl;
        }

    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}


void resolverCotaAditiva(Multigrafo_vsp& multigrafo, const string& nombre_instancia) {
    try {
        cout << "\n=== RESOLVIENDO COTA ADITIVA PARA MDVSP ===\n";

        // Obtener datos del problema
        int n = multigrafo.getn();  // número de viajes
        int d = multigrafo.getd();  // número de depósitos

        cout << "Viajes: " << n << ", Depósitos: " << d << endl;
        cout << "Estructura del grafo:" << endl;
        cout << "  Depósitos de salida: {1, ..., " << d << "}" << endl;
        cout << "  Viajes: {" << (d + 1) << ", ..., " << (d + n) << "}" << endl;
        cout << "  Depósitos de llegada: {" << (n + d + 1) << ", ..., " << (n + 2 * d) << "}" << endl;

        auto inicio_total = std::chrono::high_resolution_clock::now();

        // Obtener arcos del multigrafo
        const unordered_map<Arco, double>& arcos_costos = multigrafo.getC_original();

        // Obtener todos los depósitos
        set<int> todos_depositos;
        for (const auto& par : arcos_costos) {
            todos_depositos.insert(par.first.getDeposito());
        }

        // PASO 1: Calcular z_ADD = v(AP) usando resolverModeloAsignacion
        cout << "\nPASO 1: Calculando v(AP) con problema de asignación..." << endl;

        // Crear entorno y modelo de Gurobi para el problema de asignación
        GRBEnv env_asignacion = GRBEnv(true);
        env_asignacion.set("LogFile", "cota_aditiva_asignacion.log");
        env_asignacion.start();
        GRBModel modelo_asignacion = GRBModel(env_asignacion);

        // Variables y restricciones del modelo de asignación (reutilizando lógica de resolverModeloAsignacion)
        map<pair<int, int>, GRBVar> X;
        set<pair<int, int>> arcos_simplificados;
        map<pair<int, int>, double> costos_simplificados;

        // Simplificar arcos: eliminar la dimensión del depósito
        for (const auto& par : arcos_costos) {
            const Arco& arco = par.first;
            double costo = par.second;
            int origen = arco.getOrigen();
            int destino = arco.getDestino();
            pair<int, int> arco_simple = { origen, destino };
            costos_simplificados[arco_simple] = costo;
            arcos_simplificados.insert(arco_simple);
        }

        // Crear variables X_ij
        for (const auto& arco : arcos_simplificados) {
            int i = arco.first;
            int j = arco.second;
            string nombre_var = "X_" + to_string(i) + "_" + to_string(j);
            X[arco] = modelo_asignacion.addVar(0.0, 1.0, 0.0, GRB_BINARY, nombre_var);
        }

        // Función objetivo: minimizar suma de costos
        GRBLinExpr objetivo = 0;
        for (const auto& par : costos_simplificados) {
            const pair<int, int>& arco = par.first;
            double costo = par.second;
            objetivo += costo * X[arco];
        }
        modelo_asignacion.setObjective(objetivo, GRB_MINIMIZE);

        // Restricción 1: Cada viaje debe ser cubierto exactamente una vez
        for (int j = d + 1; j <= d + n; j++) {
            GRBLinExpr suma_cobertura = 0;
            for (const auto& arco : arcos_simplificados) {
                if (arco.second == j) {
                    suma_cobertura += X[arco];
                }
            }
            string nombre_constr = "cobertura_viaje_" + to_string(j);
            modelo_asignacion.addConstr(suma_cobertura == 1.0, nombre_constr);
        }

        // Restricción 2: Conservación de flujo para viajes
        for (int j = d + 1; j <= d + n; j++) {
            GRBLinExpr flujo_entrante = 0;
            GRBLinExpr flujo_saliente = 0;

            for (const auto& arco : arcos_simplificados) {
                if (arco.second == j) {
                    flujo_entrante += X[arco];
                }
                if (arco.first == j) {
                    flujo_saliente += X[arco];
                }
            }

            string nombre_constr = "flujo_viaje_" + to_string(j);
            modelo_asignacion.addConstr(flujo_entrante == flujo_saliente, nombre_constr);
        }

        // Restricción 3: Capacidad por depósito
        for (int k = 1; k <= d; k++) {
            GRBLinExpr vehiculos_deposito_k = 0;
            for (const auto& arco : arcos_simplificados) {
                if (arco.first == k) {
                    vehiculos_deposito_k += X[arco];
                }
            }
            int capacidad_k = multigrafo.getTerminales()[k - 1].getNumBuses();
            string nombre_constr_k = "capacidad_deposito_" + to_string(k);
            modelo_asignacion.addConstr(vehiculos_deposito_k <= capacidad_k, nombre_constr_k);
        }

        // Resolver modelo de asignación
        modelo_asignacion.set(GRB_DoubleParam_TimeLimit, 3600);
        modelo_asignacion.set(GRB_DoubleParam_MIPGap, 0.01);
        modelo_asignacion.optimize();

        double z_ADD = 0.0;
        int status = modelo_asignacion.get(GRB_IntAttr_Status);

        if (status == GRB_OPTIMAL || status == GRB_TIME_LIMIT) {
            z_ADD = modelo_asignacion.get(GRB_DoubleAttr_ObjVal);
            cout << "v(AP) = " << fixed << setprecision(6) << z_ADD << endl;
        }
        else {
            cout << "Error: No se pudo resolver el problema de asignación (status: " << status << ")" << endl;
            return;
        }

        // PASO 2: Algoritmo iterativo para mejorar la cota
        cout << "\nPASO 2: Iniciando algoritmo iterativo de cota aditiva..." << endl;

        int iteracion = 0;
        double mejora_total = 0.0;
        bool continuar = true;

        while (continuar) {
            iteracion++;
            cout << "\n--- Iteración " << iteracion << " ---" << endl;

            double delta_mu = 0.0;
            int mejor_deposito = -1;
            int mejor_viaje = -1;

            // Para cada depósito k y cada viaje i, calcular δ_ki
            for (int k : todos_depositos) {
                for (int i = d + 1; i <= d + n; i++) {
                    // Calcular λ_ki: camino más corto desde depósito k al viaje i
                    double lambda_ki = calcularCaminoMasCorto(arcos_costos, k, i, k);

                    // Calcular λ_ik: camino más corto desde viaje i al depósito de llegada n+d+k
                    int deposito_llegada_k = n + d + k;
                    double lambda_ik = calcularCaminoMasCorto(arcos_costos, i, deposito_llegada_k, k);

                    if (lambda_ki != numeric_limits<double>::infinity() &&
                        lambda_ik != numeric_limits<double>::infinity()) {

                        // δ_ki = λ_ki + λ_ik (según la fórmula de la imagen)
                        double delta_ki = lambda_ki + lambda_ik;

                        cout << "  δ_" << k << "," << (i - d) << " = " << fixed << setprecision(6)
                            << delta_ki << " (λ_ki=" << lambda_ki << " + λ_ik=" << lambda_ik << ")" << endl;

                        // δ_μ = max{δ_ki} para todos k,i
                        if (delta_ki > delta_mu) {
                            delta_mu = delta_ki;
                            mejor_deposito = k;
                            mejor_viaje = i;
                        }
                    }
                }
            }

            cout << "δ_μ = " << fixed << setprecision(6) << delta_mu << endl;
            if (mejor_deposito != -1 && mejor_viaje != -1) {
                cout << "Mejor par (k,i): (" << mejor_deposito << "," << (mejor_viaje - d) << ")" << endl;
            }

            // Verificar si hay mejora significativa
            if (delta_mu < 1e-6) {
                cout << "No hay mejora significativa (δ_μ < 1e-6). Terminando..." << endl;
                continuar = false;
            }
            else {
                // Actualizar z_ADD según la fórmula: z_ADD = z_ADD + δ_μ
                z_ADD += delta_mu;
                mejora_total += delta_mu;

                cout << "z_ADD actualizado: " << fixed << setprecision(6) << z_ADD
                    << " (mejora: +" << delta_mu << ")" << endl;

                // Limitamos las iteraciones para evitar bucles infinitos
                if (iteracion >= 100) {
                    cout << "Límite de iteraciones alcanzado (100)" << endl;
                    continuar = false;
                }
            }
        }

        auto fin_total = std::chrono::high_resolution_clock::now();
        double tiempo_total = std::chrono::duration<double>(fin_total - inicio_total).count();

        // Calcular capacidad total
        int capacidad_total = 0;
        for (int k = 1; k <= d; k++) {
            capacidad_total += multigrafo.getTerminales()[k - 1].getNumBuses();
        }

        // Variables para el CSV (similar a resolverModeloAsignacion)
        int num_vars = modelo_asignacion.get(GRB_IntAttr_NumVars);
        int num_constrs = modelo_asignacion.get(GRB_IntAttr_NumConstrs);
        string estado_str = (status == GRB_OPTIMAL) ? "OPTIMO" : "LIMITE_TIEMPO";

        // ============ RESULTADOS FINALES ============
        cout << "\n=== RESULTADOS DE LA COTA ADITIVA ===" << endl;
        cout << "Cota aditiva final (z_ADD): " << fixed << setprecision(6) << z_ADD << endl;
        cout << "Valor inicial v(AP): " << fixed << setprecision(6) << (z_ADD - mejora_total) << endl;
        cout << "Mejora total obtenida: " << fixed << setprecision(6) << mejora_total << endl;
        cout << "Iteraciones realizadas: " << iteracion << endl;
        cout << "Variables del modelo AP: " << num_vars << endl;
        cout << "Restricciones del modelo AP: " << num_constrs << endl;
        cout << "Tiempo total: " << setprecision(2) << tiempo_total << " segundos" << endl;
        cout << "Capacidad total: " << capacidad_total << " vehículos" << endl;

        // Guardar resultados en CSV (formato similar a resolverModeloAsignacion)
        string nombre_archivo_csv = nombre_instancia + "_CotaAditiva.csv";

        bool archivo_existe = false;
        ifstream test_file(nombre_archivo_csv);
        if (test_file.good()) {
            archivo_existe = true;
        }
        test_file.close();

        ofstream csv_file(nombre_archivo_csv, ios::app);
        if (csv_file.is_open()) {
            if (!archivo_existe) {
                csv_file << "Instancia,NumVars,NumRestr,TiempoResolucion,CotaAditiva,ValorInicialAP,MejoraTotal,Iteraciones,Estado,CapacidadTotal\n";
            }

            csv_file << nombre_instancia << ","
                << num_vars << ","
                << num_constrs << ","
                << fixed << setprecision(2) << tiempo_total << ","
                << fixed << setprecision(6) << z_ADD << ","
                << fixed << setprecision(6) << (z_ADD - mejora_total) << ","
                << fixed << setprecision(6) << mejora_total << ","
                << iteracion << ","
                << estado_str << ","
                << capacidad_total << "\n";

            csv_file.close();
            cout << "Resultados guardados en: " << nombre_archivo_csv << endl;
        }
        else {
            cerr << "Error: No se pudo abrir el archivo " << nombre_archivo_csv << endl;
        }

    }
    catch (GRBException& e) {
        cerr << "Error de Gurobi: " << e.getMessage() << endl;
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
    }
}


// Estructura para almacenar información de rutas
struct InfoRuta {
    string nombre;
    double valor;
    double costo_original;
    int viajes_cubiertos;
    string camino_str;
    vector<int> camino_vector;
};

void verificarSolucionFinal(const Multigrafo_vsp& multigrafo, const vector<vector <int> >& rutas_activas) {
    cout << "\n=== VERIFICANDO SOLUCIÓN FINAL ===" << endl;


    cout << "Total de rutas activas: " << rutas_activas.size() << endl;

    // Verificar factibilidad
    bool es_factible = verificarSolucionFactible(rutas_activas, multigrafo, true);

    if (es_factible) {
        cout << " ¡La solución generada es FACTIBLE!" << endl;
    }
    else {
        cout << " La solución generada NO es factible." << endl;
    }
}

void imprimirRutas(const vector<vector<int>>& rutas) {
    cout << "=== RUTAS ===" << endl;
    cout << rutas.size();
    for (size_t i = 0; i < rutas.size(); i++) {
        cout << "Ruta " << (i + 1) << ": ";
        for (size_t j = 0; j < rutas[i].size(); j++) {
            cout << rutas[i][j];
            if (j < rutas[i].size() - 1) cout << " , ";
        }
        cout << endl;
    }
}

int main() {
    try {
        // Datos del problema 
        const double COSTO_COLUMNA_ARTIFICIAL = 500;
        string nombre_instancia = "QMCG"; 
        double tiempo_total_LP = 0.0;  // Tiempo acumulado resolviendo problemas master reducido
        double tiempo_total_DAG = 0.0; // Tiempo acumulado encontrando caminos
        int num_iteraciones_final = 0;
        int num_columnas_anadidas = 0;
        double valor_objetivo_final = 0.0;
        double tiempo_ejecucion_total = 0.0;
        bool termino_optimo = false;
        double valor_IP_final = 0.0;
        double gap_final = 0.0;

        // Vector para almacenar información de todas las rutas creadas
        vector<InfoRuta> info_rutas_creadas;

        // Vector para guardar el progreso de cada iteración
        std::vector<IteracionInfo> historial_progreso;

        //iniciar el tiempo
        auto inicio_total = std::chrono::high_resolution_clock::now();

        // Cargar datos desde archivos CSV
        Multigrafo_vsp_con_DAG multigrafo_dag(0, 0);
        multigrafo_dag.leerTerminales("QMCG_terminales.csv");
        vector<Terminal> t=multigrafo_dag.getTerminales();
        multigrafo_dag.leerViajes("QMCG_Viajes.csv");
        multigrafo_dag.construirArcos();
        for (int i = 0; i < multigrafo_dag.getd(); i++)
        {
            t[i].imprimir();
        }
        //multigrafo_dag.imprimirMultigrafo();

        // Obtener datos necesarios de la clase
        int n = multigrafo_dag.getn();  
        int d = multigrafo_dag.getd(); 

        //cout << "Problema VSP: " << n << " viajes, " << d << " depósitos" << endl;
        //cout << "Numeración de nodos:" << endl;
        //cout << "- Depósitos salida: {1, ..., " << d << "}" << endl;
        //cout << "- Viajes: {" << (d + 1) << ", ..., " << (d + n) << "}" << endl;
        //cout << "- Depósitos llegada: {" << (n + d + 1) << ", ..., " << (n + 2 * d) << "}" << endl;


        // Crear entorno y modelo de Gurobi
        GRBEnv env = GRBEnv(true);
        //env.set("LogFile", "multigrafo_vsp.log");
        env.start();
        GRBModel model = GRBModel(env);

        // Variables para viajes: x_i (una por cada nodo de viaje)
        map<int, GRBVar> x_viaje;
        for (int i = 0; i < n; i++) {
            int nodo_viaje = d + 1 + i;  // nodos {d+1, ..., d+n}
            x_viaje[nodo_viaje] = model.addVar(0.0, GRB_INFINITY,
                COSTO_COLUMNA_ARTIFICIAL,
                GRB_CONTINUOUS,
                "x_viaje_nodo_" + to_string(nodo_viaje));
        }

        // Variables para depósitos: x_k (una por cada depósito)
        map<int, GRBVar> x_deposito;
        for (int k = 0; k < d; k++) {
            int nodo_deposito = 1 + k;  // nodos {1, ..., d}
            int capacidad_deposito = multigrafo_dag.getTerminales()[k].getNumBuses();

            x_deposito[nodo_deposito] = model.addVar(0.0, GRB_INFINITY,
                COSTO_COLUMNA_ARTIFICIAL,
                GRB_CONTINUOUS,
                "x_deposito_nodo_" + to_string(nodo_deposito));
        }

        // Función objetivo: minimizar costo total
        GRBLinExpr obj = 0;
        for (int i = 0; i < n; i++) {
            int nodo_viaje = d + 1 + i;
            obj += COSTO_COLUMNA_ARTIFICIAL * x_viaje[nodo_viaje];
        }
        for (int k = 0; k < d; k++) {
            int nodo_deposito = 1 + k;
            obj += COSTO_COLUMNA_ARTIFICIAL * x_deposito[nodo_deposito];
        }
        model.setObjective(obj, GRB_MINIMIZE);

        // Almacenar las restricciones para obtener precios duales
        map<int, GRBConstr> constr_viajes;
        map<int, GRBConstr> constr_depositos;

        // Restricciones tipo i: x_i = 1 para todo viaje i
        for (int i = 0; i < n; i++) {
            int nodo_viaje = d + 1 + i;
            constr_viajes[nodo_viaje] = model.addConstr(x_viaje[nodo_viaje] == 1.0,
                "cobertura_viaje" + to_string(nodo_viaje));
        }

        // Restricciones tipo ii: x_k <= num_buses(k) para todo depósito k
        for (int k = 0; k < d; k++) {
            int nodo_deposito = 1 + k;
            int capacidad = multigrafo_dag.getTerminales()[k].getNumBuses();
            constr_depositos[nodo_deposito] = model.addConstr(-x_deposito[nodo_deposito] >= -capacidad,
                "capacidad_deposito" + to_string(nodo_deposito));
        }


        // Resolver el modelo INICIAL 
        //cout << "\nResolviendo modelo PL inicial..." << endl;
        auto inicio_lp = std::chrono::high_resolution_clock::now();
        model.optimize();
        auto fin_lp = std::chrono::high_resolution_clock::now();
        double tiempo_lp_inicial = std::chrono::duration<double>(fin_lp - inicio_lp).count();
        tiempo_total_LP += tiempo_lp_inicial;
        //cout << "Tiempo LP inicial: " << tiempo_lp_inicial << " segundos" << endl;

        /////////////////////////////////////////////////////////////////////////////////////////////// inicio
        //for (const auto& par : x_viaje) {
        //    double valor = par.second.get(GRB_DoubleAttr_X);
        //    if (valor > 1e-6) {
        //        std::cout << "Variable básica: x[" << par.first << "] = " << valor << std::endl;
        //    }
        //}
        ///////////////////////////////////////////////////////////////////////////////////////////// fin


        // Mostrar resultados iniciales
        if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
            //cout << "\n=== SOLUCION INICIAL ENCONTRADA ===" << endl;
            //cout << "Costo inicial: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

            // Guardar información de la iteración inicial (0)
            IteracionInfo info_inicial;
            info_inicial.num_iteracion = 0;
            info_inicial.tiempo_acumulado_segundos = std::chrono::duration<double>(fin_lp - inicio_total).count();
            info_inicial.valor_objetivo = model.get(GRB_DoubleAttr_ObjVal);
            info_inicial.columnas_anadidas_iteracion = 0;
            historial_progreso.push_back(info_inicial);

            // Obtener precios duales iniciales
            map<int, double> alpha;
            map<int, double> beta;

            //Obteniendo precios duales iniciales
            for (int i = 0; i < n; i++) {
                int nodo_viaje = d + 1 + i;
                alpha[nodo_viaje] = constr_viajes[nodo_viaje].get(GRB_DoubleAttr_Pi);
            }

            for (int k = 0; k < d; k++) {
                int nodo_deposito = 1 + k;
                beta[nodo_deposito] = constr_depositos[nodo_deposito].get(GRB_DoubleAttr_Pi);
            }


            //cout << "\n=== PRECIOS DUALES INICIALES ===" << endl;
            //cout << "Precios duales de viajes (alpha):" << endl;
            //for (const auto& par : alpha) {
            //    cout << "  alpha[" << par.first << "] = " << par.second << endl;
            //}
            //cout << "Precios duales de depósitos (beta):" << endl;
            //for (const auto& par : beta) {
            //    cout << "  beta[" << par.first << "] = " << par.second << endl;
            //}

            // ============ ALGORITMO DE GENERACIÓN DE COLUMNAS ==================================
            //cout << "\n=== INICIANDO GENERACION DE COLUMNAS ===" << endl;
            auto inicio_algoritmo = std::chrono::high_resolution_clock::now();
            const double LIMITE_TIEMPO_HORAS = 8;
            const double LIMITE_TIEMPO_SEGUNDOS = LIMITE_TIEMPO_HORAS * 3600;  // Convertir a segundos
            //cout << "Limite de tiempo establecido: " << LIMITE_TIEMPO_HORAS << " horas (" << LIMITE_TIEMPO_SEGUNDOS << " segundos)" << endl;

            multigrafo_dag.actualizarCostosConDuales(alpha, beta);

            int count = 0;  // Contador de columnas añadidas
            int iteracion = 0;
            bool limite_tiempo_alcanzado = false;  //  Se alcanzó el límite de tiempo?

            // Variables para almacenar las nuevas columnas
            vector<GRBVar> columnas_nuevas;

            while (!limite_tiempo_alcanzado) {
                iteracion++;
                //cout << "\n--- ITERACION " << iteracion << " ---" << endl;

                // ============ TIEMPO============
                auto tiempo_actual = std::chrono::high_resolution_clock::now();
                double tiempo_transcurrido = std::chrono::duration<double>(tiempo_actual - inicio_algoritmo).count();
                double tiempo_restante = LIMITE_TIEMPO_SEGUNDOS - tiempo_transcurrido;

                //cout << "Tiempo transcurrido: " << tiempo_transcurrido << " segundos ("
                //    << (tiempo_transcurrido / 3600.0) << " horas)" << endl;
                //cout << "Tiempo restante: " << tiempo_restante << " segundos ("
                //    << (tiempo_restante / 3600.0) << " horas)" << endl;

                if (tiempo_transcurrido >= LIMITE_TIEMPO_SEGUNDOS) {
                    //cout << "\n LIMITE DE TIEMPO ALCANZADO (" << LIMITE_TIEMPO_HORAS << " horas)" << endl;
                    //cout << "Deteniendo algoritmo de generación de columnas" << endl;
                    limite_tiempo_alcanzado = true;
                    break;
                }

                // ============ MEDIR TIEMPO DE BÚSQUEDA DAG ============
                //cout << "\n Buscando caminos negativos en TODOS los depositos" << endl;
                auto inicio_dag = std::chrono::high_resolution_clock::now();

                vector<pair<int, vector<ResultadoCamino>>> caminos_por_deposito = multigrafo_dag.encontrarPrimerosKCaminosNegativosTodos(140);
                if (caminos_por_deposito.empty()) {
                    //cout << "\n No se encontraron más caminos negativos en ningún depósito." << endl;
                    termino_optimo = true;
                    break;
                }

                auto fin_dag = std::chrono::high_resolution_clock::now();
                double tiempo_dag_iteracion = std::chrono::duration<double>(fin_dag - inicio_dag).count();
                tiempo_total_DAG += tiempo_dag_iteracion;
                //cout << "Tiempo búsqueda DAG esta iteración: " << tiempo_dag_iteracion << " segundos" << endl;

                // ============ PROCESAR CADA DEPÓSITO Y SUS CAMINOS ============
                int columnas_agregadas_iteracion = 0;

                for (const auto& par_deposito : caminos_por_deposito) {
                    int deposito_origen = par_deposito.first;
                    vector<ResultadoCamino> caminos_deposito = par_deposito.second;

                    //cout << "Procesando " << caminos_deposito.size() << " caminos del depósito " << deposito_origen << endl;

                    // Procesar cada camino de este depósito
                    for (const auto& camino_negativo : caminos_deposito) {
                        //cout << "Procesando camino con costo: " << camino_negativo.getDistancia() << endl;

                        // ============ CREAR NUEVA COLUMNA ============
                        count++;
                        columnas_agregadas_iteracion++;
                        string nombre_columna = "ruta_" + to_string(count);

                        // Analizar el camino para determinar qué viajes cubre y qué depósito usa
                        list<Arco> camino = camino_negativo.getCamino();
                        set<int> viajes_cubiertos;  // Nodos de viajes que pasa el camino
                        int deposito_usado = deposito_origen;    // Depósito que usa el camino

                        // ============ CALCULAR COSTO ORIGINAL DEL CAMINO ============
                        double costo_original_total = 0.0;

                        //cout << "Analizando camino y calculando costo original:" << endl;
                        for (const auto& arco : camino) {
                            int origen = arco.getOrigen();
                            int destino = arco.getDestino();
                            int deposito = arco.getDeposito();

                            // Obtener el costo original del arco
                            double costo_arco_original = multigrafo_dag.getC_original().at(arco);
                            costo_original_total += costo_arco_original;

                            //cout << "  Arco: (" << origen << "," << destino << "," << deposito
                            //    << ") - Costo original: " << costo_arco_original << endl;

                            // Si el origen es un viaje (nodos d+1 a d+n)
                            if (origen >= d + 1 && origen <= d + n) {
                                viajes_cubiertos.insert(origen);
                                //cout << "    -> Cubre viaje en nodo " << origen << " (viaje " << (origen - d) << ")" << endl;
                            }
                            // Si el destino es un viaje (nodos d+1 a d+n)  
                            if (destino >= d + 1 && destino <= d + n) {
                                viajes_cubiertos.insert(destino);
                                //cout << "    -> Cubre viaje en nodo " << destino << " (viaje " << (destino - d) << ")" << endl;
                            }
                        }

                        //cout << "Costo DAG: " << camino_negativo.getDistancia() << endl;
                        //cout << "Costo original total: " << costo_original_total << endl;
                        //cout << "Total de viajes cubiertos: " << viajes_cubiertos.size() << endl;
                        //cout << "Usa depósito " << deposito_usado << endl;

                        // Después de calcular costo_original_total y antes de crear la columna
                        InfoRuta info_nueva_ruta;
                        info_nueva_ruta.nombre = nombre_columna;
                        info_nueva_ruta.costo_original = costo_original_total;
                        info_nueva_ruta.viajes_cubiertos = viajes_cubiertos.size();

                        // Construir string del camino
                        string camino_str = "";
                        vector<int> camino_vector;
                        bool primer_nodo = true;
                        for (const auto& arco : camino) {
                            if (primer_nodo) {
                                camino_str += to_string(arco.getOrigen());
                                camino_vector.push_back(arco.getOrigen());
                                primer_nodo = false;
                            }
                            camino_str += " , " + to_string(arco.getDestino());
                            camino_vector.push_back(arco.getDestino());
                        }
                        info_nueva_ruta.camino_str = camino_str;
                        info_nueva_ruta.camino_vector = camino_vector;

                        info_rutas_creadas.push_back(info_nueva_ruta);

                        // ============ CREAR COLUMNA CON COEFICIENTES CORRECTOS ============
                        // Preparar columna para restricciones de viajes
                        GRBColumn columna_nueva;

                        //cout << "Configurando coeficientes para restricciones de viajes:" << endl;
                        for (int i = 0; i < n; i++) {
                            int nodo_viaje = d + 1 + i;
                            if (viajes_cubiertos.count(nodo_viaje) > 0) {
                                // Este viaje está cubierto por la nueva ruta
                                columna_nueva.addTerm(1.0, constr_viajes[nodo_viaje]);
                                //cout << "  Viaje nodo " << nodo_viaje << " (viaje " << (i + 1) << "): coeficiente = 1" << endl;
                            }
                        }

                        //cout << "Configurando coeficientes para restricciones de depósitos:" << endl;
                        for (int k = 0; k < d; k++) {
                            int nodo_deposito = 1 + k;
                            if (nodo_deposito == deposito_usado) {
                                // Este depósito es usado por la nueva ruta 
                                columna_nueva.addTerm(-1.0, constr_depositos[nodo_deposito]);
                                //cout << "  Depósito nodo " << nodo_deposito << ": coeficiente = -1" << endl;
                            }
                        }

                        // 3. Crear la variable con la columna completa usando el COSTO ORIGINAL
                        //cout << " Creando columna: " << nombre_columna << " con costo original " << costo_original_total << endl;
                        GRBVar nueva_columna = model.addVar(0.0, GRB_INFINITY, costo_original_total, GRB_CONTINUOUS, columna_nueva, nombre_columna);
                        columnas_nuevas.push_back(nueva_columna);
                        //cout << " Columna " << nombre_columna << " agregada al modelo." << endl;
                    }
                }

                cout << "\n Columnas agregadas en esta iteracion: " << columnas_agregadas_iteracion << endl;
                num_columnas_anadidas += columnas_agregadas_iteracion;

                // ============ MEDIR TIEMPO DE RESOLUCIÓN LP ============
                model.update();
                auto inicio_lp_iter = std::chrono::high_resolution_clock::now();

                model.optimize();

                auto fin_lp_iter = std::chrono::high_resolution_clock::now();
                double tiempo_lp_iteracion = std::chrono::duration<double>(fin_lp_iter - inicio_lp_iter).count();
                tiempo_total_LP += tiempo_lp_iteracion;
                //cout << "Tiempo LP esta iteración: " << tiempo_lp_iteracion << " segundos" << endl;

                if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                    cout << "Error: No se pudo resolver el modelo actualizado." << endl;
                    break;
                }

                //cout << "Modelo resuelto. Nuevo costo: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

                ////////////////////////////////////////////////////////////

                // ============ GUARDAR PROGRESO DE ESTA ITERACIÓN ============
                auto tiempo_fin_iteracion = std::chrono::high_resolution_clock::now();
                double tiempo_acumulado_hasta_ahora = std::chrono::duration<double>(tiempo_fin_iteracion - inicio_total).count();

                IteracionInfo info_iteracion;
                info_iteracion.num_iteracion = iteracion;
                info_iteracion.tiempo_acumulado_segundos = tiempo_acumulado_hasta_ahora;
                info_iteracion.valor_objetivo = model.get(GRB_DoubleAttr_ObjVal);
                info_iteracion.columnas_anadidas_iteracion = columnas_agregadas_iteracion;

                historial_progreso.push_back(info_iteracion);

                ////////////////////////////////////////////////////

                //for (const auto& par : x_viaje) {
                //    double valor = par.second.get(GRB_DoubleAttr_X);
                //    if (valor > 1e-6) {
                //        std::cout << "Variable básica: x[" << par.first << "] = " << valor << std::endl;
                //    }
                //}

                // ============ ACTUALIZAR PRECIOS DUALES ============
                //cout << "\n Actualizando precios duales..." << endl;

                // Verificar que el modelo está en estado óptimo
                if (model.get(GRB_IntAttr_Status) != GRB_OPTIMAL) {
                    cout << "ERROR: El modelo no está en estado óptimo. Estado: " << model.get(GRB_IntAttr_Status) << endl;
                    break;
                }

                //cout << "Precios duales ANTES de la actualización:" << endl;
                //cout << "Alpha (viajes):" << endl;
                //for (int i = 0; i < n; i++) {
                //    int nodo_viaje = d + 1 + i;
                //    cout << "  alpha[" << nodo_viaje << "] = " << alpha[nodo_viaje] << endl;
                //}
                //cout << "Beta (depósitos):" << endl;
                //for (int k = 0; k < d; k++) {
                //    int nodo_deposito = 1 + k;
                //    cout << "  beta[" << nodo_deposito << "] = " << beta[nodo_deposito] << endl;
                //}

                // Actualizar alpha (precios duales de viajes)
                map<int, double> alpha_anterior = alpha;
                for (int i = 0; i < n; i++) {
                    int nodo_viaje = d + 1 + i;
                    try {
                        double nuevo_alpha = constr_viajes[nodo_viaje].get(GRB_DoubleAttr_Pi);
                        alpha[nodo_viaje] = nuevo_alpha;
                    }
                    catch (GRBException& e) {
                        cout << "ERROR obteniendo precio dual para viaje " << nodo_viaje << ": " << e.getMessage() << endl;
                    }
                }

                // Actualizar beta (precios duales de depósitos)
                map<int, double> beta_anterior = beta;
                for (int k = 0; k < d; k++) {
                    int nodo_deposito = 1 + k;
                    try {
                        double nuevo_beta = constr_depositos[nodo_deposito].get(GRB_DoubleAttr_Pi);
                        beta[nodo_deposito] = nuevo_beta;
                    }
                    catch (GRBException& e) {
                        cout << "ERROR obteniendo precio dual para depósito " << nodo_deposito << ": " << e.getMessage() << endl;
                    }
                }

                //cout << "\nPrecios duales DESPUÉS de la actualización:" << endl;
                //cout << "Alpha (viajes) - CAMBIOS:" << endl;
                //for (int i = 0; i < n; i++) {
                //    int nodo_viaje = d + 1 + i;
                //    double cambio = alpha[nodo_viaje] - alpha_anterior[nodo_viaje];
                //    cout << "  alpha[" << nodo_viaje << "]: " << alpha_anterior[nodo_viaje]
                //        << " -> " << alpha[nodo_viaje] << " (cambio: " << cambio << ")" << endl;
                //}

                //cout << "Beta (depósitos) - CAMBIOS:" << endl;
                //for (int k = 0; k < d; k++) {
                //    int nodo_deposito = 1 + k;
                //    double cambio = beta[nodo_deposito] - beta_anterior[nodo_deposito];
                //    cout << "  beta[" << nodo_deposito << "]: " << beta_anterior[nodo_deposito]
                //        << " -> " << beta[nodo_deposito] << " (cambio: " << cambio << ")" << endl;
                //}

                // Verificar si hubo cambios significativos
                bool cambios_significativos = false;
                for (int i = 0; i < n; i++) {
                    int nodo_viaje = d + 1 + i;
                    if (abs(alpha[nodo_viaje] - alpha_anterior[nodo_viaje]) > 1e-9) {
                        cambios_significativos = true;
                        break;
                    }
                }
                for (int k = 0; k < d && !cambios_significativos; k++) {
                    int nodo_deposito = 1 + k;
                    if (abs(beta[nodo_deposito] - beta_anterior[nodo_deposito]) > 1e-9) {
                        cambios_significativos = true;
                        break;
                    }
                }

                if (!cambios_significativos) {
                    cout << "\nADVERTENCIA: No se detectaron cambios significativos en los precios duales." << endl;
                    cout << "Esto puede indicar un problema en la adición de columnas." << endl;
                }

                // ============ ACTUALIZAR COSTOS DEL MULTIGRAFO ============
                //cout << " Actualizando costos del multigrafo..." << endl;
                multigrafo_dag.actualizarCostosConDuales(alpha, beta);

                //cout << " Iteración " << iteracion << " completada. Total columnas añadidas: " << count << endl;
            }


            // ============ FINALIZAR MÉTRICAS Y CALCULAR TIEMPOS ============
            auto fin_total = std::chrono::high_resolution_clock::now();
            tiempo_ejecucion_total = std::chrono::duration<double>(fin_total - inicio_total).count();

            num_iteraciones_final = iteracion - 1;
            valor_objetivo_final = model.get(GRB_DoubleAttr_ObjVal);

            // Actualizar valores de las rutas creadas
            for (int i = 0; i < info_rutas_creadas.size(); i++) {
                if (i < columnas_nuevas.size()) {
                    info_rutas_creadas[i].valor = columnas_nuevas[i].get(GRB_DoubleAttr_X);
                }
            }


            //////////////////////////////////////////////////////////////////////
            vector<vector<int>> rutas_activas;

            for (const auto& ruta : info_rutas_creadas) {
                if (ruta.valor > 0.01) {  // Solo rutas activas
                    rutas_activas.push_back(ruta.camino_vector);
                }
            }

         
            const Multigrafo_vsp& multigrafo_base = multigrafo_dag;  

            // Llamar a la función correctamente
            bool es_factible = esSolucionFactible(rutas_activas, multigrafo_dag);
            cout << "  " << endl;
            cout << "1SI ES FACTIBLE, 0 SINO : " << es_factible;
            cout << " " << endl;
            //////////////////////////////////////////////////////////////////////////////////////////////


            // Guardar información de variables en CSV
            //cout << "\n=== GUARDANDO INFORMACION DE VARIABLES EN CSV ===" << endl;
            string nombre_archivo_variables = nombre_instancia + "_variables.csv";
            ofstream csv_variables(nombre_archivo_variables);

            if (!csv_variables.is_open()) {
                cout << "ERROR: No se pudo crear el archivo " << nombre_archivo_variables << endl;
            }
            else {
                // Escribir headers
                csv_variables << "Variable,Valor,Costo_Original,Viajes_Cubiertos,Camino\n";

                // Escribir variables originales de viajes
                for (int i = 0; i < n; i++) {
                    int nodo_viaje = d + 1 + i;
                    double valor_viaje = x_viaje[nodo_viaje].get(GRB_DoubleAttr_X);

                    csv_variables << "Viaje_nodo_" << nodo_viaje << ","
                        << fixed << setprecision(6) << valor_viaje << ","
                        << fixed << setprecision(6) << COSTO_COLUMNA_ARTIFICIAL << ","
                        << "," // Sin viajes cubiertos para variables originales
                        << "\n"; // Sin camino para variables originales
                }

                // Escribir variables originales de depósitos
                for (int k = 0; k < d; k++) {
                    int nodo_deposito = 1 + k;
                    double valor_deposito = x_deposito[nodo_deposito].get(GRB_DoubleAttr_X);

                    csv_variables << "Deposito_nodo_" << nodo_deposito << ","
                        << fixed << setprecision(6) << valor_deposito << ","
                        << fixed << setprecision(6) << COSTO_COLUMNA_ARTIFICIAL << ","
                        << "," // Sin viajes cubiertos para variables originales
                        << "\n"; // Sin camino para variables originales
                }

                // Escribir rutas generadas
                for (const auto& ruta : info_rutas_creadas) {
                    csv_variables << ruta.nombre << ","
                        << fixed << setprecision(6) << ruta.valor << ","
                        << fixed << setprecision(6) << ruta.costo_original << ","
                        << ruta.viajes_cubiertos << ","
                        << ruta.camino_str << "\n";
                }

                csv_variables.close();
                //cout << "Archivo de variables creado: " << nombre_archivo_variables << endl;
                //cout << "Variables originales (viajes): " << n << endl;
                //cout << "Variables originales (depositos): " << d << endl;
                //cout << "Rutas generadas: " << info_rutas_creadas.size() << endl;
            }

            // ============ OBTENER MÉTRICAS DEL MODELO FINAL ============
            int num_variables_final = model.get(GRB_IntAttr_NumVars);
            int num_restricciones_final = model.get(GRB_IntAttr_NumConstrs);

            // ============ INTENTAR RESOLVER COMO IP ============
            if (termino_optimo) {
                // Crear copia del modelo para resolver como IP
                GRBModel modelo_ip = GRBModel(model);

                // Obtener las variables del modelo copiado
                GRBVar* vars_ip = modelo_ip.getVars();
                int num_vars = modelo_ip.get(GRB_IntAttr_NumVars);

                // Modificar las variables artificiales iniciales (ponerlas en 0)
                // Las primeras n variables son de viajes, las siguientes d son de depósitos
                for (int i = 0; i < n + d; i++) {
                    vars_ip[i].set(GRB_DoubleAttr_UB, 0.0);
                }

                // Las variables restantes son las columnas generadas - cambiarlas a binarias
                for (int i = n + d; i < num_vars; i++) {
                    vars_ip[i].set(GRB_CharAttr_VType, GRB_BINARY);
                }

                // Resolver como IP con límite de tiempo
                modelo_ip.set(GRB_DoubleParam_TimeLimit, 60.0); // 1 hora límite////////////////////////////////////////////////////
                modelo_ip.optimize();

                // Verificar estado del modelo IP
                int estado_ip = modelo_ip.get(GRB_IntAttr_Status);
                bool modelo_resuelto = (estado_ip == GRB_OPTIMAL || estado_ip == GRB_TIME_LIMIT);

                // Variables para almacenar resultados
                bool tiene_solucion_ip = false;
                double valor_objetivo_ip = 0.0;

                if (modelo_resuelto) {
                    // Verificar si hay solución disponible
                    try {
                        // Intentar obtener el número de soluciones
                        int num_soluciones = modelo_ip.get(GRB_IntAttr_SolCount);

                        if (num_soluciones > 0) {
                            // Si hay al menos una solución, obtener el valor objetivo
                            valor_objetivo_ip = modelo_ip.get(GRB_DoubleAttr_ObjVal);
                            tiene_solucion_ip = true;
                            cout << "\n=== SOLUCION IP ENCONTRADA ===" << endl;
                            cout << "Valor objetivo IP: " << fixed << setprecision(2) << valor_objetivo_ip << endl;
                            cout << "Estado: " << (estado_ip == GRB_OPTIMAL ? "OPTIMO" : "LIMITE_TIEMPO") << endl;
                        }
                        else {
                            cout << "ADVERTENCIA: Modelo resuelto pero sin soluciones disponibles" << endl;
                            tiene_solucion_ip = false;
                        }
                    }
                    catch (GRBException& e) {
                        cout << "ADVERTENCIA: Error al verificar solución IP (código: " << e.getErrorCode() << ")" << endl;
                        tiene_solucion_ip = false;
                    }
                }
                else {
                    cout << "Modelo IP no se resolvió satisfactoriamente. Estado: " << estado_ip << endl;
                    tiene_solucion_ip = false;
                }

                // Solo procesar si hay solución válida
                if (tiene_solucion_ip) {
                    cout << "\n=== GUARDANDO INFORMACION DE VARIABLES IP EN CSV ===" << endl;
                    string nombre_archivo_variables_ip = nombre_instancia + "_variables_IP.csv";
                    ofstream csv_variables_ip(nombre_archivo_variables_ip);

                    if (!csv_variables_ip.is_open()) {
                        cout << "ERROR: No se pudo crear el archivo " << nombre_archivo_variables_ip << endl;
                    }
                    else {
                        // Escribir headers
                        csv_variables_ip << "Variable,Valor,Costo_Original,Viajes_Cubiertos,Camino\n";

                        try {
                            // Obtener las variables del modelo IP para escribir sus valores
                            GRBVar* vars_ip_para_csv = modelo_ip.getVars();

                            // Escribir variables originales de viajes (deberían estar en 0)
                            for (int i = 0; i < n; i++) {
                                int nodo_viaje = d + 1 + i;
                                double valor_viaje = vars_ip_para_csv[i].get(GRB_DoubleAttr_X);

                                csv_variables_ip << "Viaje_" << nodo_viaje << ","
                                    << fixed << setprecision(6) << valor_viaje << ","
                                    << fixed << setprecision(6) << COSTO_COLUMNA_ARTIFICIAL << ","
                                    << "," // Sin viajes cubiertos para variables originales
                                    << "\n"; // Sin camino para variables originales
                            }

                            // Escribir variables originales de depósitos (deberían estar en 0)
                            for (int k = 0; k < d; k++) {
                                int nodo_deposito = 1 + k;
                                double valor_deposito = vars_ip_para_csv[n + k].get(GRB_DoubleAttr_X);

                                csv_variables_ip << "Deposito_" << nodo_deposito << ","
                                    << fixed << setprecision(6) << valor_deposito << ","
                                    << fixed << setprecision(6) << COSTO_COLUMNA_ARTIFICIAL << ","
                                    << "," // Sin viajes cubiertos para variables originales
                                    << "\n"; // Sin camino para variables originales
                            }

                            // Escribir rutas generadas (ahora con valores binarios)
                            for (int i = 0; i < info_rutas_creadas.size(); i++) {
                                double valor_ip = vars_ip_para_csv[n + d + i].get(GRB_DoubleAttr_X);
                                csv_variables_ip << info_rutas_creadas[i].nombre << ","
                                    << fixed << setprecision(6) << valor_ip << ","
                                    << fixed << setprecision(6) << info_rutas_creadas[i].costo_original << ","
                                    << info_rutas_creadas[i].viajes_cubiertos << ","
                                    << info_rutas_creadas[i].camino_str << "\n";
                            }

                            csv_variables_ip.close();
                            cout << "Archivo de variables IP creado: " << nombre_archivo_variables_ip << endl;
                        }
                        catch (GRBException& e) {
                            cout << "ERROR al escribir variables IP: " << e.getMessage() << " (código: " << e.getErrorCode() << ")" << endl;
                            csv_variables_ip.close();
                        }
                    }

                    // Asignar valores finales
                    valor_IP_final = valor_objetivo_ip;
                    gap_final = (valor_IP_final - valor_objetivo_final) / valor_IP_final;
                }
                else {
                    // Si no hay solución IP válida, asignar valores por defecto
                    cout << "No se pudo obtener solución IP válida" << endl;
                    valor_IP_final = 0.0;
                    gap_final = 0.0;
                }

                // Limpiar memoria
                delete[] vars_ip;
            }
            else {
                // Si no terminó óptimo, asignar valores por defecto
                valor_IP_final = 0.0;
                gap_final = 0.0;
            }
            ///////////////////////////////////////////////////////////////
            // ============ GUARDAR PROGRESO DE ITERACIONES EN EXCEL ============
            //cout << "\n=== GUARDANDO PROGRESO DE ITERACIONES EN EXCEL ===" << endl;
            if (!historial_progreso.empty()) {
                // Crear nombre de archivo único
                std::string nombre_archivo_progreso = nombre_instancia + "_progreso.csv";
                guardarProgresoIteraciones(historial_progreso, nombre_archivo_progreso);

                //cout << "Archivo de progreso creado: " << nombre_archivo_progreso << endl;
                //cout << "Total de iteraciones registradas: " << historial_progreso.size() << endl;
            }
            else {
                cout << "No hay iteraciones para guardar en el archivo de progreso." << endl;
            }
            ////////////////////////////////////////////////////////////////////

            // ============ GUARDAR MÉTRICAS EN CSV ============
            //cout << "\n=== GUARDANDO METRICAS EN CSV ===\n";

            // Verificar si el archivo ya existe para decidir si escribir headers
            bool archivo_existe = false;
            ifstream test_file(nombre_instancia + "_generCol.csv");
            if (test_file.good()) {
                archivo_existe = true;
            }
            test_file.close();

            // Abrir archivo en modo append
            ofstream csv_file(nombre_instancia + "_generCol.csv", ios::app);

            if (!csv_file.is_open()) {
                cout << "ERROR: No se pudo abrir el archivo CSV" << endl;
            }
            else {
                // Escribir headers si el archivo es nuevo
                if (!archivo_existe) {
                    csv_file << "Nombre,NumVars,NumRestr,NumColumnasAnadidas,NumIteraciones,"
                        << "ValorObjetivoRMLP,TiempoEjecucion,TiempoLP,TiempoDAG,Termino,"
                        << "ValorIP,Gap\n";
                }

                // Escribir datos
                csv_file << nombre_instancia << ","
                    << num_variables_final << ","
                    << num_restricciones_final << ","
                    << num_columnas_anadidas << ","
                    << num_iteraciones_final << ","
                    << fixed << setprecision(6) << valor_objetivo_final << ","
                    << fixed << setprecision(2) << tiempo_ejecucion_total << ","
                    << fixed << setprecision(2) << tiempo_total_LP << ","
                    << fixed << setprecision(2) << tiempo_total_DAG << ","
                    << (termino_optimo ? "SI" : "NO") << ","
                    << fixed << setprecision(6) << valor_IP_final << ","
                    << fixed << setprecision(4) << gap_final << "\n";

                csv_file.close();
                //cout << "Metricas guardadas en resultados_GC.csv" << endl;
            }

            // ============ MOSTRAR RESUMEN FINAL ============
            cout << "\n=== RESUMEN FINAL DE MÉTRICAS ===" << endl;
            cout << "Nombre instancia: " << nombre_instancia << endl;
            cout << "Numero de variables finales: " << num_variables_final << endl;
            cout << "Numero de restricciones finales: " << num_restricciones_final << endl;
            cout << "Columnas agregadas: " << num_columnas_anadidas << endl;
            cout << "Iteraciones: " << num_iteraciones_final << endl;
            cout << "Valor objetivo RMLP: " << valor_objetivo_final << endl;
            cout << "Tiempo total: " << tiempo_ejecucion_total << " segundos" << endl;
            cout << "Tiempo LP acumulado: " << tiempo_total_LP << " segundos" << endl;
            cout << "Tiempo DAG acumulado: " << tiempo_total_DAG << " segundos" << endl;
            cout << "Termino optimo: " << (termino_optimo ? "SI" : "NO") << endl;

            if (valor_IP_final > 0) {
                cout << "Valor IP: " << valor_IP_final << endl;
                cout << "Gap: " << (gap_final * 100) << "%" << endl;
            }

            // ============ RESULTADOS FINALES ============
            cout << "\n=== RESULTADOS FINALES DE GENERACION DE COLUMNAS ===" << endl;
            cout << "Total de iteraciones: " << iteracion - 1 << endl;
            cout << "Total de columnas agregadas: " << count << endl;
            cout << "Costo final: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

            // Mostrar valores de las nuevas columnas
            //cout << "\nAnalizando cobertura de viajes:" << endl;
            //for (int i = 0; i < n; i++) {
            //    int nodo_viaje = d + 1 + i;
            //    double valor_viaje = x_viaje[nodo_viaje].get(GRB_DoubleAttr_X);
            //    cout << "Viaje nodo " << nodo_viaje << " (viaje " << (i + 1) << "): valor = " << valor_viaje;
            //    if (valor_viaje < 0.01) {
            //        cout << "   NO CUBIERTO";
            //    }
            //    cout << endl;
            //}

            //if (count > 0) {
            //    cout << "\nValores de las nuevas columnas (rutas):" << endl;
            //    cout << "Columna\t\tValor" << endl;
            //    cout << "-------\t\t-----" << endl;
            //    for (int i = 0; i < columnas_nuevas.size(); i++) {
            //        double valor = columnas_nuevas[i].get(GRB_DoubleAttr_X);
            //        cout << "ruta_" << (i + 1) << "\t\t" << valor;
            //        if (valor > 0.001) cout << " <- ACTIVA";
            //        cout << endl;
            //    }
            //}
        }
        else {
            cout << "No se encontró una solución óptima inicial." << endl;
            cout << "Estado del modelo: " << model.get(GRB_IntAttr_Status) << endl;
        }
        // ============ RESOLVER MODELO DE ARCOS ============
        resolverModeloArcos(multigrafo_dag, "QMCG.csv");
        resolverModeloAsignacion(multigrafo_dag, "QMCG.csv");
        //resolverCaminoCorto(multigrafo_dag, "SUR.csv");
    }
    catch (GRBException& e) {
        cerr << "Error de Gurobi: " << e.getMessage() << endl;
        cerr << "Código de error: " << e.getErrorCode() << endl;
    }
    catch (const exception& e) {
        cerr << "Se produjo una excepcion: " << e.what() << endl;
    }
    catch (...) {
        cerr << "Se produjo una excepción desconocida." << endl;
    }

    return 0;
}