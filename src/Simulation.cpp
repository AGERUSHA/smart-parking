#include "Simulation.h"
#include <chrono>
#include <fstream>
#include <stdexcept>

Simulation::Simulation(const Config& c) : cfg(c), lot(c.rows, c.cols) {
    reset(c);
}

void Simulation::reset(const Config& newCfg) {
    newCfg.validate();
    cfg = newCfg;
    lot = ParkingLot(cfg.rows, cfg.cols);
    strategy = makeStrategy(cfg.strategy);
    cars.clear();
    waiting = std::queue<int>();
    nextCar = 0;
    gone = 0;
    time = 0;
    stats = Stats();

    // Бірдей seed -> бірдей келу кестесі (стратегияларды салыстыру үшін)
    std::mt19937 rng(cfg.seed);
    std::uniform_real_distribution<double> jitter(0.5, 1.5);
    std::uniform_real_distribution<double> stay(cfg.stayMin, cfg.stayMax);
    std::uniform_real_distribution<double> chance(0.0, 1.0);

    double t = 0;
    for (int i = 0; i < cfg.cars; i++) {
        t += cfg.interval * jitter(rng);
        double r = chance(rng);
        Kind kind = Kind::Normal;
        if (r < 0.1) kind = Kind::Disabled;
        else if (r < 0.2) kind = Kind::Electric;
        cars.emplace_back(i, kind, t, stay(rng));
    }

    // Брондалған, бірақ иесі келмейтін орындар
    std::uniform_int_distribution<int> pick(0, lot.size() - 1);
    int made = 0;
    while (made < cfg.reserved) {
        ParkingSpot& s = lot.spot(pick(rng));
        if (s.getKind() == Kind::Normal && s.isFree()) {
            s.reserve(cfg.reserveTime);
            made++;
        }
    }
}

void Simulation::step(double dt) {
    if (finished()) return;
    time += dt;

    stats.reservationsFreed += lot.releaseExpired(time);

    while (nextCar < cars.size() && cars[nextCar].getArriveTime() <= time) {
        waiting.push((int)nextCar);
        nextCar++;
        stats.arrived++;
    }

    serveQueue();

    float dist = CAR_SPEED * (float)dt;
    for (Car& car : cars) {
        if (car.getState() == CarState::Driving) {
            if (car.move(dist)) car.park(time);
        } else if (car.getState() == CarState::Parked) {
            if (time >= car.getParkedAt() + car.getStayTime()) {
                lot.spot(car.getSpot()).release();
                car.startLeaving(lot.routeFrom(car.getSpot()));
            }
        } else if (car.getState() == CarState::Leaving) {
            if (car.move(dist)) {
                car.finish();
                gone++;
                stats.left++;
            }
        }
    }

    collect(dt);
}

void Simulation::serveQueue() {
    while (!waiting.empty()) {
        Car& car = cars[waiting.front()];
        int checked = 0;
        auto t0 = std::chrono::steady_clock::now();
        int s = strategy->choose(lot, car, checked);
        auto t1 = std::chrono::steady_clock::now();
        if (s == -1) break; // орын жоқ, кезек күтеді

        stats.searches++;
        stats.checked += checked;
        stats.searchNs += (double)std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
        stats.distanceSum += lot.distance(s);
        stats.parked++;

        lot.spot(s).occupy(car.getId());
        car.startDriving(s, lot.routeTo(s));
        waiting.pop();
    }
}

void Simulation::collect(double dt) {
    stats.sampleTime += dt;

    int q = (int)waiting.size();
    stats.queueSum += q * dt;
    if (q > stats.maxQueue) stats.maxQueue = q;

    double occ = lot.occupancy();
    stats.occupancySum += occ * dt;
    if (occ > stats.maxOccupancy) stats.maxOccupancy = occ;

    double lo = 100, hi = 0;
    for (int z = 0; z < ParkingLot::ZONES; z++) {
        double zo = lot.zoneOccupancy(z);
        if (zo < lo) lo = zo;
        if (zo > hi) hi = zo;
    }
    stats.zoneDiffSum += (hi - lo) * dt;
}

void Simulation::runToEnd(double dt, double maxTime) {
    while (!finished() && time < maxTime)
        step(dt);
}

bool Simulation::finished() const {
    return gone == (int)cars.size();
}

bool Simulation::reserveSpot(int i) {
    if (i < 0 || i >= lot.size() || !lot.spot(i).isFree()) return false;
    lot.spot(i).reserve(time + cfg.reserveTime);
    return true;
}

void Simulation::saveResults(const std::string& path) const {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Файлды ашу мүмкін емес: " + path);
    f << "strategy=" << strategy->name() << "\n"
      << "cars=" << cfg.cars << "\n"
      << "time=" << time << "\n"
      << "arrived=" << stats.arrived << "\n"
      << "parked=" << stats.parked << "\n"
      << "left=" << stats.left << "\n"
      << "reservationsFreed=" << stats.reservationsFreed << "\n"
      << "avgSearchUs=" << stats.avgSearchUs() << "\n"
      << "avgChecked=" << stats.avgChecked() << "\n"
      << "avgDistance=" << stats.avgDistance() << "\n"
      << "maxQueue=" << stats.maxQueue << "\n"
      << "avgQueue=" << stats.avgQueue() << "\n"
      << "maxOccupancy=" << stats.maxOccupancy << "\n"
      << "avgOccupancy=" << stats.avgOccupancy() << "\n"
      << "avgZoneDiff=" << stats.avgZoneDiff() << "\n";
}
