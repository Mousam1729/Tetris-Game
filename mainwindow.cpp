#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tetrisengine.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::MainWindow),
      m_engine(new TetrisEngine(this))
{
    ui->setupUi(this);

    // Ensure buttons don't steal focus from game keys
    ui->btnStart->setFocusPolicy(Qt::NoFocus);
    ui->btnPause->setFocusPolicy(Qt::NoFocus);
    ui->btnRestart->setFocusPolicy(Qt::NoFocus);
    ui->btnResetHigh->setFocusPolicy(Qt::NoFocus);
    ui->spinCellSize->setFocusPolicy(Qt::NoFocus);
    ui->checkGridLines->setFocusPolicy(Qt::NoFocus);
    ui->checkGhostPiece->setFocusPolicy(Qt::NoFocus);
    ui->comboBlockStyle->setFocusPolicy(Qt::NoFocus);

    // Initialize canvases
    ui->tetrisCanvas->setEngine(m_engine);
    ui->holdCanvas->setMode(PreviewCanvas::PreviewMode::Hold);
    ui->nextCanvas->setMode(PreviewCanvas::PreviewMode::NextMulti);

    // Visual defaults
    ui->spinCellSize->setValue(Tetris::DEFAULT_CELL_SIZE);
    ui->checkGridLines->setChecked(true);
    ui->checkGhostPiece->setChecked(true);
    ui->comboBlockStyle->setCurrentIndex(0);

    setupUiConnections();

    // Start in Ready state
    updateStatusText("Ready - Click Start or press Spacebar to play!");
    ui->tetrisCanvas->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupUiConnections()
{
    connect(m_engine, &TetrisEngine::scoreChanged, this, &MainWindow::onScoreChanged);
    connect(m_engine, &TetrisEngine::levelChanged, this, &MainWindow::onLevelChanged);
    connect(m_engine, &TetrisEngine::linesChanged, this, &MainWindow::onLinesChanged);
    connect(m_engine, &TetrisEngine::gameStateChanged, this, &MainWindow::onGameStateChanged);
    connect(m_engine, &TetrisEngine::holdPieceChanged, this, &MainWindow::onHoldPieceChanged);
    connect(m_engine, &TetrisEngine::nextPieceChanged, this, &MainWindow::onNextPieceChanged);

    connect(ui->tetrisCanvas, &TetrisCanvas::mouseMovedOnGrid, this, &MainWindow::onMouseMovedOnGrid);

    // Initial label values
    onScoreChanged(m_engine->score(), m_engine->highScore());
    onLevelChanged(m_engine->level());
    onLinesChanged(m_engine->linesCleared());
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (!m_engine) {
        QMainWindow::keyPressEvent(event);
        return;
    }

    Tetris::GameState state = m_engine->gameState();

    // If game is in Ready or Game Over, pressing Space or Return starts a new game
    if (state == Tetris::GameState::Ready || state == Tetris::GameState::GameOver) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            m_engine->startGame();
            event->accept();
            return;
        }
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_A:
        m_engine->moveLeft();
        event->accept();
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        m_engine->moveRight();
        event->accept();
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        m_engine->softDrop();
        event->accept();
        break;

    case Qt::Key_Up:
    case Qt::Key_W:
    case Qt::Key_X:
        m_engine->rotateClockwise();
        event->accept();
        break;

    case Qt::Key_Z:
    case Qt::Key_Control:
        m_engine->rotateCounterClockwise();
        event->accept();
        break;

    case Qt::Key_Space:
        m_engine->hardDrop();
        event->accept();
        break;

    case Qt::Key_C:
    case Qt::Key_Shift:
        m_engine->holdPiece();
        event->accept();
        break;

    case Qt::Key_P:
    case Qt::Key_Escape:
        m_engine->togglePause();
        event->accept();
        break;

    case Qt::Key_R:
        m_engine->restartGame();
        event->accept();
        break;

    default:
        QMainWindow::keyPressEvent(event);
        break;
    }
}

void MainWindow::onScoreChanged(int score, int highScore)
{
    ui->lblScoreValue->setText(QString::number(score));
    ui->lblHighScoreValue->setText(QString::number(highScore));
}

void MainWindow::onLevelChanged(int level)
{
    ui->lblLevelValue->setText(QString::number(level));
}

void MainWindow::onLinesChanged(int lines)
{
    ui->lblLinesValue->setText(QString::number(lines));
}

void MainWindow::onGameStateChanged(Tetris::GameState state)
{
    switch (state) {
    case Tetris::GameState::Ready:
        ui->btnStart->setText("▶ START");
        ui->btnPause->setEnabled(false);
        updateStatusText("Game Ready - Press Space or Start to play!");
        break;

    case Tetris::GameState::Playing:
        ui->btnStart->setText("▶ RESUME");
        ui->btnPause->setEnabled(true);
        ui->btnPause->setText("⏸ PAUSE");
        updateStatusText("Playing - Level " + QString::number(m_engine->level()));
        break;

    case Tetris::GameState::Paused:
        ui->btnPause->setText("▶ RESUME");
        updateStatusText("Paused - Press P or Resume to continue");
        break;

    case Tetris::GameState::LineClearing:
        updateStatusText("Line Clear!");
        break;

    case Tetris::GameState::GameOver:
        ui->btnStart->setText("▶ PLAY AGAIN");
        ui->btnPause->setEnabled(false);
        updateStatusText("Game Over! Final Score: " + QString::number(m_engine->score()) + " - Press R to restart");
        break;
    }
}

void MainWindow::onHoldPieceChanged(Tetris::TetrominoType piece, bool canHold)
{
    ui->holdCanvas->setSinglePiece(piece, canHold);
}

void MainWindow::onNextPieceChanged(const QVector<Tetris::TetrominoType> &nextPieces)
{
    ui->nextCanvas->setPieces(nextPieces);
}

void MainWindow::onMouseMovedOnGrid(int col, int row, int cartX, int cartY, int screenX, int screenY)
{
    Q_UNUSED(screenX);
    Q_UNUSED(screenY);

    if (col >= 0 && col < Tetris::BOARD_WIDTH && row >= 0 && row < Tetris::BOARD_HEIGHT) {
        ui->lblMouseGrid->setText(QString("Matrix Cell: (%1, %2)").arg(col).arg(row));
        ui->lblMouseCartesian->setText(QString("Cartesian: (%1, %2)").arg(cartX).arg(cartY));
    } else {
        ui->lblMouseGrid->setText("Matrix Cell: (Out of bounds)");
        ui->lblMouseCartesian->setText("Cartesian: (-- , --)");
    }
}

void MainWindow::on_btnStart_clicked()
{
    if (m_engine->gameState() == Tetris::GameState::Paused) {
        m_engine->resumeGame();
    } else {
        m_engine->startGame();
    }
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_btnPause_clicked()
{
    m_engine->togglePause();
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_btnRestart_clicked()
{
    m_engine->restartGame();
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_btnResetHigh_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Reset High Score",
        "Are you sure you want to reset the high score to 0?",
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        m_engine->resetHighScore();
    }
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_spinCellSize_valueChanged(int val)
{
    ui->tetrisCanvas->setCellSize(val);
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_checkGridLines_toggled(bool checked)
{
    ui->tetrisCanvas->setShowGridLines(checked);
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_checkGhostPiece_toggled(bool checked)
{
    ui->tetrisCanvas->setShowGhostPiece(checked);
    ui->tetrisCanvas->setFocus();
}

void MainWindow::on_comboBlockStyle_currentIndexChanged(int index)
{
    Tetris::BlockStyle style = static_cast<Tetris::BlockStyle>(index);
    ui->tetrisCanvas->setBlockStyle(style);
    ui->holdCanvas->setBlockStyle(style);
    ui->nextCanvas->setBlockStyle(style);
    ui->tetrisCanvas->setFocus();
}

void MainWindow::updateStatusText(const QString &text)
{
    ui->statusbar->showMessage(text);
}
