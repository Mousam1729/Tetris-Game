#include "tetrisengine.h"
#include <QSettings>
#include <algorithm>
#include <chrono>

TetrisEngine::TetrisEngine(QObject *parent)
    : QObject(parent),
      m_rng(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()))
{
    m_dropTimer = new QTimer(this);
    connect(m_dropTimer, &QTimer::timeout, this, &TetrisEngine::onDropTimerTick);

    m_lockTimer = new QTimer(this);
    m_lockTimer->setSingleShot(true);
    connect(m_lockTimer, &QTimer::timeout, this, &TetrisEngine::onLockDelayTick);

    m_clearAnimTimer = new QTimer(this);
    m_clearAnimTimer->setSingleShot(true);
    connect(m_clearAnimTimer, &QTimer::timeout, this, &TetrisEngine::onClearAnimationFinished);

    loadHighScore();
    initBoard();
}

void TetrisEngine::loadHighScore()
{
    QSettings settings("CGLab", "TetrisProject");
    m_highScore = settings.value("highScore", 0).toInt();
}

void TetrisEngine::saveHighScore()
{
    QSettings settings("CGLab", "TetrisProject");
    settings.setValue("highScore", m_highScore);
}

void TetrisEngine::resetHighScore()
{
    m_highScore = 0;
    saveHighScore();
    emit scoreChanged(m_score, m_highScore);
}

void TetrisEngine::initBoard()
{
    for (int r = 0; r < Tetris::TOTAL_HEIGHT; ++r) {
        for (int c = 0; c < Tetris::BOARD_WIDTH; ++c) {
            m_board[r][c] = 0;
        }
    }
}

int TetrisEngine::getCell(int visibleCol, int visibleRow) const
{
    if (visibleCol < 0 || visibleCol >= Tetris::BOARD_WIDTH) return 0;
    if (visibleRow < 0 || visibleRow >= Tetris::BOARD_HEIGHT) return 0;

    int totalRow = visibleRow + Tetris::BUFFER_HEIGHT;
    return m_board[totalRow][visibleCol];
}

void TetrisEngine::fillBag()
{
    QVector<Tetris::TetrominoType> pieces = {
        Tetris::TetrominoType::I,
        Tetris::TetrominoType::J,
        Tetris::TetrominoType::L,
        Tetris::TetrominoType::O,
        Tetris::TetrominoType::S,
        Tetris::TetrominoType::T,
        Tetris::TetrominoType::Z
    };
    std::shuffle(pieces.begin(), pieces.end(), m_rng);
    m_bag.append(pieces);
}

Tetris::TetrominoType TetrisEngine::takeNextPiece()
{
    while (m_nextQueue.size() < 5) {
        if (m_bag.isEmpty()) {
            fillBag();
        }
        m_nextQueue.append(m_bag.takeFirst());
    }
    Tetris::TetrominoType next = m_nextQueue.takeFirst();
    emit nextPieceChanged(m_nextQueue);
    return next;
}

void TetrisEngine::startGame()
{
    m_dropTimer->stop();
    m_lockTimer->stop();
    m_clearAnimTimer->stop();

    initBoard();
    m_bag.clear();
    m_nextQueue.clear();

    m_score = 0;
    m_level = 1;
    m_linesCleared = 0;
    m_combo = -1;
    m_backToBackTetris = false;
    m_holdPiece = Tetris::TetrominoType::None;
    m_canHold = true;
    m_isLocking = false;
    m_lockMoveResets = 0;
    m_clearingRows.clear();

    // Populate queue with initial pieces
    for (int i = 0; i < 5; ++i) {
        if (m_bag.isEmpty()) fillBag();
        m_nextQueue.append(m_bag.takeFirst());
    }

    emit scoreChanged(m_score, m_highScore);
    emit levelChanged(m_level);
    emit linesChanged(m_linesCleared);
    emit holdPieceChanged(m_holdPiece, m_canHold);
    emit nextPieceChanged(m_nextQueue);

    m_state = Tetris::GameState::Playing;
    emit gameStateChanged(m_state);

    spawnPiece(takeNextPiece());
    updateDropInterval();
    m_dropTimer->start();
}

void TetrisEngine::pauseGame()
{
    if (m_state != Tetris::GameState::Playing) return;
    m_state = Tetris::GameState::Paused;
    m_dropTimer->stop();
    m_lockTimer->stop();
    emit gameStateChanged(m_state);
}

void TetrisEngine::resumeGame()
{
    if (m_state != Tetris::GameState::Paused) return;
    m_state = Tetris::GameState::Playing;
    m_dropTimer->start();
    if (m_isLocking) {
        m_lockTimer->start(500);
    }
    emit gameStateChanged(m_state);
}

void TetrisEngine::togglePause()
{
    if (m_state == Tetris::GameState::Playing) {
        pauseGame();
    } else if (m_state == Tetris::GameState::Paused) {
        resumeGame();
    }
}

void TetrisEngine::restartGame()
{
    startGame();
}

void TetrisEngine::updateDropInterval()
{
    // Authentic gravity curve: speeds up as level increases
    int interval = std::max(60, 800 - (m_level - 1) * 65);
    m_dropTimer->setInterval(interval);
}

bool TetrisEngine::spawnPiece(Tetris::TetrominoType type)
{
    m_currentPiece = type;
    m_pieceRotation = 0;
    m_pieceBlocks = Tetris::getInitialShape(type);

    int bbSize = Tetris::getBoundingBoxSize(type);
    m_pieceX = (Tetris::BOARD_WIDTH - bbSize) / 2;
    // Spawns in buffer rows, just entering the visible top
    m_pieceY = Tetris::BUFFER_HEIGHT - 2;

    m_isLocking = false;
    m_lockMoveResets = 0;
    m_lockTimer->stop();

    if (checkCollision(m_pieceBlocks, m_pieceX, m_pieceY)) {
        // Immediate collision upon spawn means block out / game over
        m_state = Tetris::GameState::GameOver;
        m_dropTimer->stop();
        m_lockTimer->stop();
        emit gameStateChanged(m_state);
        emit boardChanged();
        return false;
    }

    emit boardChanged();
    return true;
}

bool TetrisEngine::checkCollision(const QVector<QPoint> &blocks, int testX, int testY) const
{
    for (const QPoint &pt : blocks) {
        int boardCol = testX + pt.x();
        int boardRow = testY + pt.y();

        if (boardCol < 0 || boardCol >= Tetris::BOARD_WIDTH) return true;
        if (boardRow >= Tetris::TOTAL_HEIGHT) return true;

        if (boardRow >= 0 && m_board[boardRow][boardCol] != 0) {
            return true;
        }
    }
    return false;
}

QVector<QPoint> TetrisEngine::rotateShape(const QVector<QPoint> &blocks, int oldRot, int newRot, int bbSize) const
{
    Q_UNUSED(oldRot);
    Q_UNUSED(newRot);
    QVector<QPoint> rotated;
    rotated.reserve(blocks.size());

    // Clockwise rotation within bbSize x bbSize box
    for (const QPoint &p : blocks) {
        int nx = (bbSize - 1) - p.y();
        int ny = p.x();
        rotated.append(QPoint(nx, ny));
    }
    return rotated;
}

bool TetrisEngine::moveLeft()
{
    if (m_state != Tetris::GameState::Playing) return false;

    if (!checkCollision(m_pieceBlocks, m_pieceX - 1, m_pieceY)) {
        m_pieceX--;
        if (m_isLocking && m_lockMoveResets < MAX_LOCK_RESETS) {
            m_lockTimer->start(500);
            m_lockMoveResets++;
        }
        emit boardChanged();
        return true;
    }
    return false;
}

bool TetrisEngine::moveRight()
{
    if (m_state != Tetris::GameState::Playing) return false;

    if (!checkCollision(m_pieceBlocks, m_pieceX + 1, m_pieceY)) {
        m_pieceX++;
        if (m_isLocking && m_lockMoveResets < MAX_LOCK_RESETS) {
            m_lockTimer->start(500);
            m_lockMoveResets++;
        }
        emit boardChanged();
        return true;
    }
    return false;
}

bool TetrisEngine::rotateClockwise()
{
    if (m_state != Tetris::GameState::Playing) return false;
    if (m_currentPiece == Tetris::TetrominoType::O) return false;

    int bbSize = Tetris::getBoundingBoxSize(m_currentPiece);
    int nextRot = (m_pieceRotation + 1) % 4;

    QVector<QPoint> rotated = rotateShape(m_pieceBlocks, m_pieceRotation, nextRot, bbSize);

    // SRS Kick tests
    QVector<QPoint> kicks = (m_currentPiece == Tetris::TetrominoType::I)
                                ? Tetris::getSrsKicksI(m_pieceRotation, nextRot)
                                : Tetris::getSrsKicksJLSTZ(m_pieceRotation, nextRot);

    for (const QPoint &k : kicks) {
        if (!checkCollision(rotated, m_pieceX + k.x(), m_pieceY + k.y())) {
            m_pieceBlocks = rotated;
            m_pieceX += k.x();
            m_pieceY += k.y();
            m_pieceRotation = nextRot;

            if (m_isLocking && m_lockMoveResets < MAX_LOCK_RESETS) {
                m_lockTimer->start(500);
                m_lockMoveResets++;
            }
            emit boardChanged();
            return true;
        }
    }
    return false;
}

bool TetrisEngine::rotateCounterClockwise()
{
    if (m_state != Tetris::GameState::Playing) return false;
    if (m_currentPiece == Tetris::TetrominoType::O) return false;

    // 3 clockwise rotations equal 1 counter-clockwise
    int bbSize = Tetris::getBoundingBoxSize(m_currentPiece);
    int nextRot = (m_pieceRotation + 3) % 4;

    QVector<QPoint> rotated = m_pieceBlocks;
    for (int i = 0; i < 3; ++i) {
        rotated = rotateShape(rotated, 0, 1, bbSize);
    }

    QVector<QPoint> kicks = (m_currentPiece == Tetris::TetrominoType::I)
                                ? Tetris::getSrsKicksI(m_pieceRotation, nextRot)
                                : Tetris::getSrsKicksJLSTZ(m_pieceRotation, nextRot);

    for (const QPoint &k : kicks) {
        if (!checkCollision(rotated, m_pieceX + k.x(), m_pieceY + k.y())) {
            m_pieceBlocks = rotated;
            m_pieceX += k.x();
            m_pieceY += k.y();
            m_pieceRotation = nextRot;

            if (m_isLocking && m_lockMoveResets < MAX_LOCK_RESETS) {
                m_lockTimer->start(500);
                m_lockMoveResets++;
            }
            emit boardChanged();
            return true;
        }
    }
    return false;
}

bool TetrisEngine::softDrop()
{
    if (m_state != Tetris::GameState::Playing) return false;

    if (!checkCollision(m_pieceBlocks, m_pieceX, m_pieceY + 1)) {
        m_pieceY++;
        m_score += 1;
        emit scoreChanged(m_score, m_highScore);

        if (m_isLocking) {
            m_isLocking = false;
            m_lockTimer->stop();
        }
        emit boardChanged();
        return true;
    } else {
        // Ground reached
        if (!m_isLocking) {
            m_isLocking = true;
            m_lockTimer->start(500);
        }
    }
    return false;
}

void TetrisEngine::hardDrop()
{
    if (m_state != Tetris::GameState::Playing) return;

    int ghostY = calculateGhostY();
    int distance = ghostY - m_pieceY;
    m_pieceY = ghostY;

    m_score += distance * 2;
    emit scoreChanged(m_score, m_highScore);

    lockActivePiece();
}

bool TetrisEngine::holdPiece()
{
    if (m_state != Tetris::GameState::Playing || !m_canHold) return false;

    Tetris::TetrominoType temp = m_currentPiece;
    if (m_holdPiece == Tetris::TetrominoType::None) {
        m_holdPiece = temp;
        spawnPiece(takeNextPiece());
    } else {
        Tetris::TetrominoType toSpawn = m_holdPiece;
        m_holdPiece = temp;
        spawnPiece(toSpawn);
    }

    m_canHold = false;
    emit holdPieceChanged(m_holdPiece, m_canHold);
    emit boardChanged();
    return true;
}

int TetrisEngine::calculateGhostY() const
{
    int testY = m_pieceY;
    while (!checkCollision(m_pieceBlocks, m_pieceX, testY + 1)) {
        testY++;
    }
    return testY;
}

int TetrisEngine::ghostDropDistance() const
{
    return calculateGhostY() - m_pieceY;
}

QVector<QPoint> TetrisEngine::getCurrentPieceVisibleBlocks() const
{
    QVector<QPoint> result;
    if (m_state != Tetris::GameState::Playing && m_state != Tetris::GameState::Paused) {
        return result;
    }

    for (const QPoint &p : m_pieceBlocks) {
        int totalCol = m_pieceX + p.x();
        int totalRow = m_pieceY + p.y();
        int visibleRow = totalRow - Tetris::BUFFER_HEIGHT;
        if (visibleRow >= 0 && visibleRow < Tetris::BOARD_HEIGHT) {
            result.append(QPoint(totalCol, visibleRow));
        }
    }
    return result;
}

QVector<QPoint> TetrisEngine::getGhostPieceVisibleBlocks() const
{
    QVector<QPoint> result;
    if (m_state != Tetris::GameState::Playing) return result;

    int ghostY = calculateGhostY();
    for (const QPoint &p : m_pieceBlocks) {
        int totalCol = m_pieceX + p.x();
        int totalRow = ghostY + p.y();
        int visibleRow = totalRow - Tetris::BUFFER_HEIGHT;
        if (visibleRow >= 0 && visibleRow < Tetris::BOARD_HEIGHT) {
            result.append(QPoint(totalCol, visibleRow));
        }
    }
    return result;
}

void TetrisEngine::onDropTimerTick()
{
    if (m_state != Tetris::GameState::Playing) return;

    if (!checkCollision(m_pieceBlocks, m_pieceX, m_pieceY + 1)) {
        m_pieceY++;
        emit boardChanged();
    } else {
        if (!m_isLocking) {
            m_isLocking = true;
            m_lockTimer->start(500);
        }
    }
}

void TetrisEngine::onLockDelayTick()
{
    if (m_state != Tetris::GameState::Playing) return;

    // Check if still on the ground
    if (checkCollision(m_pieceBlocks, m_pieceX, m_pieceY + 1)) {
        lockActivePiece();
    } else {
        m_isLocking = false;
    }
}

void TetrisEngine::lockActivePiece()
{
    m_dropTimer->stop();
    m_lockTimer->stop();
    m_isLocking = false;

    // Stamp active piece into m_board
    int pieceVal = static_cast<int>(m_currentPiece);
    bool lockedAboveCeiling = true;

    for (const QPoint &p : m_pieceBlocks) {
        int col = m_pieceX + p.x();
        int row = m_pieceY + p.y();
        if (row >= 0 && row < Tetris::TOTAL_HEIGHT && col >= 0 && col < Tetris::BOARD_WIDTH) {
            m_board[row][col] = pieceVal;
            if (row >= Tetris::BUFFER_HEIGHT) {
                lockedAboveCeiling = false;
            }
        }
    }

    if (lockedAboveCeiling) {
        m_state = Tetris::GameState::GameOver;
        emit gameStateChanged(m_state);
        emit boardChanged();
        return;
    }

    m_canHold = true;
    emit holdPieceChanged(m_holdPiece, m_canHold);

    checkLineClears();
}

void TetrisEngine::checkLineClears()
{
    m_clearingRows.clear();
    for (int r = Tetris::BUFFER_HEIGHT; r < Tetris::TOTAL_HEIGHT; ++r) {
        bool full = true;
        for (int c = 0; c < Tetris::BOARD_WIDTH; ++c) {
            if (m_board[r][c] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            m_clearingRows.append(r - Tetris::BUFFER_HEIGHT);
        }
    }

    if (!m_clearingRows.isEmpty()) {
        m_state = Tetris::GameState::LineClearing;
        emit gameStateChanged(m_state);
        emit lineClearAnimationStarted(m_clearingRows);
        // Start animation timer
        m_clearAnimTimer->start(180);
    } else {
        m_combo = -1;
        finalizeLineClear();
    }
}

void TetrisEngine::onClearAnimationFinished()
{
    finalizeLineClear();
}

void TetrisEngine::finalizeLineClear()
{
    int lines = m_clearingRows.size();
    if (lines > 0) {
        m_combo++;

        // Shift rows down in the board
        for (int visibleRow : m_clearingRows) {
            int targetRow = visibleRow + Tetris::BUFFER_HEIGHT;
            for (int r = targetRow; r > 0; --r) {
                for (int c = 0; c < Tetris::BOARD_WIDTH; ++c) {
                    m_board[r][c] = m_board[r - 1][c];
                }
            }
            // Clear top row
            for (int c = 0; c < Tetris::BOARD_WIDTH; ++c) {
                m_board[0][c] = 0;
            }
        }

        m_clearingRows.clear();

        // Calculate score
        int basePoints = 0;
        QString bannerText;
        QColor bannerColor = Qt::cyan;

        switch (lines) {
        case 1:
            basePoints = 100 * m_level;
            bannerText = "SINGLE! +" + QString::number(basePoints);
            bannerColor = QColor(100, 200, 255);
            m_backToBackTetris = false;
            break;
        case 2:
            basePoints = 300 * m_level;
            bannerText = "DOUBLE! +" + QString::number(basePoints);
            bannerColor = QColor(100, 255, 100);
            m_backToBackTetris = false;
            break;
        case 3:
            basePoints = 500 * m_level;
            bannerText = "TRIPLE! +" + QString::number(basePoints);
            bannerColor = QColor(255, 200, 50);
            m_backToBackTetris = false;
            break;
        case 4:
            if (m_backToBackTetris) {
                basePoints = 1200 * m_level;
                bannerText = "B2B TETRIS!! +" + QString::number(basePoints);
                bannerColor = QColor(255, 50, 200);
            } else {
                basePoints = 800 * m_level;
                bannerText = "TETRIS! +" + QString::number(basePoints);
                bannerColor = QColor(0, 240, 240);
                m_backToBackTetris = true;
            }
            break;
        default:
            basePoints = 100 * lines * m_level;
            bannerText = QString::number(lines) + " LINES!";
            break;
        }

        if (m_combo > 0) {
            int comboBonus = 50 * m_combo * m_level;
            basePoints += comboBonus;
            bannerText += " [COMBO x" + QString::number(m_combo + 1) + "]";
        }

        m_score += basePoints;
        if (m_score > m_highScore) {
            m_highScore = m_score;
            saveHighScore();
        }

        m_linesCleared += lines;
        int newLevel = (m_linesCleared / 10) + 1;
        if (newLevel != m_level) {
            m_level = newLevel;
            updateDropInterval();
            emit levelChanged(m_level);
            emit floatingBanner("LEVEL UP! LV " + QString::number(m_level), QColor(255, 255, 100));
        } else {
            emit floatingBanner(bannerText, bannerColor);
        }

        emit scoreChanged(m_score, m_highScore);
        emit linesChanged(m_linesCleared);
    }

    m_state = Tetris::GameState::Playing;
    emit gameStateChanged(m_state);

    if (spawnPiece(takeNextPiece())) {
        m_dropTimer->start();
    }
}
