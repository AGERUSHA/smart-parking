#include "Strategy.h"
#include <stdexcept>

int Strategy::nearestSpecial(const ParkingLot& lot, const Car& car, int& checked) {
    if (car.getKind() == Kind::Normal) return -1;
    int best = -1;
    for (int i = 0; i < lot.size(); i++) {
        checked++;
        const ParkingSpot& s = lot.spot(i);
        if (s.getKind() != car.getKind() || !s.isFree()) continue;
        if (best == -1 || lot.distance(i) < lot.distance(best))
            best = i;
    }
    return best;
}

int NearestStrategy::choose(const ParkingLot& lot, const Car& car, int& checked) const {
    int best = nearestSpecial(lot, car, checked);
    if (best != -1) return best;

    // Сызықтық іздеу: ең кіші қашықтық (тең болса - бірінші табылғаны)
    for (int i = 0; i < lot.size(); i++) {
        checked++;
        const ParkingSpot& s = lot.spot(i);
        if (s.getKind() != Kind::Normal || !s.isFree()) continue;
        if (best == -1 || lot.distance(i) < lot.distance(best))
            best = i;
    }
    return best;
}

int BalancedStrategy::choose(const ParkingLot& lot, const Car& car, int& checked) const {
    int best = nearestSpecial(lot, car, checked);
    if (best != -1) return best;

    // 1) әр аймақтың жүктемесін санау
    int total[ParkingLot::ZONES] = {};
    int busy[ParkingLot::ZONES] = {};
    int freeNormal[ParkingLot::ZONES] = {};
    for (int i = 0; i < lot.size(); i++) {
        checked++;
        const ParkingSpot& s = lot.spot(i);
        int z = s.getZone();
        total[z]++;
        if (!s.isFree()) busy[z]++;
        else if (s.getKind() == Kind::Normal) freeNormal[z]++;
    }

    // 2) бос орны бар, ең аз толған аймақ
    int zone = -1;
    double zoneLoad = 0;
    for (int z = 0; z < ParkingLot::ZONES; z++) {
        if (freeNormal[z] == 0) continue;
        double load = (double)busy[z] / total[z];
        if (zone == -1 || load < zoneLoad) {
            zone = z;
            zoneLoad = load;
        }
    }
    if (zone == -1) return -1;

    // 3) сол аймақтағы ең жақын бос орын
    for (int i = 0; i < lot.size(); i++) {
        const ParkingSpot& s = lot.spot(i);
        if (s.getZone() != zone) continue;
        checked++;
        if (s.getKind() != Kind::Normal || !s.isFree()) continue;
        if (best == -1 || lot.distance(i) < lot.distance(best))
            best = i;
    }
    return best;
}

std::unique_ptr<Strategy> makeStrategy(const std::string& name) {
    if (name == "nearest") return std::make_unique<NearestStrategy>();
    if (name == "balanced") return std::make_unique<BalancedStrategy>();
    throw std::invalid_argument("Белгісіз стратегия: " + name);
}
