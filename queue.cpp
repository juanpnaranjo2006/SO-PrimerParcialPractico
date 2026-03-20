//
// Created by juanp on 16/03/2026.
//

#include "queue.h"

//==Constructor==
QUEUE::QUEUE(const bool isPreemptive, const bool order, const int algID, const int quantum) {
    this->isPreemptive = isPreemptive;
    this->order = order;
    this->algID = algID;
    this->quantum = quantum;
    asociatedProcesses.clear();
    arrivalT.clear();
    firstTimeSJF = true;
}

//==Métodos Principales==
void QUEUE::addProcess(int processID, int time) {
    asociatedProcesses.emplace(processID);
    arrivalT.emplace(processID, time);
    visited.emplace(processID, false);
}

void QUEUE::removeProcess(int processID) {
    asociatedProcesses.erase(processID);
    arrivalT.erase(processID);
    visited.erase(processID);
}

bool QUEUE::isEmpty() const {
    return asociatedProcesses.empty();
}

//==Getters==
int QUEUE::get_algID() const {
    return algID;
}

int QUEUE::get_quantum() const {
    return quantum;
}

bool QUEUE::isPreemp() const {
    return isPreemptive;
}

bool QUEUE::isAscending() const {
    return order;
}

bool QUEUE::isFirstTimeSJF() const {
    return firstTimeSJF;
}

std::set<int>& QUEUE::getAssociatedProcesses(){
    return asociatedProcesses;
}

std::map<int, int>& QUEUE::getArrivalT() {
    return arrivalT;
}

std::map<int, bool>& QUEUE::getVisited() {
    return visited;
}

//==Setters==
void QUEUE::set_algID(const int value) {
    algID = value;
}

void QUEUE::set_algID(const std::string &value) {
    //Falta implementar;
}

void QUEUE::set_quantum(const int value) {
    quantum = value;
}

void QUEUE::set_firstTimeSJF(const bool value) {
    firstTimeSJF = value;
}