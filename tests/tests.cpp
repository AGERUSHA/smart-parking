#include "Simulation.h"
#include <cstdio>
#include <fstream>
#include <iostream>

static int passed = 0, failed = 0;

static void check(const std::string& name, const std::string& given,
                  const std::string& expected, const std::string& actual) {
    bool ok = expected == actual;
    (ok ? passed : failed)++;
    std::cout << (ok ? "[OK]   " : "[FAIL] ") << name << "\n"
              << "       Бастапқы шарт: " << given << "\n"
              << "       Күтілетін:     " << expected << "\n"
              << "       Нақты:         " << actual << "\n\n";
}

// 1. Қалыпты: бос тұрақта ең жақын қарапайым орын
static void testNearestEmpty() {
    ParkingLot lot(4, 10);
    Car car(0, Kind::Normal, 0, 10);
    int checked = 0;
    int s = NearestStrategy().choose(lot, car, checked);
    check("Ең жақын орын (бос тұрақ)",
          "4x10 бос тұрақ, қарапайым көлік; 0,1 - мүгедек орындары",
          "2", std::to_string(s));
}

// 2. Қалыпты: теңестіру басқа аймақты таңдайды
static void testBalancedZone() {
    ParkingLot lot(2, 9);
    lot.spot(2).occupy(100); // 0-аймақта бір орын бос емес
    Car car(0, Kind::Normal, 0, 10);
    int c1 = 0, c2 = 0;
    int nearest = NearestStrategy().choose(lot, car, c1);
    int balanced = BalancedStrategy().choose(lot, car, c2);
    check("Ең жақын vs теңестіру",
          "2x9 тұрақ, 2-орын бос емес (0-аймақ толуы 1/6)",
          "nearest=9 balanced=3",
          "nearest=" + std::to_string(nearest) + " balanced=" + std::to_string(balanced));
}

// 3. Қалыпты: 20 көлік толық сценарийі
static void testFullRun() {
    Config cfg;
    cfg.cars = 20;
    Simulation sim(cfg);
    sim.runToEnd();
    const Stats& st = sim.getStats();
    check("20 көлік сценарийі",
          "4x10 тұрақ, 20 көлік, seed=42",
          "parked=20 left=20 queue=0 free=40",
          "parked=" + std::to_string(st.parked) + " left=" + std::to_string(st.left) +
          " queue=" + std::to_string(sim.queueSize()) + " free=" + std::to_string(sim.getLot().freeCount()));
}

// 4. Шеткі: тұрақ толы - орын жоқ (-1), кезек өседі
static void testFullLot() {
    ParkingLot lot(1, 6);
    for (int i = 0; i < lot.size(); i++) lot.spot(i).occupy(i);
    Car car(0, Kind::Normal, 0, 10);
    int c1 = 0, c2 = 0;
    int a = NearestStrategy().choose(lot, car, c1);
    int b = BalancedStrategy().choose(lot, car, c2);

    Config cfg;
    cfg.rows = 1;
    cfg.cols = 6;
    cfg.cars = 20;
    cfg.interval = 0.5;
    cfg.stayMin = 500;
    cfg.stayMax = 600;
    cfg.reserved = 0;
    Simulation sim(cfg);
    for (int i = 0; i < 400; i++) sim.step(0.1); // 40 сек: бәрі келді, ешкім кетпеді
    check("Толы тұрақ және кезек",
          "1x6 тұрақтың барлық орны бос емес; симуляцияда 6 орынға 20 көлік",
          "nearest=-1 balanced=-1 parked<=6 queue>0",
          "nearest=" + std::to_string(a) + " balanced=" + std::to_string(b) +
          (sim.getStats().parked <= 6 ? " parked<=6" : " parked>6") +
          (sim.queueSize() > 0 ? " queue>0" : " queue=0"));
}

// 5. Шеткі: брондау мерзімі өткенде орын босайды
static void testReservationExpired() {
    ParkingLot lot(1, 6);
    lot.spot(3).reserve(10.0);
    int before = lot.releaseExpired(9.9);
    bool stillReserved = lot.spot(3).getState() == SpotState::Reserved;
    int after = lot.releaseExpired(10.0);
    check("Брондау мерзімі өтті",
          "3-орын t=10 с дейін брондалған; t=9.9 және t=10 тексеру",
          "0 reserved; 1 free",
          std::to_string(before) + (stillReserved ? " reserved" : " free") + "; " +
          std::to_string(after) + (lot.spot(3).isFree() ? " free" : " reserved"));
}

// 6. Қате енгізу: жарамсыз конфиг файлы және бос емес орынға тұру
static void testBadInput() {
    std::string errors;
    const char* files[] = {"cars=abc\n", "cars=-5\n", "rows=2\nspeed=3\n", "strategy=random\n"};
    for (const char* text : files) {
        std::ofstream("bad_config.txt") << text;
        try {
            Config::load("bad_config.txt");
            errors += "ok ";
        } catch (const std::runtime_error&) {
            errors += "err ";
        }
    }
    std::remove("bad_config.txt");

    ParkingSpot spot(0, 0, 0, 0, Kind::Normal);
    spot.occupy(1);
    try {
        spot.occupy(2);
        errors += "ok";
    } catch (const std::logic_error&) {
        errors += "err";
    }
    check("Жарамсыз енгізу",
          "cars=abc, cars=-5, белгісіз кілт, қате стратегия; бос емес орынға екінші көлік",
          "err err err err err", errors);
}

int main() {
    testNearestEmpty();
    testBalancedZone();
    testFullRun();
    testFullLot();
    testReservationExpired();
    testBadInput();
    std::cout << "Өтті: " << passed << ", құлады: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}
