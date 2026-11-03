#include "ParkingSpot.h"
#include <stdexcept>
#include <string>

ParkingSpot::ParkingSpot(int id, int row, int col, int zone, Kind kind)
    : id(id), row(row), col(col), zone(zone), kind(kind) {}

bool ParkingSpot::fits(Kind car) const {
    return kind == Kind::Normal || kind == car;
}

void ParkingSpot::occupy(int car) {
    if (state != SpotState::Free)
        throw std::logic_error("Орын бос емес: " + std::to_string(id));
    state = SpotState::Occupied;
    carId = car;
}

void ParkingSpot::reserve(double until) {
    if (state != SpotState::Free)
        throw std::logic_error("Бос емес орынды брондауға болмайды: " + std::to_string(id));
    state = SpotState::Reserved;
    reservedUntil = until;
}

void ParkingSpot::release() {
    state = SpotState::Free;
    carId = -1;
    reservedUntil = 0;
}

bool ParkingSpot::reservationExpired(double now) const {
    return state == SpotState::Reserved && now >= reservedUntil;
}
