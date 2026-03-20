//
// Created by juanp on 16/03/2026.
//

#include "scheduler.h"
#include <stdexcept>
#include <exception>
#include <sstream>
#include <iostream>

//==Constructor==
SCHEDULER::SCHEDULER(std::vector<std::string> &arguments) {
    /*
     Se asume que todos los argumentos vienen ya separados como vienen en la llamada de ejecución
     y aquellos algoritmos de múltiples colas ya tienen cada algoritmo como strings
     Ej: SchedulingSim.exe <inputFile> <schedulingAlgorithm> [params]
     arguments[0] = <inputFile>
     arguments[1] = <schedulingAlgorithm>
     arguments[2] = [param0]
     arguments[3] = [param1]
     ...
     */
     // Carga procesos desde archivo de entrada
    dataTable.extractDataFromFile(arguments[0]);

    // Reinicia estado interno del scheduler
    MLQ.clear();
    numQ = 0;
    currentTime = 0;

    // Normaliza nombre del algoritmo principal
    arguments[1] = toUpper(arguments[1]);

    // Configura esquema MLQ con algoritmos explícitos por cola
    if (arguments[1] == "MLQ") {
        multiQueueType = 1;

        // Número de colas a configurar
        int n = std::stoi(arguments[2]);

        std::string algName;
        std::string algParam;
        std::istringstream algBuffer;
        int i = 0;

        // Construye cada cola leyendo algoritmo y parámetros
        for (i = 0; i < n && i < arguments.size(); i++) {
            algBuffer.clear();
            algBuffer.str(arguments[3 + i]);
            algBuffer >> algName;
            algBuffer >> algParam;

            // Inserta cola configurada en la estructura
            emplaceAlg(algName, algParam);
        }

        // Verifica consistencia entre colas esperadas y definidas
        if (i != n) {
            throw std::invalid_argument("The provided queue algorithm has missing algorithms\nN = " + std::to_string(n) + "; Provided = " + std::to_string(i));
        }
    }

    // Configura esquema MLFQ con RR y última cola personalizada
    else if (arguments[1] == "MLFQ") {
        multiQueueType = 2;

        // Número total de colas
        int n = std::stoi(arguments[2]);

        std::string algName = "RR";
        std::string algParam;
        int q;
        std::istringstream algBuffer;
        int i = 0;

        // Crea primeras colas como Round Robin con distintos quantums
        for (i = 0; i < n - 1 && i < arguments.size(); i++) {
            emplaceAlg(algName, arguments[3 + i]);
        }

        // Configura última cola con algoritmo arbitrario
        if (i == (n - 1)) {
            algBuffer.clear();
            algBuffer.str(arguments[3 + i]);
            algBuffer >> algName;
            algBuffer >> algParam;

            emplaceAlg(algName, algParam);
        }

        // Valida que todas las colas requeridas fueron definidas
        else {
            throw std::invalid_argument("The provided queue algorithm has missing algorithms\nN = " + std::to_string(n) + "; Provided = " + std::to_string(i));
        }
    }

    // Configura algoritmo único sin múltiples colas
    else {
        multiQueueType = 0;

        // Inserta algoritmo con o sin parámetro adicional
        if (arguments.size() > 2) {
            emplaceAlg(arguments[1], arguments[2]);
        }
        else {
            emplaceAlg(arguments[1], arguments[1]);
        }
    }

    // Asigna procesos a colas iniciales según esquema seleccionado
    assignProcesses();

    // Registra tiempos de llegada para eventos de simulación
    setRelevantTimes();
}

//==Métodos privados==
std::string SCHEDULER::toUpper(const std::string& input) {
    std::string result = input;
    for (char& c : result) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return result;
}

void SCHEDULER::emplaceAlg(std::string &alg, std::string &param) {
    alg = toUpper(alg);
    if (alg == "FCFS") {
        MLQ.emplace_back(false, false, 0, 0);
    }
    else if (alg == "SJF") {
        MLQ.emplace_back(false, false, 1, 0);
    }
    else if (alg == "PSJF") {
        MLQ.emplace_back(true, false, 2, 0);
    }
    else if (alg == "PRIORITY") {
        param = toUpper(param);
        if (param == "ASC") {
            MLQ.emplace_back(false, true, 3, 0);
        }
        else {
            MLQ.emplace_back(false, false, 3, 0);
        }
    }
    else if (alg == "P-PRIORITY") {
        param = toUpper(param);
        if (param == "ASC") {
            MLQ.emplace_back(true, true, 4, 0);
        }
        else {
            MLQ.emplace_back(true, false, 4, 0);
        }
    }
    else if (alg == "RR") {
        MLQ.emplace_back(true, false, 5, std::stoi(param));
    }
    else {
        throw std::invalid_argument("The algorithm '" + alg + "' is not supported by the simulator.");
    }
    numQ++;
}

void SCHEDULER::assignProcesses() {
    if (multiQueueType == 1) {
        for (int i = 0; i < dataTable.getSize(); i++) {
            MLQ[dataTable.getQueue()[i] - 1].addProcess(i, dataTable.getArrivalTime()[i]);
        }
    }
    else {
        for (int i = 0; i < dataTable.getSize(); i++) {
            MLQ[0].addProcess(i, dataTable.getArrivalTime()[i]);
        }
    }
}

void SCHEDULER::setRelevantTimes() {
    for (int i = 0; i < dataTable.getSize(); i++) {
        relevantTimes.emplace(dataTable.getArrivalTime()[i]);
    }
}

void SCHEDULER::simulation() {

    // Inicializa variables de control de simulación
    int numCola;
    bool proccessFound = true;
    int p;

    // Reinicia contador de procesos completados
    numPCompleted = 0;

    // Ejecuta hasta completar todos los procesos
    while (numPCompleted < dataTable.getSize()) {

        // Indica si se ejecutó algún proceso en esta iteración
        proccessFound = false;
        numCola = 0;

        // Recorre colas desde mayor a menor prioridad
        while (numCola < numQ && !proccessFound) {

            // Verifica si la cola tiene procesos disponibles
            if (!MLQ[numCola].isEmpty()) {

                // Ejecuta un proceso de la cola actual
                p = executeProcess(numCola);

                // Marca si se logró ejecutar un proceso válido
                if (p != -1) {
                    proccessFound = true;
                }
            }
            numCola++;
        }

        // Si no se ejecutó nada, avanza al siguiente evento relevante
        if (!proccessFound) {
            if (!relevantTimes.empty()) {

                // Salta el tiempo actual al próximo arribo de proceso
                currentTime = *relevantTimes.begin();

                // Elimina evento ya considerado
                relevantTimes.erase(relevantTimes.begin());
            }
        }
    }

    // Calcula Turnaround Time para cada proceso finalizado
    for (int i = 0; i < dataTable.getSize(); i++) {
        dataTable.getTAT()[i] = dataTable.getCompletionTime()[i] - dataTable.getArrivalTime()[i];
    }
}

int SCHEDULER::executeProcess(int numCola) {

    int p;
    int passedTime;
    int startTime;
    int endTime;

    // Elimina eventos ya ocurridos hasta el tiempo actual
    while (!relevantTimes.empty() && *relevantTimes.begin() <= currentTime) {
        relevantTimes.erase(relevantTimes.begin());
    }

    // Selecciona proceso según política de la cola
    p = determineProcess(numCola);

    // No hay proceso ejecutable en esta cola
    if (p == -1) {
        ;
    }

    // Ejecución completa para algoritmos no expropiativos
    else if (!MLQ[numCola].isPreemp()) {

        // Ejecuta todo el tiempo restante del proceso
        passedTime = dataTable.getRemainingTime()[p];
        dataTable.getRemainingTime()[p] = 0;

        // Avanza tiempo global y registra finalización
        currentTime += passedTime;
        dataTable.getCompletionTime()[p] = currentTime;
        numPCompleted++;

        // Intervalo de ejecución usado para actualizar esperas
        startTime = currentTime - passedTime;
        endTime = currentTime;

        // Acumula waiting time de procesos listos durante ejecución
        for (int i = 0; i < dataTable.getSize(); i++) {

            // Proceso ya estaba esperando al iniciar ejecución
            if (dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }

            // Proceso llegó durante ejecución, espera parcial
            else if (dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }

        // Elimina eventos consumidos dentro del intervalo ejecutado
        for (std::set<int>::iterator itRelevant = relevantTimes.begin(); itRelevant != relevantTimes.end();) {
            if (*itRelevant <= currentTime) {
                itRelevant = relevantTimes.erase(itRelevant);
            } else {
                break;
            }
        }

        // Remueve proceso completado de la cola
        MLQ[numCola].removeProcess(p);
    }

    // Manejo de algoritmos expropiativos (PSJF, prioridad expropiativa)
    else if (MLQ[numCola].get_algID() == 2 || MLQ[numCola].get_algID() == 4) {

        // Ejecuta completamente si no hay interrupciones próximas
        if (relevantTimes.empty() || currentTime + dataTable.getRemainingTime()[p] < *(relevantTimes.begin()) ) {

            passedTime = dataTable.getRemainingTime()[p];
            MLQ[numCola].removeProcess(p);

            // Registra finalización completa
            dataTable.getCompletionTime()[p] = currentTime + passedTime;
            numPCompleted++;

            // Consume evento si coincide con finalización
            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
        }

        // Ejecuta parcialmente hasta próximo evento relevante
        else {

            // Limita ejecución hasta siguiente llegada
            passedTime = *(relevantTimes.begin()) - currentTime;

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }

            // Aplica castigo en MLFQ moviendo a cola inferior
            if (multiQueueType == 2 && numCola != numQ - 1) {
                MLQ[numCola].removeProcess(p);
                MLQ[numCola + 1].addProcess(p, currentTime + passedTime);
            }
        }

        // Reduce tiempo restante según ejecución parcial
        dataTable.getRemainingTime()[p] -= passedTime;

        // Avanza tiempo global
        currentTime += passedTime;

        startTime = currentTime - passedTime;
        endTime = currentTime;

        // Acumula waiting time para procesos distintos al ejecutado
        for (int i = 0; i < dataTable.getSize(); i++) {

            if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }
            else if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }
    }

    // Manejo de Round Robin
    else if (MLQ[numCola].get_algID() == 5) {
        // Limita ejecución por quantum y próximos eventos
        passedTime = std::min(dataTable.getRemainingTime()[p], MLQ[numCola].get_quantum());
        /*
        if (!relevantTimes.empty()) {
            passedTime = std::min(passedTime, *(relevantTimes.begin()) - currentTime);
        }
        */

        // Proceso finaliza dentro del quantum
        if (dataTable.getRemainingTime()[p] <= passedTime) {

            MLQ[numCola].removeProcess(p);

            // Registra tiempo de finalización
            dataTable.getCompletionTime()[p] = currentTime + passedTime;
            numPCompleted++;

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
        }

        // Quantum agotado, se reubica proceso
        else if (passedTime == MLQ[numCola].get_quantum()) {

            // Aplica castigo en MLFQ o reencola en misma cola
            if (multiQueueType == 2 && numCola != numQ - 1) {
                MLQ[numCola].removeProcess(p);
                MLQ[numCola + 1].addProcess(p, currentTime + passedTime);
            }
            else {
                MLQ[numCola].removeProcess(p);
                MLQ[numCola].addProcess(p, currentTime + passedTime);
            }

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
        }

        /*
        // Interrupción antes de completar quantum
        else {

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }

            // Reencola proceso en misma cola
            //MLQ[numCola].removeProcess(p);
            //MLQ[numCola].addProcess(p, currentTime + passedTime);
        }
        */

        // Actualiza tiempo restante tras ejecución parcial
        dataTable.getRemainingTime()[p] -= passedTime;

        // Avanza tiempo global
        currentTime += passedTime;

        startTime = currentTime - passedTime;
        endTime = currentTime;

        // Acumula waiting time para procesos en espera
        for (int i = 0; i < dataTable.getSize(); i++) {

            if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }
            else if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }
    }

    // Registra response time en la primera ejecución del proceso
    if (p != -1 && dataTable.getResponseTime()[p] == -1) {
        dataTable.getResponseTime()[p] = currentTime - passedTime;
    }

    return p;
}

int SCHEDULER::determineProcess(int numCola) {

    // Obtiene identificador del algoritmo de la cola
    int id = MLQ[numCola].get_algID();

    int process;

    // Variables auxiliares para seleccionar mejor candidato
    std::string firstTag = "!";
    int firstTime = -1;
    int firstProcess = -1;

    // Selección FCFS por menor tiempo de llegada
    if (id == 0) {

        for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {

            process = *it;

            // Filtra procesos que ya han llegado
            if (dataTable.getArrivalTime()[process] <= currentTime &&

                // Selecciona menor arrival time, desempata por etiqueta
                ( firstTime == -1 ||
                  dataTable.getArrivalTime()[process] < firstTime ||
                  (dataTable.getArrivalTime()[process] == firstTime &&
                   (firstTag == "!" || dataTable.getProcessTag()[process] < firstTag)) )) {

                firstTime = dataTable.getArrivalTime()[process];
                firstTag = dataTable.getProcessTag()[process];
                firstProcess = process;
            }
        }
    }

    // Selección SJF / PSJF por menor tiempo restante
    else if (id == 1 || id == 2) {

        // Primera iteración SJF usa arrival time como criterio inicial
        if (id == 1 && numCola == 0 && MLQ[numCola].isFirstTimeSJF()) {

            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {

                process = *it;

                // Selección inicial por orden de llegada
                if (dataTable.getArrivalTime()[process] <= currentTime &&
                    ( firstTime == -1 ||
                      dataTable.getArrivalTime()[process] < firstTime ||
                      (dataTable.getArrivalTime()[process] == firstTime &&
                       dataTable.getProcessTag()[process] < firstTag) )) {

                    firstTime = dataTable.getArrivalTime()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }

            // Desactiva comportamiento especial de primera iteración
            MLQ[numCola].set_firstTimeSJF(false);
        }

        else {

            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {

                process = *it;

                // Selecciona menor tiempo restante, desempata por etiqueta
                if (dataTable.getArrivalTime()[process] <= currentTime &&
                    ( firstTime == -1 ||
                      dataTable.getRemainingTime()[process] < firstTime ||
                      (dataTable.getRemainingTime()[process] == firstTime &&
                       dataTable.getProcessTag()[process] < firstTag) )) {

                    firstTime = dataTable.getRemainingTime()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }
    }

    // Selección por prioridad (ascendente o descendente)
    else if (id == 3 || id == 4) {

        // Prioridad ascendente: menor valor es mayor prioridad
        if (MLQ[numCola].isAscending()) {

            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {

                process = *it;

                // Selecciona menor prioridad, desempata por etiqueta
                if (dataTable.getArrivalTime()[process] <= currentTime &&
                    ( firstTime == -1 ||
                      dataTable.getPriority()[process] < firstTime ||
                      (dataTable.getPriority()[process] == firstTime &&
                       dataTable.getProcessTag()[process] < firstTag) )) {

                    firstTime = dataTable.getPriority()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }

        // Prioridad descendente: mayor valor es mayor prioridad
        else {

            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {

                process = *it;

                // Selecciona mayor prioridad, desempata por etiqueta
                if (dataTable.getArrivalTime()[process] <= currentTime &&
                    ( firstTime == -1 ||
                      dataTable.getPriority()[process] > firstTime ||
                      (dataTable.getPriority()[process] == firstTime &&
                       dataTable.getProcessTag()[process] < firstTag) )) {

                    firstTime = dataTable.getPriority()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }
    }

    // Selección Round Robin por orden de llegada en cola
    else if (id == 5) {

        for (std::set<int>::iterator itRR = MLQ[numCola].getAssociatedProcesses().begin(); itRR != MLQ[numCola].getAssociatedProcesses().end(); itRR++) {

            process = *itRR;

            // Selecciona proceso con menor tiempo de entrada a cola
            if (MLQ[numCola].getArrivalT()[process] <= currentTime &&
                ( firstTime == -1 ||
                  MLQ[numCola].getArrivalT()[process] < firstTime ||
                  (MLQ[numCola].getArrivalT()[process] == firstTime &&
                   dataTable.getProcessTag()[process] < firstTag) )) {

                firstTime = MLQ[numCola].getArrivalT()[process];
                firstTag = dataTable.getProcessTag()[process];
                firstProcess = process;
            }
        }
    }

    // Retorna proceso seleccionado o -1 si ninguno disponible
    return firstProcess;
}

//==Getters==

std::vector<QUEUE>& SCHEDULER::getMLQ() {
    return MLQ;
}

TABLE& SCHEDULER::getTable() {
    return dataTable;
}