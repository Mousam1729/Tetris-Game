#ifndef TETRISCANVAS_H
#define TETRISCANVAS_H

#include "tetrisconstants.h"
#include <QWidget>
#include <QPainter>
#include <QTimer>
#include <QPoint>
#include <QVector>

class TetrisEngine;

class TetrisCanvas : public QWidget {
    Q_OBJECT

public:
    explicit TetrisCanvas(QWidget *parent = nullptr);
    ~TetrisCanvas() override = default;

    void setEngine(TetrisEngine *engine);

    // Raster visual options (CG Lab features)
    void setCellSize(int size);
    int cellSize() const { return m_cellSize; }

    void setShowGridLines(bool show);
    bool showGridLines() const { return m_showGridLines; }

    void setShowGhostPiece(bool show);
    bool showGhostPiece() const { return m_showGhostPiece; }

    void setBlockStyle(Tetris::BlockStyle style);
    Tetris::BlockStyle blockStyle() const { return m_blockStyle; }

    // Coordinate conversions (direct CG Lab mapping)
    QPoint screenToGrid(const QPoint &pos) const;
    QPoint gridToScreen(int col, int row) const;
    QPoint gridToCartesian(int col, int row) const;

signals:
    void mouseMovedOnGrid(int col, int row, int cartX, int cartY, int screenX, int screenY);

public slots:
    void showFloatingBanner(const QString &text, const QColor &color);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void drawBackground(QPainter &painter);
    void drawGridLines(QPainter &painter);
    void drawBoardCells(QPainter &painter);
    void drawGhostPiece(QPainter &painter);
    void drawActivePiece(QPainter &painter);
    void drawLineClearAnimation(QPainter &painter);
    void drawBanner(QPainter &painter);
    void drawOverlay(QPainter &painter);

    void drawRasterBlock(QPainter &painter, int px, int py, int size,
                         const QColor &baseColor, Tetris::BlockStyle style,
                         int alpha = 255);

    TetrisEngine *m_engine = nullptr;

    int m_cellSize = Tetris::DEFAULT_CELL_SIZE;
    bool m_showGridLines = true;
    bool m_showGhostPiece = true;
    Tetris::BlockStyle m_blockStyle = Tetris::BlockStyle::Beveled3D;

    // Board offset in the canvas for centering
    int m_boardOffsetX = 0;
    int m_boardOffsetY = 0;

    // Floating banner animation
    QString m_bannerText;
    QColor m_bannerColor = Qt::cyan;
    int m_bannerAlpha = 0;
    QTimer *m_bannerTimer = nullptr;

    // Line clear flash animation state
    bool m_flashState = false;
    QTimer *m_flashTimer = nullptr;
};

#endif // TETRISCANVAS_H
