#include "Car.h"
#include <cmath>

Car::Car(int id, Kind kind, double arriveTime, double stayTime)
    : id(id), kind(kind), arriveTime(arriveTime), stayTime(stayTime) {}

void Car::startDriving(int spotIndex, const std::vector<Point>& path) {
    spot = spotIndex;
    route = path;
    step = 0;
    pos = Point{0, 0};
    state = CarState::Driving;
}

void Car::startLeaving(const std::vector<Point>& path) {
    route = path;
    step = 0;
    state = CarState::Leaving;
}

void Car::park(double now) {
    parkedAt = now;
    state = CarState::Parked;
}

void Car::finish() {
    state = CarState::Gone;
}

bool Car::move(float dist) {
    while (dist > 0 && step < route.size()) {
        Point target = route[step];
        float dx = target.x - pos.x;
        float dy = target.y - pos.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len <= dist) {
            pos = target;
            dist -= len;
            step++;
        } else {
            pos.x += dx / len * dist;
            pos.y += dy / len * dist;
            dist = 0;
        }
    }
    return step >= route.size();
}
