#ifndef PREVIEWCANVAS_H
#define PREVIEWCANVAS_H

#include "tetrisconstants.h"
#include <QWidget>
#include <QPainter>
#include <QVector>

class PreviewCanvas : public QWidget {
    Q_OBJECT

public:
    enum class PreviewMode {
        Hold,
        NextSingle,
        NextMulti
    };

    explicit PreviewCanvas(QWidget *parent = nullptr);
    ~PreviewCanvas() override = default;

    void setMode(PreviewMode mode);
    void setPieces(const QVector<Tetris::TetrominoType> &pieces);
    void setSinglePiece(Tetris::TetrominoType piece, bool enabled = true);
    void setBlockStyle(Tetris::BlockStyle style);
    void setCellSize(int size);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    void drawPiece(QPainter &painter, Tetris::TetrominoType type, int centerX, int centerY, bool enabled);
    void drawRasterBlock(QPainter &painter, int px, int py, int size,
                         const QColor &baseColor, bool enabled);

    PreviewMode m_mode = PreviewMode::NextSingle;
    QVector<Tetris::TetrominoType> m_pieces;
    Tetris::TetrominoType m_singlePiece = Tetris::TetrominoType::None;
    bool m_singleEnabled = true;
    Tetris::BlockStyle m_blockStyle = Tetris::BlockStyle::Beveled3D;
    int m_cellSize = 20;
};

#endif // PREVIEWCANVAS_H
