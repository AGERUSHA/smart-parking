#pragma once
#include "Car.h"

enum class SpotState { Free, Occupied, Reserved };

class ParkingSpot {
public:
    ParkingSpot(int id, int row, int col, int zone, Kind kind);

    int getId() const { return id; }
    int getRow() const { return row; }
    int getCol() const { return col; }
    int getZone() const { return zone; }
    Kind getKind() const { return kind; }
    SpotState getState() const { return state; }
    int getCarId() const { return carId; }
    double getReservedUntil() const { return reservedUntil; }

    bool isFree() const { return state == SpotState::Free; }
    // Қарапайым орынға кез келген көлік, арнайы орынға тек сол түрі
    bool fits(Kind car) const;

    void occupy(int car);
    void reserve(double until);
    void release();
    bool reservationExpired(double now) const;

private:
    int id;
    int row;
    int col;
    int zone;
    Kind kind;
    SpotState state = SpotState::Free;
    int carId = -1;
    double reservedUntil = 0;
};
