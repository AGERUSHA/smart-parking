#pragma once
#include <vector>

// Көлік пен орын түрі (бір enum екеуіне де ортақ)
enum class Kind { Normal, Disabled, Electric };

enum class CarState { Waiting, Driving, Parked, Leaving, Gone };

struct Point {
    float x = 0;
    float y = 0;
};

class Car {
public:
    Car(int id, Kind kind, double arriveTime, double stayTime);

    int getId() const { return id; }
    Kind getKind() const { return kind; }
    CarState getState() const { return state; }
    double getArriveTime() const { return arriveTime; }
    double getStayTime() const { return stayTime; }
    double getParkedAt() const { return parkedAt; }
    int getSpot() const { return spot; }
    Point getPos() const { return pos; }

    void startDriving(int spotIndex, const std::vector<Point>& path);
    void startLeaving(const std::vector<Point>& path);
    void park(double now);
    void finish();

    // Маршрут бойымен dist қашықтыққа жылжиды, соңына жетсе true
    bool move(float dist);

private:
    int id;
    Kind kind;
    CarState state = CarState::Waiting;
    double arriveTime;
    double stayTime;
    double parkedAt = 0;
    int spot = -1;
    Point pos;
    std::vector<Point> route;
    size_t step = 0;
};
