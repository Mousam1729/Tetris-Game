#include "previewcanvas.h"
#include <algorithm>

PreviewCanvas::PreviewCanvas(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_OpaquePaintEvent, false);
}

void PreviewCanvas::setMode(PreviewMode mode)
{
    m_mode = mode;
    updateGeometry();
    update();
}

void PreviewCanvas::setPieces(const QVector<Tetris::TetrominoType> &pieces)
{
    m_pieces = pieces;
    update();
}

void PreviewCanvas::setSinglePiece(Tetris::TetrominoType piece, bool enabled)
{
    m_singlePiece = piece;
    m_singleEnabled = enabled;
    update();
}

void PreviewCanvas::setBlockStyle(Tetris::BlockStyle style)
{
    m_blockStyle = style;
    update();
}

void PreviewCanvas::setCellSize(int size)
{
    m_cellSize = size;
    updateGeometry();
    update();
}

QSize PreviewCanvas::sizeHint() const
{
    if (m_mode == PreviewMode::NextMulti) {
        return QSize(4 * m_cellSize + 20, 10 * m_cellSize + 20);
    }
    return QSize(4 * m_cellSize + 20, 4 * m_cellSize + 20);
}

void PreviewCanvas::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    // Dark container background
    painter.fillRect(rect(), QColor(10, 12, 16));

    // Outer subtle border
    painter.setPen(QPen(QColor(40, 50, 65), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(0, 0, width() - 1, height() - 1);

    if (m_mode == PreviewMode::Hold) {
        if (m_singlePiece != Tetris::TetrominoType::None) {
            drawPiece(painter, m_singlePiece, width() / 2, height() / 2, m_singleEnabled);
        }
    } else if (m_mode == PreviewMode::NextSingle) {
        Tetris::TetrominoType next = m_pieces.isEmpty() ? Tetris::TetrominoType::None : m_pieces.first();
        if (next != Tetris::TetrominoType::None) {
            drawPiece(painter, next, width() / 2, height() / 2, true);
        }
    } else if (m_mode == PreviewMode::NextMulti) {
        int count = std::min(3, static_cast<int>(m_pieces.size()));
        int slotHeight = height() / std::max(1, count);
        for (int i = 0; i < count; ++i) {
            int cy = i * slotHeight + slotHeight / 2;
            drawPiece(painter, m_pieces[i], width() / 2, cy, true);
        }
    }
}

void PreviewCanvas::drawPiece(QPainter &painter, Tetris::TetrominoType type, int centerX, int centerY, bool enabled)
{
    QVector<QPoint> blocks = Tetris::getInitialShape(type);
    if (blocks.isEmpty()) return;

    // Compute bounding box of the piece
    int minX = 4, maxX = -1, minY = 4, maxY = -1;
    for (const QPoint &p : blocks) {
        minX = std::min(minX, p.x());
        maxX = std::max(maxX, p.x());
        minY = std::min(minY, p.y());
        maxY = std::max(maxY, p.y());
    }

    int pw = (maxX - minX + 1) * m_cellSize;
    int ph = (maxY - minY + 1) * m_cellSize;

    int startX = centerX - pw / 2;
    int startY = centerY - ph / 2;

    QColor color = enabled ? Tetris::getPieceColor(type) : QColor(90, 95, 105);

    for (const QPoint &p : blocks) {
        int px = startX + (p.x() - minX) * m_cellSize;
        int py = startY + (p.y() - minY) * m_cellSize;
        drawRasterBlock(painter, px, py, m_cellSize, color, enabled);
    }
}

void PreviewCanvas::drawRasterBlock(QPainter &painter, int px, int py, int size,
                                    const QColor &baseColor, bool enabled)
{
    int bevel = std::max(1, size / 8);

    if (m_blockStyle == Tetris::BlockStyle::Beveled3D) {
        painter.fillRect(px, py, size, size, baseColor);

        QColor light = baseColor.lighter(150);
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

        QColor dark = baseColor.darker(160);
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

        painter.fillRect(px + bevel, py + bevel, size - 2 * bevel, size - 2 * bevel, baseColor);

    } else {
        painter.fillRect(px + 1, py + 1, size - 2, size - 2, baseColor);
        painter.setPen(QPen(baseColor.darker(170), 1));
        painter.setBrush(Qt::NoBrush);
        painter.drawRect(px, py, size - 1, size - 1);
    }

    if (!enabled) {
        // Disabled tint
        painter.fillRect(px, py, size, size, QColor(0, 0, 0, 80));
    }
}
