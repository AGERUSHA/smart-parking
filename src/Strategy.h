#pragma once
#include "ParkingLot.h"
#include <memory>
#include <string>

// Орын таңдау стратегиясы. checked - қаралған орындар саны (салыстыру үшін).
class Strategy {
public:
    virtual ~Strategy() = default;
    virtual std::string name() const = 0;
    virtual int choose(const ParkingLot& lot, const Car& car, int& checked) const = 0;

protected:
    // Мүгедек/электр көлігіне өз түріндегі ең жақын бос орын (жоқ болса -1)
    static int nearestSpecial(const ParkingLot& lot, const Car& car, int& checked);
};

// 1-стратегия: кіреберіске ең жақын бос орын
class NearestStrategy : public Strategy {
public:
    std::string name() const override { return "nearest"; }
    int choose(const ParkingLot& lot, const Car& car, int& checked) const override;
};

// 2-стратегия: ең аз толған аймақ, сол аймақтағы ең жақын орын
class BalancedStrategy : public Strategy {
public:
    std::string name() const override { return "balanced"; }
    int choose(const ParkingLot& lot, const Car& car, int& checked) const override;
};

std::unique_ptr<Strategy> makeStrategy(const std::string& name);
