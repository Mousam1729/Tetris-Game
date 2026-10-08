#include "tetriscanvas.h"
#include "tetrisengine.h"
#include <QPaintEvent>
#include <QMouseEvent>
#include <QFont>
#include <QFontMetrics>
#include <cmath>

TetrisCanvas::TetrisCanvas(QWidget *parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_OpaquePaintEvent, false);

    // Banner animation timer
    m_bannerTimer = new QTimer(this);
    connect(m_bannerTimer, &QTimer::timeout, this, [this]() {
        m_bannerAlpha -= 12;
        if (m_bannerAlpha <= 0) {
            m_bannerAlpha = 0;
            m_bannerTimer->stop();
        }
        update();
    });

    // Line clear flash timer
    m_flashTimer = new QTimer(this);
    connect(m_flashTimer, &QTimer::timeout, this, [this]() {
        m_flashState = !m_flashState;
        update();
    });
}

void TetrisCanvas::setEngine(TetrisEngine *engine)
{
    m_engine = engine;
    if (m_engine) {
        connect(m_engine, &TetrisEngine::boardChanged, this, QOverload<>::of(&TetrisCanvas::update));
        connect(m_engine, &TetrisEngine::gameStateChanged, this, [this](Tetris::GameState state) {
            if (state == Tetris::GameState::LineClearing) {
                m_flashState = true;
                m_flashTimer->start(60);
            } else {
                m_flashTimer->stop();
                m_flashState = false;
            }
            update();
        });
        connect(m_engine, &TetrisEngine::floatingBanner, this, &TetrisCanvas::showFloatingBanner);
    }
    update();
}

void TetrisCanvas::setCellSize(int size)
{
    if (size < Tetris::MIN_CELL_SIZE) size = Tetris::MIN_CELL_SIZE;
    if (size > Tetris::MAX_CELL_SIZE) size = Tetris::MAX_CELL_SIZE;
    m_cellSize = size;
    updateGeometry();
    update();
}

void TetrisCanvas::setShowGridLines(bool show)
{
    m_showGridLines = show;
    update();
}

void TetrisCanvas::setShowGhostPiece(bool show)
{
    m_showGhostPiece = show;
    update();
}

void TetrisCanvas::setBlockStyle(Tetris::BlockStyle style)
{
    m_blockStyle = style;
    update();
}

QPoint TetrisCanvas::screenToGrid(const QPoint &pos) const
{
    int col = static_cast<int>(std::floor((pos.x() - m_boardOffsetX) / static_cast<double>(m_cellSize)));
    int row = static_cast<int>(std::floor((pos.y() - m_boardOffsetY) / static_cast<double>(m_cellSize)));
    return QPoint(col, row);
}

QPoint TetrisCanvas::gridToScreen(int col, int row) const
{
    int x = m_boardOffsetX + col * m_cellSize;
    int y = m_boardOffsetY + row * m_cellSize;
    return QPoint(x, y);
}

QPoint TetrisCanvas::gridToCartesian(int col, int row) const
{
    // CG Lab Cartesian coordinates: origin at grid center
    int midCol = Tetris::BOARD_WIDTH / 2;
    int midRow = Tetris::BOARD_HEIGHT / 2;
    int cartX = col - midCol;
    int cartY = midRow - row; // Y is inverted in Cartesian space
    return QPoint(cartX, cartY);
}

QSize TetrisCanvas::sizeHint() const
{
    int w = Tetris::BOARD_WIDTH * m_cellSize + 40;
    int h = Tetris::BOARD_HEIGHT * m_cellSize + 40;
    return QSize(w, h);
}

QSize TetrisCanvas::minimumSizeHint() const
{
    int w = Tetris::BOARD_WIDTH * Tetris::MIN_CELL_SIZE + 20;
    int h = Tetris::BOARD_HEIGHT * Tetris::MIN_CELL_SIZE + 20;
    return QSize(w, h);
}

void TetrisCanvas::showFloatingBanner(const QString &text, const QColor &color)
{
    m_bannerText = text;
    m_bannerColor = color;
    m_bannerAlpha = 255;
    m_bannerTimer->start(35);
    update();
}

void TetrisCanvas::mouseMoveEvent(QMouseEvent *event)
{
    QPoint gridPos = screenToGrid(event->pos());
    QPoint cartPos = gridToCartesian(gridPos.x(), gridPos.y());

    emit mouseMovedOnGrid(gridPos.x(), gridPos.y(), cartPos.x(), cartPos.y(),
                          event->pos().x(), event->pos().y());
    QWidget::mouseMoveEvent(event);
}

void TetrisCanvas::mousePressEvent(QMouseEvent *event)
{
    setFocus();
    QWidget::mousePressEvent(event);
}

void TetrisCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    int boardW = Tetris::BOARD_WIDTH * m_cellSize;
    int boardH = Tetris::BOARD_HEIGHT * m_cellSize;
    m_boardOffsetX = std::max(10, (width() - boardW) / 2);
    m_boardOffsetY = std::max(10, (height() - boardH) / 2);

    drawBackground(painter);

    if (m_showGridLines) {
        drawGridLines(painter);
    }

    if (m_engine) {
        drawBoardCells(painter);

        if (m_showGhostPiece && m_engine->gameState() == Tetris::GameState::Playing) {
            drawGhostPiece(painter);
        }

        drawActivePiece(painter);

        if (m_engine->gameState() == Tetris::GameState::LineClearing) {
            drawLineClearAnimation(painter);
        }
    }

    // Outer border frame for the raster matrix
    painter.setPen(QPen(QColor(70, 85, 110), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(m_boardOffsetX - 1, m_boardOffsetY - 1, boardW + 2, boardH + 2);

    if (m_bannerAlpha > 0) {
        drawBanner(painter);
    }

    if (m_engine) {
        drawOverlay(painter);
    }
}

void TetrisCanvas::drawBackground(QPainter &painter)
{
    // Fill entire widget with dark aesthetic background
    painter.fillRect(rect(), QColor(14, 18, 24));

    int boardW = Tetris::BOARD_WIDTH * m_cellSize;
    int boardH = Tetris::BOARD_HEIGHT * m_cellSize;

    // Board playing field background
    painter.fillRect(m_boardOffsetX, m_boardOffsetY, boardW, boardH, QColor(10, 12, 16));
}

void TetrisCanvas::drawGridLines(QPainter &painter)
{
    int boardW = Tetris::BOARD_WIDTH * m_cellSize;
    int boardH = Tetris::BOARD_HEIGHT * m_cellSize;

    QPen pen(QColor(30, 38, 50), 1, Qt::SolidLine);
    painter.setPen(pen);

    // Vertical raster lines
    for (int c = 0; c <= Tetris::BOARD_WIDTH; ++c) {
        int x = m_boardOffsetX + c * m_cellSize;
        painter.drawLine(x, m_boardOffsetY, x, m_boardOffsetY + boardH);
    }

    // Horizontal raster lines
    for (int r = 0; r <= Tetris::BOARD_HEIGHT; ++r) {
        int y = m_boardOffsetY + r * m_cellSize;
        painter.drawLine(m_boardOffsetX, y, m_boardOffsetX + boardW, y);
    }
}

void TetrisCanvas::drawRasterBlock(QPainter &painter, int px, int py, int size,
                                   const QColor &baseColor, Tetris::BlockStyle style,
                                   int alpha)
{
    int bevel = std::max(2, size / 8);

    if (style == Tetris::BlockStyle::Beveled3D) {
        // Base fill
        QColor fill = baseColor;
        fill.setAlpha(alpha);
        painter.fillRect(px, py, size, size, fill);

        // Highlight (top & left)
        QColor light = baseColor.lighter(155);
        light.setAlpha(alpha);
        painter.setBrush(light);
        painter.setPen(Qt::NoPen);

        QPolygon topBevel;
        topBevel << QPoint(px, py)
                 << QPoint(px + size, py)
                 << QPoint(px + size - bevel, py + bevel)
                 << QPoint(px + bevel, py + bevel);
        painter.drawPolygon(topBevel);

        QPolygon leftBevel;
        leftBevel << QPoint(px, py)
                  << QPoint(px + bevel, py + bevel)
                  << QPoint(px + bevel, py + size - bevel)
                  << QPoint(px, py + size);
        painter.drawPolygon(leftBevel);

        // Shadow (bottom & right)
        QColor dark = baseColor.darker(165);
        dark.setAlpha(alpha);
        painter.setBrush(dark);

        QPolygon bottomBevel;
        bottomBevel << QPoint(px, py + size)
                    << QPoint(px + bevel, py + size - bevel)
                    << QPoint(px + size - bevel, py + size - bevel)
                    << QPoint(px + size, py + size);
        painter.drawPolygon(bottomBevel);

        QPolygon rightBevel;
        rightBevel << QPoint(px + size, py)
                   << QPoint(px + size, py + size)
                   << QPoint(px + size - bevel, py + size - bevel)
                   << QPoint(px + size - bevel, py + bevel);
        painter.drawPolygon(rightBevel);

        // Inner face
        painter.fillRect(px + bevel, py + bevel, size - 2 * bevel, size - 2 * bevel, fill);

    } else if (style == Tetris::BlockStyle::ClassicFlat) {
        QColor fill = baseColor;
        fill.setAlpha(alpha);
        painter.fillRect(px + 1, py + 1, size - 2, size - 2, fill);
        painter.setPen(QPen(baseColor.darker(180), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(px, py, size - 1, size - 1);

    } else if (style == Tetris::BlockStyle::NeonGlow) {
        // High-tech neon glow
        QColor inner = baseColor.darker(280);
        inner.setAlpha(alpha);
        painter.fillRect(px + 1, py + 1, size - 2, size - 2, inner);

        QColor neon = baseColor.lighter(130);
        neon.setAlpha(alpha);
        painter.setPen(QPen(neon, 2));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(px + 1, py + 1, size - 3, size - 3);

        // Bright center dot
        painter.fillRect(px + size / 2 - 1, py + size / 2 - 1, 2, 2, neon);
    }
}

void TetrisCanvas::drawBoardCells(QPainter &painter)
{
    for (int r = 0; r < Tetris::BOARD_HEIGHT; ++r) {
        for (int c = 0; c < Tetris::BOARD_WIDTH; ++c) {
            int cellVal = m_engine->getCell(c, r);
            if (cellVal > 0) {
                Tetris::TetrominoType type = static_cast<Tetris::TetrominoType>(cellVal);
                QColor color = Tetris::getPieceColor(type);
                int px = m_boardOffsetX + c * m_cellSize;
                int py = m_boardOffsetY + r * m_cellSize;
                drawRasterBlock(painter, px, py, m_cellSize, color, m_blockStyle);
            }
        }
    }
}

void TetrisCanvas::drawGhostPiece(QPainter &painter)
{
    QVector<QPoint> ghostBlocks = m_engine->getGhostPieceVisibleBlocks();
    Tetris::TetrominoType type = m_engine->currentPieceType();
    QColor color = Tetris::getPieceColor(type);

    for (const QPoint &p : ghostBlocks) {
        int px = m_boardOffsetX + p.x() * m_cellSize;
        int py = m_boardOffsetY + p.y() * m_cellSize;

        // Draw translucent body
        QColor ghostFill = color;
        ghostFill.setAlpha(45);
        painter.fillRect(px + 1, py + 1, m_cellSize - 2, m_cellSize - 2, ghostFill);

        // Crisp dashed outline
        QPen ghostPen(color.lighter(120), 1.5, Qt::DashLine);
        painter.setPen(ghostPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(px + 1, py + 1, m_cellSize - 3, m_cellSize - 3);
    }
}

void TetrisCanvas::drawActivePiece(QPainter &painter)
{
    QVector<QPoint> activeBlocks = m_engine->getCurrentPieceVisibleBlocks();
    Tetris::TetrominoType type = m_engine->currentPieceType();
    QColor color = Tetris::getPieceColor(type);

    for (const QPoint &p : activeBlocks) {
        int px = m_boardOffsetX + p.x() * m_cellSize;
        int py = m_boardOffsetY + p.y() * m_cellSize;
        drawRasterBlock(painter, px, py, m_cellSize, color, m_blockStyle);
    }
}

void TetrisCanvas::drawLineClearAnimation(QPainter &painter)
{
    const QVector<int> &rows = m_engine->clearingRows();
    int boardW = Tetris::BOARD_WIDTH * m_cellSize;

    for (int r : rows) {
        int py = m_boardOffsetY + r * m_cellSize;
        QColor flashColor = m_flashState ? QColor(255, 255, 255, 230) : QColor(255, 230, 80, 200);
        painter.fillRect(m_boardOffsetX, py, boardW, m_cellSize, flashColor);

        // Scanline beam
        painter.setPen(QPen(Qt::white, 2));
        painter.drawLine(m_boardOffsetX, py + m_cellSize / 2,
                         m_boardOffsetX + boardW, py + m_cellSize / 2);
    }
}

void TetrisCanvas::drawBanner(QPainter &painter)
{
    painter.save();
    QFont font("Segoe UI", 16, QFont::Bold);
    painter.setFont(font);

    QFontMetrics fm(font);
    int tw = fm.horizontalAdvance(m_bannerText) + 30;
    int th = fm.height() + 14;

    int cx = m_boardOffsetX + (Tetris::BOARD_WIDTH * m_cellSize) / 2;
    int cy = m_boardOffsetY + (Tetris::BOARD_HEIGHT * m_cellSize) / 2 - 40;

    QRect r(cx - tw / 2, cy - th / 2, tw, th);

    QColor bg = QColor(10, 15, 25, std::min(220, m_bannerAlpha));
    painter.setBrush(bg);
    QColor border = m_bannerColor;
    border.setAlpha(m_bannerAlpha);
    painter.setPen(QPen(border, 2));
    painter.drawRoundedRect(r, 6, 6);

    painter.setPen(border);
    painter.drawText(r, Qt::AlignCenter, m_bannerText);
    painter.restore();
}

void TetrisCanvas::drawOverlay(QPainter &painter)
{
    Tetris::GameState state = m_engine->gameState();
    if (state == Tetris::GameState::Playing || state == Tetris::GameState::LineClearing) {
        return;
    }

    int boardW = Tetris::BOARD_WIDTH * m_cellSize;
    int boardH = Tetris::BOARD_HEIGHT * m_cellSize;
    QRect boardRect(m_boardOffsetX, m_boardOffsetY, boardW, boardH);

    painter.save();
    // Semi-transparent scrim
    painter.fillRect(boardRect, QColor(0, 0, 0, 185));

    QFont titleFont("Segoe UI", 18, QFont::Bold);
    QFont subFont("Segoe UI", 11);

    if (state == Tetris::GameState::Ready) {
        painter.setFont(titleFont);
        painter.setPen(QColor(0, 240, 240));
        painter.drawText(boardRect.adjusted(0, -30, 0, -30), Qt::AlignCenter, "TETRIS");

        painter.setFont(subFont);
        painter.setPen(QColor(220, 230, 245));
        painter.drawText(boardRect.adjusted(0, 30, 0, 30), Qt::AlignCenter, "Press ▶ START or SPACE\nto Play");

    } else if (state == Tetris::GameState::Paused) {
        painter.setFont(titleFont);
        painter.setPen(QColor(255, 200, 50));
        painter.drawText(boardRect.adjusted(0, -20, 0, -20), Qt::AlignCenter, "PAUSED");

        painter.setFont(subFont);
        painter.setPen(QColor(200, 210, 230));
        painter.drawText(boardRect.adjusted(0, 30, 0, 30), Qt::AlignCenter, "Press P to Resume");

    } else if (state == Tetris::GameState::GameOver) {
        painter.setFont(titleFont);
        painter.setPen(QColor(255, 60, 60));
        painter.drawText(boardRect.adjusted(0, -45, 0, -45), Qt::AlignCenter, "GAME OVER");

        painter.setFont(subFont);
        painter.setPen(Qt::white);
        QString scoreStr = "Score: " + QString::number(m_engine->score());
        painter.drawText(boardRect.adjusted(0, 5, 0, 5), Qt::AlignCenter, scoreStr);

        painter.setPen(QColor(180, 190, 210));
        painter.drawText(boardRect.adjusted(0, 50, 0, 50), Qt::AlignCenter, "Press R to Restart");
    }

    painter.restore();
}
