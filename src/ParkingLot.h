#pragma once
#include "ParkingSpot.h"
#include <vector>

// Тұрақ торы: y=2r жолында жол (проезд), y=2r+1 жолында r-ші қатар орындар.
// Кіру/шығу нүктесі (0,0), сол жақта тік жол x=0.
class ParkingLot {
public:
    static const int ZONES = 3;

    ParkingLot(int rows, int cols);

    int getRows() const { return rows; }
    int getCols() const { return cols; }
    int size() const { return (int)spots.size(); }

    const ParkingSpot& spot(int i) const { return spots.at(i); }
    ParkingSpot& spot(int i) { return spots.at(i); }

    int distance(int i) const;              // кіреберістен орынға дейінгі жол ұзындығы
    std::vector<Point> routeTo(int i) const; // кіреберістен орынға
    std::vector<Point> routeFrom(int i) const; // орыннан шығуға

    int freeCount() const;
    double occupancy() const;            // бос емес орындар, %
    double zoneOccupancy(int zone) const; // аймақ бойынша, %

    int releaseExpired(double now); // мерзімі өткен броньдарды босатады

private:
    int rows;
    int cols;
    std::vector<ParkingSpot> spots;
};
