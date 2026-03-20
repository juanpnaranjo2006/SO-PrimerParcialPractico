#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include "scheduler.h"

int main(int argc, char* argv[]) {
    //Recordar, argv[0] siempre es el nombre del ejecutable
    if (argc == 2) {
       std::string argument(argv[1]);
        if (argument == "-h" || argument == "--help") {
            std::cout << "Usage: SchedulingSim.exe <inputFile> <schedulingAlgorithm> [params]\n";
            std::cout << "Additional parameters may be required depending on the selected algorithm.\n";
            std::cout << "Available options for <schedulingAlgorithm>:\n";
            std::cout << "--------------------------------------------------------------\n";
            std::cout << "FCFS                                : First Come First Served\n";
            std::cout << "SJF                                 : Shortest Job First (Non-Preemptive)\n";
            std::cout << "PSJF                                : Preemptive Shortest Job First\n";
            std::cout << "PRIORITY [asc|des]                  : Priority (Non-Preemptive) with either ascending or descending order\n";
            std::cout << "P-PRIORITY [asc|des]                : Preemptive Priority with either ascending or descending order\n";
            std::cout << "RR [Q]                              : Round Robin with quantum Q\n";
            std::cout << "MLQ [N] [schAlg1] ... [schAlgN]     : Multi-Level Queue with N queues\n";
            std::cout << "MLFQ [N] [Q_1] ... [Q_N-1] [schAlg] : Multi-Level Feedback Queue with N queues\n";
            std::cout << "Notes:\n";
            std::cout << "  - For MLQ and MLFQ, specify the algorithms in order using same notation shown above, leaving at least a blank space between each queue algorithm.\n";
            std::cout << "  - For MLFQ, even if the last algorithm is Round Robin, it must also be specified so with the notation shown above.\n";
        }
        else {
            std::cout << "Try 'SchedulingSim.exe --help' for more information.\n";
        }

        return 1;
    }
    else if (argc < 3) {
        std::cout << "Error: missing required arguments.\n";
        std::cout << "Try 'SchedulingSim.exe --help' for more information.\n";
        return -1;
    }
    else {
        //Normal execution
        try {
            //Primero vamos a reunir todos los argumentos del uso del simulador para enpaquetarlos de forma que se pueda inicializar el scheduler
            std::vector<std::string> args;
            args.emplace_back(argv[1]);
            args.emplace_back(argv[2]);
            if (args[1] == "PRIORITY" || args[1] == "P-PRIORITY" || args[1] == "RR") {
                args.emplace_back(argv[3]);
            }
            else if (args[1] == "MLQ") {
                args.emplace_back(argv[3]);
                std::string algType;
                for (int i = 4; i < argc;) {
                    algType = argv[i];
                    if (algType == "PRIORITY" || algType == "P-PRIORITY" || algType == "RR") {
                        args.emplace_back(algType + " " + std::string(argv[i + 1]));
                        i += 2;
                    }
                    else {
                        args.emplace_back(algType);
                        i += 1;
                    }
                }
            }
            else if (args[1] == "MLFQ") {
                args.emplace_back(argv[3]);
                int i;
                int n = std::stoi(argv[3]);
                for (i = 0; i < n - 1; i++) {
                    args.emplace_back(argv[4 + i]);
                }
                std::string algType = argv[4 + i];
                if (algType == "PRIORITY" || algType == "P-PRIORITY" || algType == "RR") {
                    args.emplace_back(algType + " " + std::string(argv[4 + i + 1]));
                    i += 2;
                }
                else {
                    args.emplace_back(algType);
                    i += 1;
                }
            }
            //Creación del SCHEDULER
            SCHEDULER sch = SCHEDULER(args);
            sch.simulation();

            //Impresión de los datos de la tabla
            // Crear archivo con nombre único
            std::ofstream outputFile("output.txt");

            float promRes = 0;
            float promComp = 0;
            float promTAT = 0;
            float promWait = 0;
            outputFile << "# archivo: " << argv[1] << "\n";
            outputFile << "# etiqueta; BT; AT; Q; Pr; WT; CT; RT; TAT" << "\n";
            for (int i = 0; i < sch.getTable().getSize(); ++i) {
                outputFile << sch.getTable().getProcessTag()[i] << "; "
                << sch.getTable().getBurstTime()[i] << "; "
                << sch.getTable().getArrivalTime()[i] << "; "
                << sch.getTable().getQueue()[i] << "; "
                << sch.getTable().getPriority()[i] << "; "
                << sch.getTable().getWaitingTime()[i] << "; "
                << sch.getTable().getCompletionTime()[i] << "; "
                << sch.getTable().getResponseTime()[i] << "; "
                << sch.getTable().getTAT()[i] << "\n";

                promRes += static_cast<float>(sch.getTable().getResponseTime()[i]);
                promComp += static_cast<float>(sch.getTable().getCompletionTime()[i]);
                promTAT += static_cast<float>(sch.getTable().getTAT()[i]);
                promWait += static_cast<float>(sch.getTable().getWaitingTime()[i]);
            }
            promRes = promRes / static_cast<float>(sch.getTable().getSize());
            promComp = promComp / static_cast<float>(sch.getTable().getSize());
            promTAT = promTAT / static_cast<float>(sch.getTable().getSize());
            promWait = promWait / static_cast<float>(sch.getTable().getSize());
            outputFile << std::fixed << std::setprecision(1) << "WT=" << promWait << "; CT=" << promComp << "; RT=" << promRes << "; TAT=" << promTAT << ";\n";

            outputFile.close();
        }
        catch (std::exception& e) {
            std::cout << "Error: " << e.what() << "\nThe issue is most likely due to incorrect input parameters:\n";
            for (int i = 0; i < argc; i++) {
                std::cout << argv[i];
                if (i != argc - 1) {
                    std::cout << " ";
                }
                else {
                    std::cout << "\n";
                }
            }
            std::cout << "Try 'SchedulingSim.exe --help' for more information.\n";
            return -1;
        }
    }
    return 0;
}