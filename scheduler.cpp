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
    dataTable.extractDataFromFile(arguments[0]);
    MLQ.clear();
    numQ = 0;
    currentTime = 0;
    arguments[1] = toUpper(arguments[1]);
    if (arguments[1] == "MLQ") {
        multiQueueType = 1;
        int n = std::stoi(arguments[2]);
        std::string algName;
        std::string algParam;
        std::istringstream algBuffer;
        int i = 0;
        for (i = 0; i < n && i < arguments.size(); i++) {
            // schAlg_i param_i
            algBuffer.clear();
            algBuffer.str(arguments[3 + i]);
            algBuffer >> algName;
            algBuffer >> algParam;
            emplaceAlg(algName, algParam);
        }

        if (i != n) {
            throw std::invalid_argument("The provided queue algorithm has missing algorithms\nN = " + std::to_string(n) + "; Provided = " + std::to_string(i));
        }
    }
    else if (arguments[1] == "MLFQ") {
        multiQueueType = 2;
        int n = std::stoi(arguments[2]);
        std::string algName = "RR";
        std::string algParam;
        int q;
        std::istringstream algBuffer;
        int i = 0;
        for (i = 0; i < n - 1 && i < arguments.size(); i++) {
            // Q_i
            emplaceAlg(algName,arguments[3 + i]);
        }

        if (i == (n - 1)) {
            algBuffer.clear();
            algBuffer.str(arguments[3 + i]);
            algBuffer >> algName;
            algBuffer >> algParam;
            emplaceAlg(algName, algParam);
        }
        else {
            throw std::invalid_argument("The provided queue algorithm has missing algorithms\nN = " + std::to_string(n) + "; Provided = " + std::to_string(i));
        }
    }
    else {
        multiQueueType = 0;
        if (arguments.size() > 2) {
            emplaceAlg(arguments[1], arguments[2]);
        }
        else {
            emplaceAlg(arguments[1], arguments[1]);
        }
    }

    //Después de crear cada cola asignamos cada algoritmo a su cola correspondiente
    assignProcesses();

    //Ahora tomamos un registro de todos los tiempos donde llega un nuevo procesos a CPU, será útil para la simulación
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
    /*
    Vamos a ir cola por cola, empezando por las colas de mayor prioridad (que son las primeras)
    hasta llegar a una cla que aún tenga procesos sin resolver. Sabemos que hemos terminado
    en tanto pasemos por todas las colas y todas estén sin procesos por ejecutar
    */
    int numCola;
    bool proccessFound = true;
    int p;
    numPCompleted = 0;
    while (numPCompleted < dataTable.getSize()) {
        proccessFound = false;
        numCola = 0;
        while (numCola < numQ && !proccessFound) {
            if (!MLQ[numCola].isEmpty()) {
                p = executeProcess(numCola);

                if (p != -1) {
                    proccessFound = true;
                }
            }
            numCola++;
        }

        if (!proccessFound) {
            if (!relevantTimes.empty()) {
                currentTime = *relevantTimes.begin();
                relevantTimes.erase(relevantTimes.begin());
            }
        }
    }

    for (int i = 0; i < dataTable.getSize(); i++) {
        dataTable.getTAT()[i] = dataTable.getCompletionTime()[i] - dataTable.getArrivalTime()[i];
    }
}

int SCHEDULER::executeProcess(int numCola) {
    int p;
    int passedTime;
    int startTime;
    int endTime;
    while (!relevantTimes.empty() && *relevantTimes.begin() <= currentTime) {
        relevantTimes.erase(relevantTimes.begin());
    }
    p = determineProcess(numCola);
    if (p == -1) {
        //No se encontró un proceso válido para este tiempo, entonces no hacemos nada para esta cola
        ;
    }
    else if (!MLQ[numCola].isPreemp()) {
        passedTime = dataTable.getRemainingTime()[p];
        dataTable.getRemainingTime()[p] = 0;
        currentTime += passedTime;
        dataTable.getCompletionTime()[p] = currentTime;
        numPCompleted++;

        startTime = currentTime - passedTime;
        endTime = currentTime;
        for (int i = 0; i < dataTable.getSize(); i++) {
            if (dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }
            else if (dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }

        for (std::set<int>::iterator itRelevant = relevantTimes.begin(); itRelevant != relevantTimes.end();) {
            if (*itRelevant <= currentTime) {
                itRelevant = relevantTimes.erase(itRelevant); // devuelve el siguiente válido
            } else {
                break; // como está ordenado, ya puedo parar
            }
        }

        MLQ[numCola].removeProcess(p);
    }
    else if (MLQ[numCola].get_algID() == 2 || MLQ[numCola].get_algID() == 4) {
        //PSJF, P-PRIORITY
        if (relevantTimes.empty() || currentTime + dataTable.getRemainingTime()[p] < *(relevantTimes.begin()) ) {
            passedTime = dataTable.getRemainingTime()[p];
            MLQ[numCola].removeProcess(p);
            dataTable.getCompletionTime()[p] = currentTime + passedTime;
            numPCompleted++;

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
        }
        else {
            passedTime = *(relevantTimes.begin()) - currentTime;
            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
            if (multiQueueType == 2 && numCola != numQ - 1) {
                MLQ[numCola].removeProcess(p);
                MLQ[numCola + 1].addProcess(p, currentTime + passedTime);
            }
        }
        dataTable.getRemainingTime()[p] -= passedTime;
        currentTime += passedTime;

        startTime = currentTime - passedTime;
        endTime = currentTime;
        for (int i = 0; i < dataTable.getSize(); i++) {
            if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }
            else if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }
    }
    else if (MLQ[numCola].get_algID() == 5) {
        //RR
        passedTime = std::min(dataTable.getRemainingTime()[p],MLQ[numCola].get_quantum());
        if (!relevantTimes.empty()) {
            passedTime = std::min(passedTime, *(relevantTimes.begin()) - currentTime);
        }

        if (dataTable.getRemainingTime()[p] <= passedTime) {
            MLQ[numCola].removeProcess(p);
            dataTable.getCompletionTime()[p] = currentTime + passedTime;
            numPCompleted++;

            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
        }
        else if (passedTime == MLQ[numCola].get_quantum()) {
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
        else {
            if (!relevantTimes.empty() && currentTime + passedTime == *relevantTimes.begin()) {
                relevantTimes.erase(relevantTimes.begin());
            }
            MLQ[numCola].removeProcess(p);
            MLQ[numCola].addProcess(p, currentTime + passedTime);
        }

        dataTable.getRemainingTime()[p] -= passedTime;
        currentTime += passedTime;

        startTime = currentTime - passedTime;
        endTime = currentTime;
        for (int i = 0; i < dataTable.getSize(); i++) {
            if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] <= startTime) {
                dataTable.getWaitingTime()[i] += passedTime;
            }
            else if (i != p && dataTable.getRemainingTime()[i] > 0 && dataTable.getArrivalTime()[i] < endTime) {
                dataTable.getWaitingTime()[i] += (endTime - dataTable.getArrivalTime()[i]);
            }
        }
    }

    if (p != -1 && dataTable.getResponseTime()[p] == -1) {
        dataTable.getResponseTime()[p] = currentTime - passedTime;
    }
    return p;
}

int SCHEDULER::determineProcess(int numCola) {
    int id = MLQ[numCola].get_algID();
    int process;
    std::string firstTag = "!";
    int firstTime = -1;
    int firstProcess = -1;
    if (id == 0) {
        //FCFS
        for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {
            process = *it;
            if (dataTable.getArrivalTime()[process] <= currentTime && ( firstTime == -1 || (dataTable.getArrivalTime()[process] < firstTime || ( dataTable.getArrivalTime()[process] == firstTime && ( firstTag == "!" || dataTable.getProcessTag()[process] < firstTag ) ) ) ) ) {
                firstTime = dataTable.getArrivalTime()[process];
                firstTag = dataTable.getProcessTag()[process];
                firstProcess = process;
            }
        }
    }
    else if (id == 1 || id == 2) {
        // SJF / PSJF (Unifico esto porque algo en SJF podría venir semi-trabajado de otra cola)
        if (id == 1 && MLQ[numCola].isFirstTimeSJF()) {
            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {
                process = *it;
                if (dataTable.getArrivalTime()[process] <= currentTime && ( firstTime == -1 || ( dataTable.getArrivalTime()[process] < firstTime || ( dataTable.getArrivalTime()[process] == firstTime && dataTable.getProcessTag()[process] < firstTag ) ) ) ) {
                    firstTime = dataTable.getArrivalTime()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
            MLQ[numCola].set_firstTimeSJF(false);
        }
        else {
            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {
                process = *it;
                if (dataTable.getArrivalTime()[process] <= currentTime && ( firstTime == -1 || ( dataTable.getRemainingTime()[process] < firstTime || ( dataTable.getRemainingTime()[process] == firstTime && dataTable.getProcessTag()[process] < firstTag  ) ) ) ) {
                    firstTime = dataTable.getRemainingTime()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }
    }
    else if (id == 3 || id == 4) {
        //PRIORITY / P-PRIORITY
        if (MLQ[numCola].isAscending()) {
            //ASC
            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {
                process = *it;
                if (dataTable.getArrivalTime()[process] <= currentTime && ( firstTime == -1 || ( dataTable.getPriority()[process] < firstTime || ( dataTable.getPriority()[process] == firstTime && dataTable.getProcessTag()[process] < firstTag  ) ) ) ) {
                    firstTime = dataTable.getPriority()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }
        else {
            //DESC
            for (std::set<int>::iterator it = MLQ[numCola].getAssociatedProcesses().begin(); it != MLQ[numCola].getAssociatedProcesses().end(); ++it) {
                process = *it;
                if (dataTable.getArrivalTime()[process] <= currentTime && ( firstTime == -1 || ( dataTable.getPriority()[process] > firstTime || ( dataTable.getPriority()[process] == firstTime && dataTable.getProcessTag()[process] < firstTag  ) ) ) ) {
                    firstTime = dataTable.getPriority()[process];
                    firstTag = dataTable.getProcessTag()[process];
                    firstProcess = process;
                }
            }
        }
    }
    else if (id == 5) {
        //RR
        for (std::set<int>::iterator itRR = MLQ[numCola].getAssociatedProcesses().begin(); itRR != MLQ[numCola].getAssociatedProcesses().end(); itRR++) {
            process = *itRR;

            if (MLQ[numCola].getArrivalT()[process] <= currentTime && (firstTime == -1 || MLQ[numCola].getArrivalT()[process] < firstTime || (MLQ[numCola].getArrivalT()[process] == firstTime && dataTable.getProcessTag()[process] < firstTag))) {

                firstTime = MLQ[numCola].getArrivalT()[process];
                firstTag = dataTable.getProcessTag()[process];
                firstProcess = process;
            }
        }
    }
    return firstProcess;
}

//==Getters==

std::vector<QUEUE>& SCHEDULER::getMLQ() {
    return MLQ;
}

TABLE& SCHEDULER::getTable() {
    return dataTable;
}