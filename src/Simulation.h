#pragma once
#include "Config.h"
#include "Strategy.h"
#include <queue>
#include <random>

// Өлшенетін нәтижелер
struct Stats {
    int arrived = 0;
    int parked = 0;
    int left = 0;
    int reservationsFreed = 0;

    int searches = 0;          // сәтті іздеулер
    long long checked = 0;     // барлық қаралған орындар
    double searchNs = 0;       // іздеуге кеткен уақыт, нс
    double distanceSum = 0;    // көліктердің жүрген жолы

    int maxQueue = 0;
    double queueSum = 0;       // уақыт бойынша интеграл
    double maxOccupancy = 0;
    double occupancySum = 0;
    double zoneDiffSum = 0;    // аймақтар арасындағы айырмашылық
    double sampleTime = 0;

    double avgSearchUs() const { return searches ? searchNs / searches / 1000.0 : 0; }
    double avgChecked() const { return searches ? (double)checked / searches : 0; }
    double avgDistance() const { return searches ? distanceSum / searches : 0; }
    double avgQueue() const { return sampleTime > 0 ? queueSum / sampleTime : 0; }
    double avgOccupancy() const { return sampleTime > 0 ? occupancySum / sampleTime : 0; }
    double avgZoneDiff() const { return sampleTime > 0 ? zoneDiffSum / sampleTime : 0; }
};

class Simulation {
public:
    static constexpr float CAR_SPEED = 3.0f; // тор ұяшығы / сек

    explicit Simulation(const Config& cfg);

    void reset(const Config& newCfg);
    void step(double dt);
    void runToEnd(double dt = 0.1, double maxTime = 100000);
    bool finished() const;

    bool reserveSpot(int i); // интерфейстен брондау

    const Config& getConfig() const { return cfg; }
    const ParkingLot& getLot() const { return lot; }
    const std::vector<Car>& getCars() const { return cars; }
    const Stats& getStats() const { return stats; }
    const Strategy& getStrategy() const { return *strategy; }
    size_t queueSize() const { return waiting.size(); }
    double getTime() const { return time; }

    void saveResults(const std::string& path) const;

private:
    void serveQueue();
    void collect(double dt);

    Config cfg;
    ParkingLot lot;
    std::unique_ptr<Strategy> strategy;
    std::vector<Car> cars;
    std::queue<int> waiting;
    size_t nextCar = 0;
    int gone = 0;
    double time = 0;
    Stats stats;
};
