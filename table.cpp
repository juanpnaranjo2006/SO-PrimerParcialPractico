//
// Created by juanp on 16/03/2026.
//

#include "table.h"
#include <fstream>
#include <stdexcept>
#include <exception>
#include <sstream>
#include <iostream>
#include <string>

void TABLE::extractDataFromFile(const std::string &path) {
    std::ifstream file(path);
    size = 0;

    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + path);
        file.close();
    }

    std::string line;
    std::istringstream lineStream;
    std::string token;
    int conv;
    processTag.clear();
    burstTime.clear();
    arrivalTime.clear();
    queue.clear();
    priority.clear();
    waitingTime.clear();
    remainingTime.clear();
    TAT.clear();
    completionTime.clear();
    responseTime.clear();
    while (std::getline(file, line)) {
        if (!line.empty() && line[0] != '#') {
            lineStream.clear();
            lineStream.str(line);

            lineStream >> token;
            token.pop_back();
            processTag.push_back(token);

            lineStream >> token;
            token.pop_back();
            conv = std::stoi(token);
            burstTime.push_back(conv);
            remainingTime.push_back(conv);

            lineStream >> token;
            token.pop_back();
            arrivalTime.push_back(std::stoi(token));

            lineStream >> token;
            token.pop_back();
            queue.push_back(std::stoi(token));

            lineStream >> token; //No se hace pop_back puesto que el ultimo elemento NO tiene separador ";" a su derecha
            priority.push_back(std::stoi(token));

            waitingTime.push_back(0);
            TAT.push_back(-1); //Valores por defecto, si quedan así es porque no se calcularon al final como deberían haber sido
            completionTime.push_back(-1);
            responseTime.push_back(-1);

            size++;
        }
    }

    if (!file.eof()) {
        file.close();
        throw std::runtime_error("Error al leer el archivo: " + path);
    }
    file.close();
}

//==Getters==

std::vector<std::string>& TABLE::getProcessTag() {
    return processTag;
}

std::vector<int>& TABLE::getBurstTime() {
    return burstTime;
}

std::vector<int>& TABLE::getArrivalTime() {
    return arrivalTime;
}

std::vector<int>& TABLE::getQueue() {
    return queue;
}

std::vector<int>& TABLE::getPriority() {
    return priority;
}

std::vector<int>& TABLE::getResponseTime() {
    return responseTime;
}

std::vector<int>& TABLE::getCompletionTime() {
    return completionTime;
}

std::vector<int>& TABLE::getTAT() {
    return TAT;
}

std::vector<int>& TABLE::getWaitingTime() {
    return waitingTime;
}

std::vector<int>& TABLE::getRemainingTime() {
    return remainingTime;
}

int TABLE::getSize() {
    return size;
}