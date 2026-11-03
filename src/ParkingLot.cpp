#include "ParkingLot.h"
#include <stdexcept>

ParkingLot::ParkingLot(int rows, int cols) : rows(rows), cols(cols) {
    if (rows < 1 || cols < 6)
        throw std::invalid_argument("Тұрақ өлшемі қате (rows >= 1, cols >= 6)");

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            Kind kind = Kind::Normal;
            if (r == 0 && c < 2)
                kind = Kind::Disabled;          // кіреберіске ең жақын екі орын
            else if (r == rows - 1 && c >= cols - 2)
                kind = Kind::Electric;          // соңғы қатардағы екі орын
            int zone = c * ZONES / cols;
            spots.emplace_back(r * cols + c, r, c, zone, kind);
        }
    }
}

int ParkingLot::distance(int i) const {
    const ParkingSpot& s = spot(i);
    return 2 * s.getRow() + (s.getCol() + 1) + 1;
}

std::vector<Point> ParkingLot::routeTo(int i) const {
    const ParkingSpot& s = spot(i);
    float x = (float)s.getCol() + 1;
    float aisle = 2.0f * s.getRow();
    return {{0, aisle}, {x, aisle}, {x, aisle + 1}};
}

std::vector<Point> ParkingLot::routeFrom(int i) const {
    const ParkingSpot& s = spot(i);
    float x = (float)s.getCol() + 1;
    float aisle = 2.0f * s.getRow();
    return {{x, aisle}, {0, aisle}, {0, 0}};
}

int ParkingLot::freeCount() const {
    int n = 0;
    for (const ParkingSpot& s : spots)
        if (s.isFree()) n++;
    return n;
}

double ParkingLot::occupancy() const {
    return 100.0 * (size() - freeCount()) / size();
}

double ParkingLot::zoneOccupancy(int zone) const {
    int total = 0, busy = 0;
    for (const ParkingSpot& s : spots) {
        if (s.getZone() != zone) continue;
        total++;
        if (!s.isFree()) busy++;
    }
    return total == 0 ? 0 : 100.0 * busy / total;
}

int ParkingLot::releaseExpired(double now) {
    int n = 0;
    for (ParkingSpot& s : spots) {
        if (s.reservationExpired(now)) {
            s.release();
            n++;
        }
    }
    return n;
}
