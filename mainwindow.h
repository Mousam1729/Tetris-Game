#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "tetrisconstants.h"
#include <QMainWindow>
#include <QKeyEvent>

namespace Ui {
class MainWindow;
}

class TetrisEngine;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onScoreChanged(int score, int highScore);
    void onLevelChanged(int level);
    void onLinesChanged(int lines);
    void onGameStateChanged(Tetris::GameState state);
    void onHoldPieceChanged(Tetris::TetrominoType piece, bool canHold);
    void onNextPieceChanged(const QVector<Tetris::TetrominoType> &nextPieces);
    void onMouseMovedOnGrid(int col, int row, int cartX, int cartY, int screenX, int screenY);

    // UI button slots
    void on_btnStart_clicked();
    void on_btnPause_clicked();
    void on_btnRestart_clicked();
    void on_btnResetHigh_clicked();

    // Visual settings slots
    void on_spinCellSize_valueChanged(int val);
    void on_checkGridLines_toggled(bool checked);
    void on_checkGhostPiece_toggled(bool checked);
    void on_comboBlockStyle_currentIndexChanged(int index);

private:
    void setupUiConnections();
    void updateStatusText(const QString &text);

    Ui::MainWindow *ui;
    TetrisEngine *m_engine;
};

#endif // MAINWINDOW_H
