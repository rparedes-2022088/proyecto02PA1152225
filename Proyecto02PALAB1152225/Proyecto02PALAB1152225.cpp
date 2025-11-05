#include <iostream>
#include <vector>
#include <cstdlib>
#include <stack>
#include <queue>
#include <chrono>
#include <thread>
#include <string>

using namespace std;
using namespace chrono;

struct Pos { int x, y; };

struct Jugador {
    Pos pos;
    int vida = 100;
    int monedas = 0;
    int deshacerDisponibles = 2;
    stack<Pos> historial;
};

struct Celda { char tipo; };

class Laberinto {
public:
    int n;
    vector<vector<Celda>> mapa;
    Pos salida;

    Laberinto(int size = 7) {
        n = size;
        mapa.resize(n, vector<Celda>(n));
        generar();
    }

    void generar() {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                int r = rand() % 100;
                if (r < 25) mapa[i][j].tipo = '#';
                else if (r < 35) mapa[i][j].tipo = 'T';
                else if (r < 40) mapa[i][j].tipo = 'O';
                else mapa[i][j].tipo = '.';
            }
        }
        salida = { n - 1, n - 1 };
        mapa[salida.x][salida.y].tipo = 'S';
    }

    void mostrar() {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) cout << mapa[i][j].tipo << ' ';
            cout << endl;
        }
    }
};

bool dentro(int x, int y, int n) {
    return x >= 0 && y >= 0 && x < n && y < n;
}

bool esTransitable(char c) {
    return c != '#';
}

bool tieneCaminoR(Laberinto& l, int x, int y, vector<vector<bool>>& vis, Pos destino) {
    if (x == destino.x && y == destino.y)
        return true;

    vis[x][y] = true;

    int dx[] = { -1, 1, 0, 0 };
    int dy[] = { 0, 0, -1, 1 };

    for (int k = 0; k < 4; k++) {
        int nx = x + dx[k];
        int ny = y + dy[k];

        if (dentro(nx, ny, l.n) && !vis[nx][ny] && esTransitable(l.mapa[nx][ny].tipo)) {
            if (tieneCaminoR(l, nx, ny, vis, destino))
                return true;
        }
    }
    return false;
}

bool tieneCamino(Laberinto& l, Pos inicio, Pos destino) {
    vector<vector<bool>> vis(l.n, vector<bool>(l.n, false));
    return tieneCaminoR(l, inicio.x, inicio.y, vis, destino);
}

bool lineaDeVision(const Laberinto& l, Pos a, Pos b) {
    if (a.x == b.x) {
        int paso = (a.y < b.y) ? 1 : -1;
        for (int y = a.y + paso; y != b.y; y += paso)
            if (l.mapa[a.x][y].tipo == '#') return false;
        return true;
    }
    if (a.y == b.y) {
        int paso = (a.x < b.x) ? 1 : -1;
        for (int x = a.x + paso; x != b.x; x += paso)
            if (l.mapa[x][a.y].tipo == '#') return false;
        return true;
    }
    return false;
}

void despejarJugador(Laberinto& l, const Pos& p) {
    if (l.mapa[p.x][p.y].tipo == '#') l.mapa[p.x][p.y].tipo = '.';
    int dx[4] = { -1, 1, 0, 0 };
    int dy[4] = { 0, 0, -1, 1 };
    for (int k = 0; k < 4; k++) {
        int nx = p.x + dx[k];
        int ny = p.y + dy[k];
        if (dentro(nx, ny, l.n) && l.mapa[nx][ny].tipo == '#')
            l.mapa[nx][ny].tipo = '.';
    }
}

void abrirAgujerosHastaCamino(Laberinto& l, const Pos& s, const Pos& t) {
    int intentos = 0;
    while (!tieneCamino(l, s, t) && intentos < 2000) {
        int rx = rand() % l.n, ry = rand() % l.n;
        if ((rx == s.x && ry == s.y) || (rx == t.x && ry == t.y)) { intentos++; continue; }
        if (l.mapa[rx][ry].tipo == '#') l.mapa[rx][ry].tipo = '.';
        intentos++;
    }
}

void generarElementos(Laberinto& l, vector<Pos>& enemigos, const Pos& jugador) {
    enemigos.clear();
    int total = l.n * l.n;
    int enemigosNum = max(1, total / 10);
    int objetosNum = max(1, total * 15 / 100);

    int colocE = 0;
    while (colocE < enemigosNum) {
        int x = rand() % l.n, y = rand() % l.n;
        if ((x == jugador.x && y == jugador.y) || (x == l.salida.x && y == l.salida.y)) continue;
        if (l.mapa[x][y].tipo == '.' || l.mapa[x][y].tipo == 'O') {
            l.mapa[x][y].tipo = 'E';
            enemigos.push_back({ x, y });
            colocE++;
        }
    }

    int colocO = 0;
    while (colocO < objetosNum) {
        int x = rand() % l.n, y = rand() % l.n;
        if ((x == jugador.x && y == jugador.y) || (x == l.salida.x && y == l.salida.y)) continue;
        if (l.mapa[x][y].tipo == '.') {
            int t = rand() % 3;
            if (t == 0) l.mapa[x][y].tipo = 'P';
            else if (t == 1) l.mapa[x][y].tipo = 'M';
            else l.mapa[x][y].tipo = 'O';
            colocO++;
        }
    }
}

void asegurarMapaConCamino(Laberinto& lab, Jugador& j, vector<Pos>& enemigos) {
    if (j.pos.x >= lab.n) j.pos.x = lab.n - 1;
    if (j.pos.y >= lab.n) j.pos.y = lab.n - 1;
    despejarJugador(lab, j.pos);
    lab.mapa[lab.salida.x][lab.salida.y].tipo = 'S';
    abrirAgujerosHastaCamino(lab, j.pos, lab.salida);

    generarElementos(lab, enemigos, j.pos);
    lab.mapa[j.pos.x][j.pos.y].tipo = 'J';
    lab.mapa[lab.salida.x][lab.salida.y].tipo = 'S';
    while (!j.historial.empty()) j.historial.pop();
}

void aplicarPocion(Jugador& j, char tipo) {
    if (tipo == 'P' && j.vida <= 80) j.vida = min(100, j.vida + 20);
    else if (tipo == 'M' && j.vida <= 40) j.vida = min(100, j.vida + 60);
}

void moverJugador(Jugador& j, Laberinto& l, char dir) {
    int dx = 0, dy = 0;
    if (dir == 'W') dx = -1;
    else if (dir == 'S') dx = 1;
    else if (dir == 'A') dy = -1;
    else if (dir == 'D') dy = 1;

    int nx = j.pos.x + dx, ny = j.pos.y + dy;
    if (!dentro(nx, ny, l.n)) { cout << "No puedes salir del laberinto.\n"; return; }
    if (l.mapa[nx][ny].tipo == '#') { cout << "Hay una pared.\n"; return; }

    j.historial.push(j.pos);
    if (j.pos.x == l.salida.x && j.pos.y == l.salida.y) l.mapa[j.pos.x][j.pos.y].tipo = 'S';
    else l.mapa[j.pos.x][j.pos.y].tipo = '.';

    j.pos = { nx, ny };
    char t = l.mapa[nx][ny].tipo;
    if (t == 'T') { j.vida -= 20; cout << "Trampa activada (-20 vida)\n"; }
    else if (t == 'O') { j.monedas++; cout << "Encontraste un objeto (+1 moneda)\n"; }
    else if (t == 'P' || t == 'M') { aplicarPocion(j, t); }
    else if (t == 'E') { j.vida -= 20; cout << "¡Choque con enemigo! (-20 vida)\n"; }

    l.mapa[nx][ny].tipo = 'J';
}

void moverEnemigos(Laberinto& l, Jugador& j, vector<Pos>& enemigos) {
    vector<Pos> nuevos;
    for (auto e : enemigos) {
        if (l.mapa[e.x][e.y].tipo == 'E') l.mapa[e.x][e.y].tipo = '.';
        int dx = 0, dy = 0;
        int distX = abs(e.x - j.pos.x), distY = abs(e.y - j.pos.y);
        bool ve = (distX <= 3 && e.y == j.pos.y && lineaDeVision(l, e, j.pos)) ||
            (distY <= 3 && e.x == j.pos.x && lineaDeVision(l, e, j.pos));

        if (ve) {
            if (e.x < j.pos.x) dx = 1;
            else if (e.x > j.pos.x) dx = -1;
            else if (e.y < j.pos.y) dy = 1;
            else if (e.y > j.pos.y) dy = -1;
        }
        else {
            int dir = rand() % 4;
            if (dir == 0) dx = -1;
            else if (dir == 1) dx = 1;
            else if (dir == 2) dy = -1;
            else dy = 1;
        }

        int nx = e.x + dx, ny = e.y + dy;
        if (!dentro(nx, ny, l.n)) { nx = e.x; ny = e.y; }
        char destino = l.mapa[nx][ny].tipo;
        if (nx == l.salida.x && ny == l.salida.y) { nx = e.x; ny = e.y; destino = l.mapa[nx][ny].tipo; }

        if (destino == 'J') { j.vida -= 20; cout << "¡Un enemigo te ha atacado! (-20 vida)\n"; nx = e.x; ny = e.y; }
        if (destino != '#' && !(nx == l.salida.x && ny == l.salida.y) && destino != 'E') {
            l.mapa[nx][ny].tipo = 'E'; nuevos.push_back({ nx, ny });
        }
        else {
            l.mapa[e.x][e.y].tipo = 'E'; nuevos.push_back(e);
        }
    }
    enemigos = nuevos;
}

int main() {
    srand((unsigned)steady_clock::now().time_since_epoch().count());
    cout << "=== LABERINTO MAGICO (version DFS recursiva) ===\n";
    cout << "# = Pared\n. = Espacio libre\nT = Trampa (-20 vida)\nO = Objeto (+1 moneda)\n";
    cout << "P = Poción normal (+20 vida si ≤80)\nM = Poción maestra (+60 vida si ≤40)\n";
    cout << "E = Enemigo (-20 vida)\nS = Salida\nJ = Jugador\n\n";

    int cambioTiempo = 60;
    cout << "Ingresa segundos para cambiar el laberinto (45–200, Enter=60): ";
    string entrada; getline(cin, entrada);
    if (entrada != "") {
        int v = 0; bool ok = true;
        for (char c : entrada) { if (!isdigit(c)) { ok = false; break; } v = v * 10 + (c - '0'); }
        if (ok) cambioTiempo = v;
    }
    if (cambioTiempo < 45) cambioTiempo = 45;
    if (cambioTiempo > 200) cambioTiempo = 200;

    Laberinto lab(7 + rand() % 6);
    Jugador j; j.pos = { 0, 0 };
    vector<Pos> enemigos;
    asegurarMapaConCamino(lab, j, enemigos);
    auto ultimoCambio = steady_clock::now();

    while (true) {
        int tiempoRestante = cambioTiempo - (int)duration_cast<seconds>(steady_clock::now() - ultimoCambio).count();
        if (tiempoRestante <= 0) {
            cout << "\n*** El laberinto ha cambiado ***\n";
            lab = Laberinto(7 + rand() % 6);
            asegurarMapaConCamino(lab, j, enemigos);
            ultimoCambio = steady_clock::now();
        }

        cout << "\nVida: " << j.vida << " | Monedas: " << j.monedas
            << " | Cambio en: " << tiempoRestante << "s | Deshacer: " << j.deshacerDisponibles << "\n";
        lab.mostrar();
        cout << "Mover (W/A/S/D), U=deshacer: ";
        char op; cin >> op; op = toupper(op);

        if (op == 'U') {
            if (j.deshacerDisponibles > 0 && !j.historial.empty()) {
                if (j.pos.x == lab.salida.x && j.pos.y == lab.salida.y) lab.mapa[j.pos.x][j.pos.y].tipo = 'S';
                else lab.mapa[j.pos.x][j.pos.y].tipo = '.';
                j.pos = j.historial.top(); j.historial.pop();
                j.deshacerDisponibles--;
                lab.mapa[j.pos.x][j.pos.y].tipo = 'J';
                cout << "Movimiento deshecho. Te quedan " << j.deshacerDisponibles << " intento(s).\n";
            }
            else cout << "Ya no puedes deshacer más movimientos.\n";
        }
        else moverJugador(j, lab, op);

        moverEnemigos(lab, j, enemigos);

        if (j.vida <= 0) { cout << "Te has quedado sin vida.\n"; break; }
        if (j.pos.x == lab.salida.x && j.pos.y == lab.salida.y) { cout << "¡Ganaste!\n"; break; }

        this_thread::sleep_for(milliseconds(350));
    }

    cout << "\nJuego terminado. Puntaje final: " << j.monedas * 10 << "\n";
    return 0;
}