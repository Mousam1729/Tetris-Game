#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QPen>
#include <QFont>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <random>

// ---------------- Tetromino MODEL data ----------------
// Each piece = 4 cells (x, y) around a pivot at the model origin (0,0), y pointing UP.
// Order: I, O, T, S, Z, J, L
static const int SHAPES[7][4][2] = {
    {{-1, 0}, {0, 0}, {1, 0}, {2, 0}},     // I
    {{0, 0}, {1, 0}, {0, 1}, {1, 1}},      // O
    {{-1, 0}, {0, 0}, {1, 0}, {0, 1}},     // T
    {{-1, 0}, {0, 0}, {0, 1}, {1, 1}},     // S
    {{-1, 1}, {0, 1}, {0, 0}, {1, 0}},     // Z
    {{-1, 1}, {-1, 0}, {0, 0}, {1, 0}},    // J
    {{-1, 0}, {0, 0}, {1, 0}, {1, 1}}      // L
};

static std::mt19937 &rng()
{
    static std::mt19937 gen{std::random_device{}()};
    return gen;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    gridSize(25),
    sc_x(0), sc_y(0),
    nextType(0),
    score(0), lines(0), level(1),
    paused(false), gameOver(false),
    timer(nullptr)
{
    ui->setupUi(this);

    // Keyboard must always reach the main window, so no child widget may keep focus.
    // (The spin box / buttons still work with the mouse.)
    setFocusPolicy(Qt::StrongFocus);
    const QList<QWidget *> kids = findChildren<QWidget *>();
    for (QWidget *w : kids) w->setFocusPolicy(Qt::NoFocus);
    setFocus();

    gridSize = ui->spinBox->value();
    if (gridSize <= 0) gridSize = 25;

    ui->label_help->setText(
        "Controls\n"
        "Left / Right  (A / D) : translate\n"
        "Up (W) or Left-click : rotate 90\n"
        "F : flip (reflection)\n"
        "Down  (S) : soft drop\n"
        "Space : hard drop\n"
        "P : pause     N : new game");

    // Hover coordinates + click-to-rotate (signals of my_label)
    connect(ui->frame, &my_label::sendMousePosition, this, &MainWindow::showMousePosition);
    connect(ui->frame, &my_label::Mouse_Pos, this, &MainWindow::Mouse_Pressed);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::gameTick);

    newGame();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ---------------- Mouse ----------------

void MainWindow::showMousePosition(QPoint &pos)
{
    int width = ui->frame->width();
    int height = ui->frame->height();
    int centerX = (width / 2 / gridSize) * gridSize;
    int centerY = (height / 2 / gridSize) * gridSize;

    sc_x = static_cast<int>(std::floor((pos.x() - centerX) * 1.0 / gridSize));
    sc_y = static_cast<int>(std::floor((centerY - pos.y()) * 1.0 / gridSize));
    ui->mouse_movement->setText("Hover: X: " + QString::number(sc_x) + ", Y: " + QString::number(sc_y));
}

// Every left click on the canvas rotates the falling piece by 90 degrees
void MainWindow::Mouse_Pressed()
{
    if (paused || gameOver) return;
    tryRotate();
    redraw();
}

// ---------------- Game logic ----------------

void MainWindow::newGame()
{
    for (int r = 0; r < BOARD_H; ++r)
        for (int c = 0; c < BOARD_W; ++c)
            board[r][c] = 0;

    score = 0;
    lines = 0;
    level = 1;
    paused = false;
    gameOver = false;
    bag.clear();
    ui->btn_pause->setText("Pause (P)");

    nextType = takeFromBag();
    spawnPiece();
    updateTimer();
    timer->start();
    updateLabels();
    redraw();
}

// 7-bag: every 7 pieces contain each tetromino exactly once
int MainWindow::takeFromBag()
{
    if (bag.empty()) {
        for (int i = 0; i < 7; ++i) bag.push_back(i);
        std::shuffle(bag.begin(), bag.end(), rng());
    }
    int t = bag.back();
    bag.pop_back();
    return t;
}

// SPAWN = translate the model (pivot at origin) to the top-middle of the well:  M = T(4, 18)
void MainWindow::spawnPiece()
{
    cur.type = nextType;
    cur.M = Mat3::translation(BOARD_W / 2 - 1, BOARD_H - 2);
    nextType = takeFromBag();

    if (!isValid(cur)) {          // no room for the new piece -> game over
        gameOver = true;
        timer->stop();
    }
}

// Board cells of a piece:  every model cell (x, y) is multiplied by the matrix M
std::array<QPoint, 4> MainWindow::cellsOf(const Piece &p) const
{
    std::array<QPoint, 4> out;
    for (int i = 0; i < 4; ++i)
        out[i] = p.M.apply(SHAPES[p.type][i][0], SHAPES[p.type][i][1]);
    return out;
}

// The model origin (0,0) is the pivot; its image under M is the translation column of M
QPoint MainWindow::pivotOf(const Piece &p) const
{
    return p.M.apply(0, 0);
}

bool MainWindow::isValid(const Piece &p) const
{
    for (const QPoint &q : cellsOf(p)) {
        if (q.x() < 0 || q.x() >= BOARD_W || q.y() < 0) return false;
        if (q.y() < BOARD_H && board[q.y()][q.x()] != 0) return false;
    }
    return true;
}

// TRANSLATION:  M' = T(dx, dy) * M
bool MainWindow::tryMove(int dx, int dy)
{
    Piece p = cur;
    p.M = Mat3::translation(dx, dy) * cur.M;
    if (!isValid(p)) return false;
    cur = p;
    return true;
}

// ROTATION by 90 degrees clockwise (= -90 deg, y up) about the piece's pivot:
//      M' = [ T(p) * R(-90) * T(-p) ] * M
// If the turned piece does not fit, try shifting it left/right first (wall kick),
// which is just one more translation concatenated in front:  T(k, 0) * R_about * M
void MainWindow::tryRotate()
{
    if (cur.type == 1) return;                 // O piece: a square looks the same rotated

    QPoint pv = pivotOf(cur);
    Mat3 R = Mat3::rotationAbout(-90.0, pv.x(), pv.y());

    const int kicks[5] = {0, -1, 1, -2, 2};
    for (int k : kicks) {
        Piece p = cur;
        p.M = Mat3::translation(k, 0) * R * cur.M;
        if (isValid(p)) {
            cur = p;
            return;
        }
    }
}

// REFLECTION (flip) about the vertical line x = pivot.x :
//      M' = [ T(p) * S(-1, 1) * T(-p) ] * M       (turns an L into a J, an S into a Z ...)
void MainWindow::tryFlip()
{
    if (cur.type == 1) return;                 // O piece: nothing to flip

    QPoint pv = pivotOf(cur);
    Mat3 F = Mat3::reflectionAboutX(pv.x());

    const int kicks[5] = {0, -1, 1, -2, 2};
    for (int k : kicks) {
        Piece p = cur;
        p.M = Mat3::translation(k, 0) * F * cur.M;
        if (isValid(p)) {
            cur = p;
            return;
        }
    }
}

// HARD DROP = repeated translation T(0, -1) until the piece collides
void MainWindow::hardDrop()
{
    int dropped = 0;
    while (tryMove(0, -1)) ++dropped;
    score += 2 * dropped;
    lockPiece();
}

void MainWindow::lockPiece()
{
    for (const QPoint &q : cellsOf(cur))
        if (q.y() >= 0 && q.y() < BOARD_H)
            board[q.y()][q.x()] = cur.type + 1;

    int cleared = clearLines();
    if (cleared > 0) {
        static const int table[5] = {0, 100, 300, 500, 800};
        score += table[cleared] * level;
        lines += cleared;
        level = lines / 10 + 1;
        updateTimer();
    }
    spawnPiece();
    updateLabels();
}

// Removes every full row; rows above fall down (translation of the stack by one row).
int MainWindow::clearLines()
{
    int cleared = 0;
    int r = 0;
    while (r < BOARD_H) {
        bool full = true;
        for (int c = 0; c < BOARD_W; ++c)
            if (board[r][c] == 0) { full = false; break; }

        if (full) {
            for (int rr = r; rr < BOARD_H - 1; ++rr)
                for (int c = 0; c < BOARD_W; ++c)
                    board[rr][c] = board[rr + 1][c];
            for (int c = 0; c < BOARD_W; ++c)
                board[BOARD_H - 1][c] = 0;
            ++cleared;            // same r again: a new row has just fallen into it
        } else {
            ++r;
        }
    }
    return cleared;
}

void MainWindow::updateTimer()
{
    timer->setInterval(std::max(100, 800 - (level - 1) * 70));   // ms per gravity step
}

void MainWindow::updateLabels()
{
    ui->label_score->setText("Score: " + QString::number(score));
    ui->label_lines->setText("Lines: " + QString::number(lines));
    ui->label_level->setText("Level: " + QString::number(level));
}

void MainWindow::gameTick()
{
    if (paused || gameOver) return;
    if (!tryMove(0, -1)) lockPiece();      // gravity = translation T(0, -1)
    redraw();
}

// ---------------- Keyboard ----------------

void MainWindow::keyPressEvent(QKeyEvent *ev)
{
    switch (ev->key()) {
    case Qt::Key_N:
        newGame();
        return;
    case Qt::Key_P:
        on_btn_pause_clicked();
        return;
    default:
        break;
    }

    if (paused || gameOver) {
        QMainWindow::keyPressEvent(ev);
        return;
    }

    switch (ev->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        tryMove(-1, 0);
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        tryMove(1, 0);
        break;
    case Qt::Key_Up:
    case Qt::Key_W:
        tryRotate();
        break;
    case Qt::Key_F:
        tryFlip();
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if (tryMove(0, -1)) {
            score += 1;
            updateLabels();
        } else {
            lockPiece();
        }
        break;
    case Qt::Key_Space:
        hardDrop();
        break;
    default:
        QMainWindow::keyPressEvent(ev);
        return;
    }
    redraw();
}

// ---------------- Grid & drawing functions ----------------

QRgb MainWindow::pieceColor(int type) const
{
    switch (type) {
    case 0: return qRgb(0, 220, 230);     // I - cyan
    case 1: return qRgb(240, 220, 0);     // O - yellow
    case 2: return qRgb(170, 60, 200);    // T - purple
    case 3: return qRgb(0, 200, 60);      // S - green
    case 4: return qRgb(230, 30, 30);     // Z - red
    case 5: return qRgb(40, 80, 230);     // J - blue
    case 6: return qRgb(250, 140, 0);     // L - orange
    default: return qRgb(0, 0, 0);
    }
}

// Fills ONE grid cell given its logical (lx, ly).
// (The first pixel row/column of the cell is left alone so the grid lines stay visible.)
void MainWindow::colorGridPixelLogical(QImage &img, int lx, int ly, QRgb color) {
    int width = img.width();
    int height = img.height();
    int centerX = (width / 2 / gridSize) * gridSize;
    int centerY = (height / 2 / gridSize) * gridSize;
    int screenX = centerX + lx * gridSize;
    int screenY = centerY - ly * gridSize - gridSize;

    for (int i = 1; i < gridSize; i++) {
        for (int j = 1; j < gridSize; j++) {
            if (screenX + i >= 0 && screenY + j >= 0 && screenX + i < width && screenY + j < height) {
                img.setPixel(screenX + i, screenY + j, color);
            }
        }
    }
}

// Board cell (c, r) -> logical grid cell by the translation matrix T(X0, Y0)
void MainWindow::drawBoardCell(QImage &img, int c, int r, QRgb color)
{
    static const Mat3 boardToLogical = Mat3::translation(X0, Y0);
    QPoint l = boardToLogical.apply(c, r);
    colorGridPixelLogical(img, l.x(), l.y(), color);
}

void MainWindow::drawGrid(QPainter &painter, int frameWidth, int frameHeight)
{
    int centerX = (frameWidth / 2 / gridSize) * gridSize;
    int centerY = (frameHeight / 2 / gridSize) * gridSize;

    // Axes
    painter.fillRect(centerX, 0, gridSize, frameHeight, QColor(255, 0, 0, 100));
    painter.fillRect(0, centerY - gridSize, frameWidth, gridSize, QColor(255, 0, 0, 100));

    // Grid lines
    painter.setPen(QPen(QColor(200, 200, 200), 1));
    for (int x = 0; x <= frameWidth; x += gridSize) painter.drawLine(x, 0, x, frameHeight);
    for (int y = 0; y <= frameHeight; y += gridSize) painter.drawLine(0, y, frameWidth, y);
}

// Next-piece preview = model -> screen pipeline made of matrices:
//      V = T(ox, oy) * S(cell, -cell) * T(-minX, -maxY)
//   T(-minX,-maxY) : move the shape's top-left corner to the origin
//   S(cell, -cell) : SCALE up to pixels (the negative sy flips y: shape is y-up, screen is y-down)
//   T(ox, oy)      : TRANSLATE to the centre of the preview box
void MainWindow::drawNextPreview()
{
    const int cell = 20;
    QPixmap pm(ui->label_next->width(), ui->label_next->height());
    pm.fill(Qt::white);
    QPainter p(&pm);

    int minX = 99, maxX = -99, minY = 99, maxY = -99;
    for (int i = 0; i < 4; ++i) {
        minX = std::min(minX, SHAPES[nextType][i][0]);
        maxX = std::max(maxX, SHAPES[nextType][i][0]);
        minY = std::min(minY, SHAPES[nextType][i][1]);
        maxY = std::max(maxY, SHAPES[nextType][i][1]);
    }
    int ox = (pm.width()  - (maxX - minX + 1) * cell) / 2;
    int oy = (pm.height() - (maxY - minY + 1) * cell) / 2;

    Mat3 V = Mat3::translation(ox, oy) * Mat3::scaling(cell, -cell) * Mat3::translation(-minX, -maxY);

    QColor col = QColor::fromRgb(pieceColor(nextType));
    for (int i = 0; i < 4; ++i) {
        QPoint s = V.apply(SHAPES[nextType][i][0], SHAPES[nextType][i][1]);
        p.fillRect(s.x() + 1, s.y() + 1, cell - 1, cell - 1, col);
    }
    p.end();
    ui->label_next->setPixmap(pm);
}

// Redraws everything from scratch: grid -> walls -> well -> locked blocks -> ghost -> piece.
void MainWindow::redraw()
{
    int w = ui->frame->width();
    int h = ui->frame->height();
    int centerX = (w / 2 / gridSize) * gridSize;
    int centerY = (h / 2 / gridSize) * gridSize;

    QPixmap pix(w, h);
    pix.fill(Qt::white);
    {
        QPainter painter(&pix);
        drawGrid(painter, w, h);
    }

    QImage img = pix.toImage().convertToFormat(QImage::Format_ARGB32);

    // Walls (left, right, bottom)
    QRgb wall = qRgb(90, 90, 100);
    for (int r = -1; r < BOARD_H; ++r) {
        drawBoardCell(img, -1, r, wall);
        drawBoardCell(img, BOARD_W, r, wall);
    }
    for (int c = -1; c <= BOARD_W; ++c)
        drawBoardCell(img, c, -1, wall);

    // Well background + locked blocks
    for (int r = 0; r < BOARD_H; ++r)
        for (int c = 0; c < BOARD_W; ++c)
            drawBoardCell(img, c, r, board[r][c] ? pieceColor(board[r][c] - 1) : qRgb(28, 28, 38));

    if (!gameOver) {
        // Ghost piece: the current piece translated down by T(0,-1) until it would collide
        Piece g = cur;
        for (;;) {
            Piece t = g;
            t.M = Mat3::translation(0, -1) * g.M;
            if (!isValid(t)) break;
            g = t;
        }
        if (pivotOf(g).y() != pivotOf(cur).y())
            for (const QPoint &q : cellsOf(g))
                if (q.y() < BOARD_H) drawBoardCell(img, q.x(), q.y(), qRgb(85, 85, 105));

        // Current piece
        for (const QPoint &q : cellsOf(cur))
            if (q.y() < BOARD_H) drawBoardCell(img, q.x(), q.y(), pieceColor(cur.type));
    }

    QPixmap out = QPixmap::fromImage(img);

    // Overlay text for pause / game over
    if (paused || gameOver) {
        QPainter tp(&out);
        QRect well(centerX + X0 * gridSize, centerY - (Y0 + BOARD_H) * gridSize,
                   BOARD_W * gridSize, BOARD_H * gridSize);
        tp.fillRect(well, QColor(0, 0, 0, 150));
        QFont f = tp.font();
        f.setPointSize(16);
        f.setBold(true);
        tp.setFont(f);
        tp.setPen(Qt::white);
        QString msg = gameOver ? "GAME OVER\nPress N" : "PAUSED\nPress P";
        tp.drawText(well, Qt::AlignCenter, msg);
    }

    ui->frame->setPixmap(out);
    drawNextPreview();
}

// ---------------- UI Slots ----------------

void MainWindow::on_clear_clicked() {        // "New Game"
    newGame();
}

void MainWindow::on_btn_pause_clicked()
{
    if (gameOver) return;
    paused = !paused;
    if (paused) timer->stop(); else timer->start();
    ui->btn_pause->setText(paused ? "Resume (P)" : "Pause (P)");
    redraw();
}

void MainWindow::on_btn_draw_grid_clicked() {
    on_spinBox_valueChanged(ui->spinBox->value());
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if (arg1 <= 0) return;
    gridSize = arg1;         // the game keeps running; only the cell size changes
    redraw();
}