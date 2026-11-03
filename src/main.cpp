#include "Simulation.h"
#include <SFML/Graphics.hpp>
#include <cmath>
#include <functional>
#include <iomanip>
#include <iostream>
#include <sstream>

const float CELL = 50.f;
const sf::Vector2f MAP_POS(48.f, 116.f);
const float PANEL_X = 764.f;
const float PANEL_W = 308.f;
const double STEP = 0.05; // симуляцияның бір қадамы, сек

// Түстер
const sf::Color BG(17, 20, 28);
const sf::Color CARD(27, 31, 42);
const sf::Color ROAD(48, 52, 64);
const sf::Color TEXT(230, 233, 240);
const sf::Color MUTED(140, 148, 165);
const sf::Color GREEN(46, 170, 100);
const sf::Color RED(214, 76, 86);
const sf::Color AMBER(226, 168, 52);
const sf::Color BLUE(70, 130, 240);
const sf::Color CYAN(40, 205, 200);

struct Button {
    sf::FloatRect box;
    std::function<std::string()> label;
    std::function<void()> onClick;
    sf::Color color;
};

static sf::String U(const std::string& s) {
    return sf::String::fromUtf8(s.begin(), s.end());
}

static std::string num(double v, int prec = 1) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(prec) << v;
    return out.str();
}

static sf::Color lighter(sf::Color c, int d) {
    auto f = [d](std::uint8_t v) { return (std::uint8_t)std::max(0, std::min(255, v + d)); };
    return sf::Color(f(c.r), f(c.g), f(c.b), c.a);
}

static sf::Color carColor(const Car& car) {
    if (car.getKind() == Kind::Disabled) return BLUE;
    if (car.getKind() == Kind::Electric) return CYAN;
    const sf::Color palette[] = {{236, 236, 240}, {240, 140, 60}, {240, 200, 70}, {170, 176, 190}, {230, 110, 150}};
    return palette[car.getId() % 5];
}

// Бұрыштары дөңгелектенген төртбұрыш
static sf::ConvexShape rounded(sf::Vector2f size, float r) {
    const int N = 6;
    sf::ConvexShape s(4 * N);
    sf::Vector2f c[4] = {{size.x - r, r}, {size.x - r, size.y - r}, {r, size.y - r}, {r, r}};
    for (int k = 0; k < 4; k++)
        for (int i = 0; i < N; i++) {
            float a = (k * 90.f - 90.f + 90.f * i / (N - 1)) * 3.14159265f / 180.f;
            s.setPoint(k * N + i, {c[k].x + r * std::cos(a), c[k].y + r * std::sin(a)});
        }
    return s;
}

class App {
public:
    App() : window(sf::VideoMode({1100, 760}), "Smart Parking Simulator", sf::Style::Titlebar | sf::Style::Close),
            sim(cfg), text(font) {
        window.setFramerateLimit(60);
        const char* fonts[] = {"C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
                               "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "arial.ttf"};
        for (const char* f : fonts)
            if (font.openFromFile(f)) { fontOk = true; break; }
        makeButtons();
    }

    int run() {
        if (!fontOk) {
            std::cerr << "Қаріп табылмады (arial.ttf)\n";
            return 1;
        }
        sf::Clock clock;
        while (window.isOpen()) {
            while (const std::optional event = window.pollEvent()) {
                if (event->is<sf::Event::Closed>())
                    window.close();
                else if (const auto* m = event->getIf<sf::Event::MouseButtonPressed>())
                    if (m->button == sf::Mouse::Button::Left)
                        click({(float)m->position.x, (float)m->position.y});
            }
            update(clock.restart().asSeconds());
            draw();
        }
        return 0;
    }

private:
    void makeButtons() {
        float x = PANEL_X, h = 34;
        auto add = [&](float bx, float by, float bw, std::function<std::string()> l, std::function<void()> f, sf::Color c) {
            buttons.push_back({sf::FloatRect({bx, by}, {bw, h}), l, f, c});
        };
        auto fixed = [](std::string s) { return [s] { return s; }; };
        sf::Color gray(62, 72, 96);

        add(x, 118, 96, fixed("Бастау"), [this] { running = !sim.finished(); setStatus("Жүріп жатыр"); }, GREEN);
        add(x + 106, 118, 96, fixed("Кідірту"), [this] { running = false; setStatus("Кідіртілді"); }, AMBER);
        add(x + 212, 118, 96, fixed("Қайта"), [this] { apply(cfg, "Қайта басталды"); }, gray);
        add(x, 160, PANEL_W, [this] { return "Стратегия: " + std::string(cfg.strategy == "nearest" ? "ең жақын орын" : "теңестіру"); },
            [this] {
                Config c = cfg;
                c.strategy = (c.strategy == "nearest") ? "balanced" : "nearest";
                apply(c, "Стратегия өзгерді");
            }, sf::Color(112, 84, 200));

        // Параметрлер: [-] және [+]
        float bx = x + PANEL_W - 76;
        add(bx, 232, 36, fixed("-"), [this] { changeCars(-1); }, gray);
        add(bx + 40, 232, 36, fixed("+"), [this] { changeCars(+1); }, gray);
        add(bx, 272, 36, fixed("-"), [this] { changeInterval(-0.5); }, gray);
        add(bx + 40, 272, 36, fixed("+"), [this] { changeInterval(+0.5); }, gray);
        add(bx, 312, 36, fixed("-"), [this] { if (speed > 1) speed /= 2; }, gray);
        add(bx + 40, 312, 36, fixed("+"), [this] { if (speed < 16) speed *= 2; }, gray);

        add(x, 360, 149, fixed("Конфиг сақтау"), [this] { saveConfig(); }, gray);
        add(x + 159, 360, 149, fixed("Конфиг жүктеу"), [this] { loadConfig(); }, gray);
        add(x, 400, PANEL_W, fixed("Нәтижені сақтау"), [this] { saveResults(); }, gray);
    }

    void changeCars(int dir) {
        const int options[] = {20, 50, 100};
        int i = 0;
        while (i < 2 && options[i] < cfg.cars) i++;
        i = std::max(0, std::min(2, i + dir));
        Config c = cfg;
        c.cars = options[i];
        apply(c, "Көліктер саны: " + std::to_string(c.cars));
    }

    void changeInterval(double d) {
        Config c = cfg;
        c.interval = std::max(0.5, std::min(5.0, c.interval + d));
        apply(c, "Келу аралығы: " + num(c.interval) + " с");
    }

    void apply(const Config& c, const std::string& msg) {
        try {
            sim.reset(c);
            cfg = c;
            running = false;
            setStatus(msg);
        } catch (const std::exception& e) {
            setStatus(std::string("Қате: ") + e.what(), true);
        }
    }

    void saveConfig() {
        try {
            cfg.save("config.txt");
            setStatus("config.txt сақталды");
        } catch (const std::exception& e) {
            setStatus(std::string("Қате: ") + e.what(), true);
        }
    }

    void loadConfig() {
        try {
            apply(Config::load("config.txt"), "config.txt жүктелді");
        } catch (const std::exception& e) {
            setStatus(std::string("Қате: ") + e.what(), true);
        }
    }

    void saveResults() {
        try {
            sim.saveResults("results.txt");
            setStatus("results.txt сақталды");
        } catch (const std::exception& e) {
            setStatus(std::string("Қате: ") + e.what(), true);
        }
    }

    void setStatus(const std::string& s, bool error = false) {
        status = s;
        statusError = error;
    }

    void click(sf::Vector2f p) {
        for (Button& b : buttons) {
            if (b.box.contains(p)) {
                b.onClick();
                return;
            }
        }
        // Картадағы бос орынды басу - брондау
        int cx = (int)((p.x - MAP_POS.x) / CELL);
        int cy = (int)((p.y - MAP_POS.y) / CELL);
        if (p.x < MAP_POS.x || p.y < MAP_POS.y || cy % 2 == 0 || cx < 1) return;
        int row = cy / 2, col = cx - 1;
        if (row >= cfg.rows || col >= cfg.cols) return;
        int id = row * cfg.cols + col;
        if (sim.reserveSpot(id))
            setStatus("Орын " + std::to_string(id) + " " + num(cfg.reserveTime, 0) + " с брондалды");
        else
            setStatus("Бұл орын бос емес", true);
    }

    void update(float frame) {
        if (!running) return;
        acc += std::min(frame, 0.1f) * (double)speed;
        while (acc >= STEP) {
            sim.step(STEP);
            acc -= STEP;
        }
        if (sim.finished()) {
            running = false;
            setStatus("Симуляция аяқталды");
        }
    }

    // --- Сурет салу көмекшілері ---

    void label(const std::string& s, float x, float y, unsigned size = 16, sf::Color c = TEXT) {
        text.setString(U(s));
        text.setCharacterSize(size);
        text.setFillColor(c);
        text.setPosition({x, y});
        window.draw(text);
    }

    // Мәтінді аймақтың ортасына (align=0) немесе оң жағына (align=1) қою
    void labelIn(const std::string& s, sf::FloatRect r, unsigned size, sf::Color c, int align = 0) {
        text.setString(U(s));
        text.setCharacterSize(size);
        sf::FloatRect b = text.getLocalBounds();
        float x = align == 0 ? r.position.x + (r.size.x - b.size.x) / 2 : r.position.x + r.size.x - b.size.x;
        label(s, x - b.position.x, r.position.y + (r.size.y - b.size.y) / 2 - b.position.y, size, c);
    }

    void box(float x, float y, float w, float h, float r, sf::Color fill,
             sf::Color outline = sf::Color::Transparent, float t = 0) {
        sf::ConvexShape s = rounded({w, h}, r);
        s.setPosition({x, y});
        s.setFillColor(fill);
        s.setOutlineColor(outline);
        s.setOutlineThickness(t);
        window.draw(s);
    }

    void drawMap() {
        const ParkingLot& lot = sim.getLot();
        float width = (lot.getCols() + 1) * CELL;
        float height = 2 * lot.getRows() * CELL;

        box(MAP_POS.x - 16, MAP_POS.y - 36, width + 32, height + 84, 14, CARD);
        box(MAP_POS.x, MAP_POS.y, CELL, height, 6, ROAD);
        for (int r = 0; r < lot.getRows(); r++) {
            float y = MAP_POS.y + 2 * r * CELL;
            box(MAP_POS.x, y, width, CELL, 6, ROAD);
            for (float x = MAP_POS.x + CELL + 10; x < MAP_POS.x + width - 20; x += 34)
                box(x, y + CELL / 2 - 1, 16, 2, 1, sf::Color(120, 124, 136));
        }
        box(MAP_POS.x, MAP_POS.y - 28, 96, 22, 11, sf::Color(46, 170, 100, 60));
        label("КІРУ / ШЫҒУ", MAP_POS.x + 10, MAP_POS.y - 26, 13, GREEN);

        for (int i = 0; i < lot.size(); i++) {
            const ParkingSpot& s = lot.spot(i);
            float x = MAP_POS.x + (s.getCol() + 1) * CELL + 3;
            float y = MAP_POS.y + (2 * s.getRow() + 1) * CELL + 3;
            float w = CELL - 6;

            sf::Color fill(34, 74, 54), border(70, 120, 90);
            if (s.getState() == SpotState::Occupied) { fill = sf::Color(78, 36, 44); border = sf::Color(130, 60, 70); }
            if (s.getState() == SpotState::Reserved) { fill = sf::Color(96, 74, 26); border = AMBER; }
            if (s.getKind() == Kind::Disabled) border = BLUE;
            if (s.getKind() == Kind::Electric) border = CYAN;
            box(x, y, w, w, 7, fill, border, s.getKind() == Kind::Normal ? 1.f : 2.f);

            if (s.getKind() == Kind::Disabled) label("М", x + 4, y + 1, 12, BLUE);
            if (s.getKind() == Kind::Electric) label("Э", x + 4, y + 1, 12, CYAN);
            if (s.getState() == SpotState::Reserved)
                labelIn(num(s.getReservedUntil() - sim.getTime(), 0), {{x, y}, {w, w}}, 15, AMBER);
        }

        float size = CELL * 0.56f;
        for (const Car& car : sim.getCars()) {
            CarState st = car.getState();
            if (st != CarState::Driving && st != CarState::Parked && st != CarState::Leaving) continue;
            Point p = car.getPos();
            float cx = MAP_POS.x + (p.x + 0.5f) * CELL;
            float cy = MAP_POS.y + (p.y + 0.5f) * CELL;
            sf::Color c = carColor(car);
            box(cx - size / 2, cy - size / 2 + 2, size, size, 7, sf::Color(0, 0, 0, 90)); // көлеңке
            box(cx - size / 2, cy - size / 2, size, size, 7, c, lighter(c, -60), 1);
            box(cx - size * 0.3f, cy - size * 0.22f, size * 0.6f, size * 0.42f, 4, sf::Color(25, 30, 42, 210));
        }

        // Шартты белгілер
        float ly = MAP_POS.y + height + 18;
        auto legend = [&](float lx, sf::Color c, const std::string& s) {
            box(lx, ly + 2, 14, 14, 4, c);
            label(s, lx + 20, ly, 14, MUTED);
        };
        legend(MAP_POS.x, sf::Color(34, 120, 74), "бос");
        legend(MAP_POS.x + 70, sf::Color(150, 60, 70), "бос емес");
        legend(MAP_POS.x + 170, AMBER, "бронь");
        legend(MAP_POS.x + 250, BLUE, "М - мүгедек");
        legend(MAP_POS.x + 375, CYAN, "Э - электр");
    }

    void drawPanel() {
        box(PANEL_X - 16, 80, PANEL_W + 32, 664, 14, CARD);
        sf::Vector2f mouse = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        label("БАСҚАРУ", PANEL_X, 94, 13, MUTED);
        label("ПАРАМЕТРЛЕР", PANEL_X, 208, 13, MUTED);
        for (const Button& b : buttons) {
            sf::Color c = b.box.contains(mouse) ? lighter(b.color, 25) : b.color;
            box(b.box.position.x, b.box.position.y, b.box.size.x, b.box.size.y, 8, c);
            labelIn(b.label(), b.box, 15, sf::Color::White);
        }
        label("Көліктер:  " + std::to_string(cfg.cars), PANEL_X, 238);
        label("Келу аралығы:  " + num(cfg.interval) + " с", PANEL_X, 278);
        label("Жылдамдық:  x" + std::to_string(speed), PANEL_X, 318);

        const Stats& st = sim.getStats();
        const ParkingLot& lot = sim.getLot();

        label("КӨРСЕТКІШТЕР", PANEL_X, 452, 13, MUTED);
        double occ = lot.occupancy();
        label("Толу", PANEL_X, 474, 15);
        labelIn(num(occ, 0) + "%", {{PANEL_X, 474}, {PANEL_W, 20}}, 15, TEXT, 1);
        box(PANEL_X, 500, PANEL_W, 10, 5, ROAD);
        sf::Color barColor = occ < 70 ? GREEN : (occ < 95 ? AMBER : RED);
        if (occ > 0) box(PANEL_X, 500, (float)(PANEL_W * occ / 100), 10, 5, barColor);

        float y = 522;
        auto row = [&](const std::string& k, const std::string& v) {
            label(k, PANEL_X, y, 15, MUTED);
            labelIn(v, {{PANEL_X, y}, {PANEL_W, 20}}, 15, TEXT, 1);
            y += 23;
        };
        row("Уақыт", num(sim.getTime()) + " с");
        row("Келді", std::to_string(st.arrived) + " / " + std::to_string(cfg.cars));
        row("Бос орын", std::to_string(lot.freeCount()) + " / " + std::to_string(lot.size()));
        row("Кезек", std::to_string(sim.queueSize()) + "  (макс " + std::to_string(st.maxQueue) + ")");
        row("Іздеу уақыты", num(st.avgSearchUs(), 2) + " мкс");
        row("Қаралған орын", num(st.avgChecked()));
        row("Орташа жол", num(st.avgDistance()));
        row("Брондан босады", std::to_string(st.reservationsFreed));

        // Кезектегі көліктер
        int shown = 0;
        for (const Car& car : sim.getCars()) {
            if (car.getState() != CarState::Waiting || car.getArriveTime() > sim.getTime()) continue;
            if (shown < 13) box(PANEL_X + shown * 24, y + 8, 18, 18, 5, carColor(car));
            shown++;
        }
    }

    void draw() {
        window.clear(BG);
        label("Smart Parking Simulator", 32, 16, 26);
        label("Ақылды автотұрақ", 340, 24, 18, MUTED);
        box(32, 54, 10, 10, 5, statusError ? RED : (running ? GREEN : AMBER));
        label(status, 50, 49, 15, statusError ? RED : MUTED);
        drawMap();
        drawPanel();
        window.display();
    }

    sf::RenderWindow window;
    Config cfg;
    Simulation sim;
    sf::Font font;
    sf::Text text;
    bool fontOk = false;
    std::vector<Button> buttons;
    bool running = false;
    int speed = 2;
    double acc = 0;
    std::string status = "Бастау батырмасын басыңыз. Бос орынды басса - брондалады.";
    bool statusError = false;
};

int main() {
    App app;
    return app.run();
}
