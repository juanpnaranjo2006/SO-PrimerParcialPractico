//
// Created by juanp on 16/03/2026.
//

#ifndef SCHEDULINGSIMSO_TABLE_H
#define SCHEDULINGSIMSO_TABLE_H

#include <vector>
#include <string>

//Tabla donde se almacenan todos los datos relevantes para ejecutar los algoritmos (como si fuera un ejercicio)
class TABLE {
private:
    int size;
    std::vector<std::string> processTag;
    std::vector<int> burstTime;
    std::vector<int> arrivalTime;
    std::vector<int> queue;
    std::vector<int> priority;
    std::vector<int> responseTime;
    std::vector<int> completionTime;
    std::vector<int> TAT;
    std::vector<int> waitingTime;
    std::vector<int> remainingTime;
public:
    TABLE() = default;

    //==Métodos principales==
    void extractDataFromFile(const std::string &path);

    //==Getters==
    int getSize();
    std::vector<std::string>& getProcessTag();
    std::vector<int>& getBurstTime();
    std::vector<int>& getArrivalTime();
    std::vector<int>& getQueue();
    std::vector<int>& getPriority();
    std::vector<int>& getResponseTime();
    std::vector<int>& getCompletionTime();
    std::vector<int>& getTAT();
    std::vector<int>& getWaitingTime();
    std::vector<int>& getRemainingTime();

};


#endif //SCHEDULINGSIMSO_TABLE_H