#pragma once
#include <string>

// Симуляция параметрлері. Файлда "кілт=мән" түрінде сақталады.
struct Config {
    int rows = 4;
    int cols = 10;
    int cars = 50;            // келетін көліктер саны
    double interval = 1.5;    // орташа келу аралығы, сек
    double stayMin = 30;      // тұрақта тұру уақыты, сек
    double stayMax = 90;
    int reserved = 3;         // басында брондалатын орындар
    double reserveTime = 20;  // брондау мерзімі, сек
    unsigned seed = 42;
    std::string strategy = "nearest";

    void validate() const;    // қате болса std::runtime_error
    void save(const std::string& path) const;
    static Config load(const std::string& path);
};
