#ifndef TETRISENGINE_H
#define TETRISENGINE_H

#include "tetrisconstants.h"
#include <QObject>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <random>

class TetrisEngine : public QObject {
    Q_OBJECT

public:
    explicit TetrisEngine(QObject *parent = nullptr);
    ~TetrisEngine() override = default;

    // Game lifecycle controls
    void startGame();
    void pauseGame();
    void resumeGame();
    void togglePause();
    void restartGame();
    void resetHighScore();

    // Player inputs
    bool moveLeft();
    bool moveRight();
    bool rotateClockwise();
    bool rotateCounterClockwise();
    bool softDrop();
    void hardDrop();
    bool holdPiece();

    // Getters for rendering & state inspection
    Tetris::GameState gameState() const { return m_state; }
    int score() const { return m_score; }
    int highScore() const { return m_highScore; }
    int level() const { return m_level; }
    int linesCleared() const { return m_linesCleared; }
    int comboCount() const { return m_combo; }

    int getCell(int visibleCol, int visibleRow) const;
    Tetris::TetrominoType currentPieceType() const { return m_currentPiece; }
    Tetris::TetrominoType heldPieceType() const { return m_holdPiece; }
    bool canHold() const { return m_canHold; }
    QVector<Tetris::TetrominoType> nextQueue() const { return m_nextQueue; }

    // Coordinates of the active piece in visible space
    QVector<QPoint> getCurrentPieceVisibleBlocks() const;
    // Coordinates of the ghost piece in visible space
    QVector<QPoint> getGhostPieceVisibleBlocks() const;

    const QVector<int>& clearingRows() const { return m_clearingRows; }
    int ghostDropDistance() const;

signals:
    void boardChanged();
    void scoreChanged(int score, int highScore);
    void levelChanged(int level);
    void linesChanged(int lines);
    void nextPieceChanged(const QVector<Tetris::TetrominoType> &nextPieces);
    void holdPieceChanged(Tetris::TetrominoType piece, bool canHold);
    void gameStateChanged(Tetris::GameState newState);
    void lineClearAnimationStarted(const QVector<int> &visibleRows);
    void floatingBanner(const QString &text, const QColor &color);

private slots:
    void onDropTimerTick();
    void onLockDelayTick();
    void onClearAnimationFinished();

private:
    void initBoard();
    void fillBag();
    Tetris::TetrominoType takeNextPiece();
    bool spawnPiece(Tetris::TetrominoType type);
    bool checkCollision(const QVector<QPoint> &blocks, int testX, int testY) const;
    QVector<QPoint> rotateShape(const QVector<QPoint> &blocks, int oldRot, int newRot, int bbSize) const;
    int calculateGhostY() const;
    void lockActivePiece();
    void checkLineClears();
    void finalizeLineClear();
    void updateDropInterval();
    void loadHighScore();
    void saveHighScore();

    // Board storage: rows 0..(BUFFER_HEIGHT-1) are buffer rows
    // rows BUFFER_HEIGHT..(TOTAL_HEIGHT-1) are visible rows
    int m_board[Tetris::TOTAL_HEIGHT][Tetris::BOARD_WIDTH];

    Tetris::GameState m_state = Tetris::GameState::Ready;
    Tetris::TetrominoType m_currentPiece = Tetris::TetrominoType::None;
    int m_pieceX = 0;
    int m_pieceY = 0;
    int m_pieceRotation = 0;
    QVector<QPoint> m_pieceBlocks;

    Tetris::TetrominoType m_holdPiece = Tetris::TetrominoType::None;
    bool m_canHold = true;

    QVector<Tetris::TetrominoType> m_bag;
    QVector<Tetris::TetrominoType> m_nextQueue;
    std::mt19937 m_rng;

    int m_score = 0;
    int m_highScore = 0;
    int m_level = 1;
    int m_linesCleared = 0;
    int m_combo = -1;
    bool m_backToBackTetris = false;

    // Timers
    QTimer *m_dropTimer = nullptr;
    QTimer *m_lockTimer = nullptr;
    QTimer *m_clearAnimTimer = nullptr;
    bool m_isLocking = false;
    int m_lockMoveResets = 0;
    static constexpr int MAX_LOCK_RESETS = 15;

    QVector<int> m_clearingRows; // Visible rows currently flashing
};

#endif // TETRISENGINE_H
