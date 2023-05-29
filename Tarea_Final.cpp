#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <random>
#include <chrono>
#include <iomanip>
using namespace std;

const int NUMERO_MAXIMO = 50;
const int NUM_COLUMNAS = 4;
const int NUM_OPERACIONES = 30;

mutex mtx;
condition_variable cv;
vector<vector<int>> colas(NUM_COLUMNAS);
vector<vector<int>> cajas(NUM_COLUMNAS);
vector<vector<int>> pilas(NUM_COLUMNAS);
int suma = 0;
bool terminado = false;

int obtenerNumeroAleatorio() {
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dis(1, NUMERO_MAXIMO);
    return dis(gen);
}

void hiloCliente(int id) {
    this_thread::sleep_for(std::chrono::seconds(1));
    
    for (int i = 0; i < NUM_OPERACIONES; i++) {
        std::unique_lock<mutex> lock(mtx);
        cv.wait(lock, [id] { return colas[id].size() < NUM_OPERACIONES / NUM_COLUMNAS; });

        int numero = obtenerNumeroAleatorio();
        colas[id].push_back(numero);
        cout << "El Cliente; " << id <<endl;
		cout << "Agrego El Numero: " << numero << " A la COLA " << id + 1 << "." << std::endl;

        lock.unlock();
        cv.notify_all();

        this_thread::sleep_for(chrono::seconds(1));
    }

    unique_lock<mutex> lock(mtx);
    terminado = true;
    cv.notify_all();
}

void hiloCajero(int id) {
    while (true) {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [id] { return terminado || !colas[id].empty(); });

        if (terminado && colas[id].empty()) {
            break;
        }

        if (!colas[id].empty()) {
            int numero = colas[id].front();
            colas[id].erase(colas[id].begin());
            cout << "El Cajero; " << id + 1 <<endl;
			cout << " Atendio Al Numero: " << numero << " De La COLA " << id + 1 << "." << std::endl;

            lock.unlock();
            cv.notify_all();

            this_thread::sleep_for(chrono::seconds(2));

            lock.lock();
            cajas[id].push_back(numero);
            pilas[id].push_back(numero);
            suma += numero;
            cout << "El Cajero; " << id + 1 << endl;
			cout << "Coloco El Numero: " << numero << " En La PILA " << id + 1 << "." <<endl;
        }

        lock.unlock();
    }
}

int main() {
    vector<thread> clientes(NUM_COLUMNAS);
    vector<thread> cajeros(NUM_COLUMNAS);

    for (int i = 0; i < NUM_COLUMNAS; i++) {
        clientes[i] = thread(hiloCliente, i);
        cajeros[i] = thread(hiloCajero, i);
    }

    for (int i = 0; i < NUM_COLUMNAS; i++) {
        clientes[i].join();
        cajeros[i].join();
    }

    cout << "\nRESUMEN DE ATENCION:\n";
    cout << "+--------+--------+--------+--------+\n";
    cout << "| COLA 1 | COLA 2 | COLA 3 | COLA 4 |\n";
    cout << "+--------+--------+--------+--------+\n";
	cout << "_____________________________________\n";
    cout << "+--------+--------+--------+--------+\n";
    cout << "| CAJA 1 | CAJA 2 | CAJA 3 | CAJA 4 |\n";
    cout << "+--------+--------+--------+--------+\n";
    for (int i = 0; i < NUM_OPERACIONES / NUM_COLUMNAS; i++) {
       	cout << "|";
        for (int j = 0; j < NUM_COLUMNAS; j++) {
            if (i < cajas[j].size()) {
                cout << " " << setw(5) << cajas[j][i] << "  |";
            } else {
                cout << "        |";
            }
        }
        cout << "\n";
    }
    cout << "+--------+--------+--------+--------+\n";
    cout << "| PILA 1 | PILA 2 | PILA 3 | PILA 4 |\n";
    cout << "+--------+--------+--------+--------+\n";
    for (int i = pilas[0].size() - 1; i >= 0; i--) {
        cout << "|";
        for (int j = 0; j < NUM_COLUMNAS; j++) {
            if (i < pilas[j].size()) {
                cout << " " << setw(5) << pilas[j][i] << "  |";
            } else {
                cout << "        |";
            }
        }
        cout << "\n";
    }
    cout << "+--------+--------+--------+--------+\n";

    cout << "\nTotal De Numeros Atendidos: " << suma << endl;

    return 0;
}
