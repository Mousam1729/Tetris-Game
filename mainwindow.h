#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPoint>
#include <QImage>
#include <QPainter>
#include <QTimer>
#include <QKeyEvent>
#include <array>
#include <vector>
#include <cmath>
#include "my_label.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// ============================================================================
//  2D transformations in HOMOGENEOUS coordinates (3x3 matrices)
//
//      | x' |   | a  b  tx |   | x |
//      | y' | = | c  d  ty | * | y |
//      | 1  |   | 0  0  1  |   | 1 |
//
//  Every operation in the game (moving, rotating, mirroring, spawning, the
//  preview scaling ...) is built from these matrices and CONCATENATED by
//  matrix multiplication.
// ============================================================================
struct Mat3
{
    double m[3][3];

    Mat3()                                   // identity
    {
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                m[i][j] = (i == j) ? 1.0 : 0.0;
    }

    // Translation T(tx, ty)
    static Mat3 translation(double tx, double ty)
    {
        Mat3 t;
        t.m[0][2] = tx;
        t.m[1][2] = ty;
        return t;
    }

    // Rotation R(theta) about the ORIGIN (counter-clockwise for +theta, y up)
    static Mat3 rotation(double degrees)
    {
        const double PI = 3.14159265358979323846;
        double rad = degrees * PI / 180.0;
        double c = std::cos(rad), s = std::sin(rad);
        Mat3 r;
        r.m[0][0] = c;  r.m[0][1] = -s;
        r.m[1][0] = s;  r.m[1][1] =  c;
        return r;
    }

    // Scaling S(sx, sy) about the origin  (negative value = reflection)
    static Mat3 scaling(double sx, double sy)
    {
        Mat3 s;
        s.m[0][0] = sx;
        s.m[1][1] = sy;
        return s;
    }

    // Rotation about an arbitrary pivot (px, py):  T(p) * R(theta) * T(-p)
    static Mat3 rotationAbout(double degrees, double px, double py)
    {
        return translation(px, py) * rotation(degrees) * translation(-px, -py);
    }

    // Reflection about the vertical line x = px:  T(p) * S(-1, 1) * T(-p)
    static Mat3 reflectionAboutX(double px)
    {
        return translation(px, 0) * scaling(-1, 1) * translation(-px, 0);
    }

    // Matrix concatenation  (this * o): o is applied FIRST, then this
    Mat3 operator*(const Mat3 &o) const
    {
        Mat3 r;
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j) {
                r.m[i][j] = 0;
                for (int k = 0; k < 3; ++k)
                    r.m[i][j] += m[i][k] * o.m[k][j];
            }
        return r;
    }

    // Transform the point (x, y, 1) and round to the nearest grid cell
    QPoint apply(double x, double y) const
    {
        double nx = m[0][0] * x + m[0][1] * y + m[0][2];
        double ny = m[1][0] * x + m[1][1] * y + m[1][2];
        return QPoint(static_cast<int>(std::lround(nx)), static_cast<int>(std::lround(ny)));
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *ev) override;

private slots:
    void showMousePosition(QPoint &pos);
    void Mouse_Pressed();                    // left click on canvas = rotate piece
    void on_clear_clicked();                 // "New Game"
    void on_btn_draw_grid_clicked();
    void on_btn_pause_clicked();
    void on_spinBox_valueChanged(int arg1);
    void gameTick();                         // gravity: called by the timer

private:
    Ui::MainWindow *ui;

    int gridSize;
    int sc_x, sc_y;          // logical coordinates under the mouse

    // ---------------- Tetris state ----------------
    // The well is BOARD_W x BOARD_H cells. Board cell (c, r): c = 0..9 (left->right),
    // r = 0..19 (bottom->top). It is mapped onto the logical grid (origin in the middle
    // of the canvas, y up) by the translation  T(X0, Y0):  lx = c + X0,  ly = r + Y0.
    static const int BOARD_W = 10;
    static const int BOARD_H = 20;
    static const int X0 = -5;
    static const int Y0 = -10;

    // A piece = its MODEL shape (4 cells around a pivot at the origin)
    //           + ONE transformation matrix M that carries the model into the board.
    // Moving / rotating / mirroring the piece only changes M:  M' = Transform * M
    struct Piece { int type; Mat3 M; };

    int board[BOARD_H][BOARD_W];   // 0 = empty, otherwise (piece type + 1)
    Piece cur;
    int nextType;
    std::vector<int> bag;          // 7-bag randomiser

    int score, lines, level;
    bool paused, gameOver;
    QTimer *timer;

    // game logic
    void newGame();
    int  takeFromBag();
    void spawnPiece();
    std::array<QPoint, 4> cellsOf(const Piece &p) const;   // M applied to every model cell
    QPoint pivotOf(const Piece &p) const;                  // image of the model origin
    bool isValid(const Piece &p) const;
    bool tryMove(int dx, int dy);        // translation
    void tryRotate();                    // rotation about the pivot (-90 deg = clockwise)
    void tryFlip();                      // reflection about the vertical axis through the pivot
    void hardDrop();
    void lockPiece();
    int  clearLines();
    void updateTimer();
    void updateLabels();

    // drawing (same grid helpers as before)
    QRgb pieceColor(int type) const;
    void drawGrid(QPainter &painter, int w, int h);
    void colorGridPixelLogical(QImage &img, int lx, int ly, QRgb color);
    void drawBoardCell(QImage &img, int c, int r, QRgb color);
    void drawNextPreview();
    void redraw();                                   // redraws the WHOLE scene
};

#endif // MAINWINDOW_H