#include "Config.h"
#include <fstream>
#include <stdexcept>

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r");
    return s.substr(a, b - a + 1);
}

static int toInt(const std::string& key, const std::string& v) {
    try {
        size_t pos = 0;
        int x = std::stoi(v, &pos);
        if (pos == v.size()) return x;
    } catch (...) {}
    throw std::runtime_error("Қате бүтін сан: " + key + "=" + v);
}

static double toDouble(const std::string& key, const std::string& v) {
    try {
        size_t pos = 0;
        double x = std::stod(v, &pos);
        if (pos == v.size()) return x;
    } catch (...) {}
    throw std::runtime_error("Қате сан: " + key + "=" + v);
}

void Config::validate() const {
    if (rows < 1 || rows > 6) throw std::runtime_error("rows 1..6 аралығында болуы керек");
    if (cols < 6 || cols > 14) throw std::runtime_error("cols 6..14 аралығында болуы керек");
    if (cars < 1 || cars > 500) throw std::runtime_error("cars 1..500 аралығында болуы керек");
    if (interval <= 0 || interval > 60) throw std::runtime_error("interval 0..60 аралығында болуы керек");
    if (stayMin <= 0 || stayMax < stayMin) throw std::runtime_error("stayMin/stayMax қате");
    if (reserved < 0 || reserved > (rows * cols - 4) / 2) throw std::runtime_error("reserved тым көп");
    if (reserveTime <= 0) throw std::runtime_error("reserveTime оң сан болуы керек");
    if (strategy != "nearest" && strategy != "balanced")
        throw std::runtime_error("strategy: nearest немесе balanced");
}

void Config::save(const std::string& path) const {
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Файлды ашу мүмкін емес: " + path);
    f << "rows=" << rows << "\n"
      << "cols=" << cols << "\n"
      << "cars=" << cars << "\n"
      << "interval=" << interval << "\n"
      << "stayMin=" << stayMin << "\n"
      << "stayMax=" << stayMax << "\n"
      << "reserved=" << reserved << "\n"
      << "reserveTime=" << reserveTime << "\n"
      << "seed=" << seed << "\n"
      << "strategy=" << strategy << "\n";
}

Config Config::load(const std::string& path) {
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Файл табылмады: " + path);

    Config c;
    std::string line;
    while (std::getline(f, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq == std::string::npos) throw std::runtime_error("'=' жоқ жол: " + line);
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        if (key == "rows") c.rows = toInt(key, val);
        else if (key == "cols") c.cols = toInt(key, val);
        else if (key == "cars") c.cars = toInt(key, val);
        else if (key == "interval") c.interval = toDouble(key, val);
        else if (key == "stayMin") c.stayMin = toDouble(key, val);
        else if (key == "stayMax") c.stayMax = toDouble(key, val);
        else if (key == "reserved") c.reserved = toInt(key, val);
        else if (key == "reserveTime") c.reserveTime = toDouble(key, val);
        else if (key == "seed") c.seed = (unsigned)toInt(key, val);
        else if (key == "strategy") c.strategy = val;
        else throw std::runtime_error("Белгісіз параметр: " + key);
    }
    c.validate();
    return c;
}
