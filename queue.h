//
// Created by juanp on 16/03/2026.
//
#ifndef SCHEDULINGSIMSO_QUEUE_H
#define SCHEDULINGSIMSO_QUEUE_H

#include <string>
#include <queue>
#include <set>
#include <map>

//Representa una cola con un algoritmo particular
class QUEUE {
private:
    bool isPreemptive;
    bool order; //False = desc, True = asc
    int algID;
    int quantum;
    bool firstTimeSJF;
    std::set<int> asociatedProcesses;
    std::map<int, int> arrivalT;
    std::map<int, bool> visited;
public:
    QUEUE(bool isPreemptive, bool order, int algID, int quantum);

    //==Métodos Principales==
    void addProcess(int processID, int time);
    void removeProcess(int processID);
    bool isEmpty() const;

    //==Getters==
    int get_algID() const;
    int get_quantum() const;
    bool isPreemp() const;
    bool isAscending() const;
    bool isFirstTimeSJF() const;
    std::set<int>::iterator getItRR() const;
    std::set<int>& getAssociatedProcesses();
    std::map<int, int>& getArrivalT();
    std::map<int, bool>& getVisited();

    //==Setters==
    void set_algID(int value);
    void set_algID(const std::string &value);
    void set_quantum(int value);
    void set_firstTimeSJF(bool value);
};


#endif //SCHEDULINGSIMSO_QUEUE_H