//
// Created by juanp on 16/03/2026.
//

#ifndef SCHEDULINGSIMSO_SCHEDULER_H
#define SCHEDULINGSIMSO_SCHEDULER_H

#include "table.h"
#include "queue.h"

class SCHEDULER {
private:
    std::vector<QUEUE> MLQ;
    TABLE dataTable;
    int numQ;
    int multiQueueType; // 0 = Only one queue with 1 algorithm, 1 = MLQ, 2 = MLFQ
    std::set<int> relevantTimes;
    int currentTime;
    int numPCompleted;

    //==Métodos privados==
    std::string toUpper(const std::string& input);
    void emplaceAlg(std::string &alg, std::string &param);
    void assignProcesses();
    void setRelevantTimes();
public:
    SCHEDULER(std::vector<std::string> &arguments); //Debe incluir el nombre del archivo para mandar a la TABLE a leerlo

    //==Métodos Principales==
    void simulation(); //Al final la tabla queda con todos los datos necesarios
    int executeProcess(int numCola);
    int determineProcess(int numCola);

    //==Getters==
    std::vector<QUEUE>& getMLQ();
    TABLE& getTable();
};


#endif //SCHEDULINGSIMSO_SCHEDULER_H