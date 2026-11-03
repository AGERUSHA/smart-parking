// 20, 50, 100 көлік сценарийлерін екі стратегиямен бірдей шартта өлшеу
#include "Simulation.h"
#include <fstream>
#include <iomanip>
#include <iostream>

int main() {
    const int scenarios[] = {20, 50, 100};
    const char* strategies[] = {"nearest", "balanced"};

    std::ofstream csv("results.csv");
    csv << "cars,strategy,avgSearchUs,avgChecked,avgDistance,maxQueue,avgQueue,maxOccupancy,avgOccupancy,avgZoneDiff\n";

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "cars  strategy   search_us  checked  distance  maxQueue  avgQueue  maxOcc%  avgOcc%  zoneDiff%\n";

    for (int n : scenarios) {
        for (const char* name : strategies) {
            Config cfg; // 4x10, interval=1.5, stay 30..90, seed=42
            cfg.cars = n;
            cfg.strategy = name;
            Simulation sim(cfg);
            sim.runToEnd();
            const Stats& s = sim.getStats();

            std::cout << std::setw(4) << n << "  " << std::left << std::setw(9) << name << std::right
                      << std::setw(11) << s.avgSearchUs() << std::setw(9) << s.avgChecked()
                      << std::setw(10) << s.avgDistance() << std::setw(10) << s.maxQueue
                      << std::setw(10) << s.avgQueue() << std::setw(9) << s.maxOccupancy
                      << std::setw(9) << s.avgOccupancy() << std::setw(11) << s.avgZoneDiff() << "\n";

            csv << n << "," << name << "," << s.avgSearchUs() << "," << s.avgChecked() << ","
                << s.avgDistance() << "," << s.maxQueue << "," << s.avgQueue() << ","
                << s.maxOccupancy << "," << s.avgOccupancy() << "," << s.avgZoneDiff() << "\n";
        }
    }
    std::cout << "\nresults.csv сақталды\n";
    return 0;
}
