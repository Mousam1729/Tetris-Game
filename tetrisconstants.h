#ifndef TETRISCONSTANTS_H
#define TETRISCONSTANTS_H

#include <QColor>
#include <QPoint>
#include <QVector>

namespace Tetris {

// Matrix dimensions (standard guideline Tetris)
constexpr int BOARD_WIDTH = 10;
constexpr int BOARD_HEIGHT = 20;
constexpr int BUFFER_HEIGHT = 4; // Hidden rows above the visible ceiling
constexpr int TOTAL_HEIGHT = BOARD_HEIGHT + BUFFER_HEIGHT; // 24 rows total

// Default raster graphics settings
constexpr int DEFAULT_CELL_SIZE = 28;
constexpr int MIN_CELL_SIZE = 18;
constexpr int MAX_CELL_SIZE = 42;

// Tetromino identifiers
enum class TetrominoType {
    None = 0,
    I = 1,
    J = 2,
    L = 3,
    O = 4,
    S = 5,
    T = 6,
    Z = 7
};

// Visual block styles for the raster grid
enum class BlockStyle {
    Beveled3D,   // Modern arcade with light/shadow beveled edges
    ClassicFlat, // Flat crisp retro blocks with dark borders
    NeonGlow     // High-tech neon outline and vibrant core
};

// Game state
enum class GameState {
    Ready,
    Playing,
    Paused,
    LineClearing,
    GameOver
};

// Colors for each tetromino (standard Tetris guideline palette)
inline QColor getPieceColor(TetrominoType type, int alpha = 255) {
    switch (type) {
    case TetrominoType::I: return QColor(0, 240, 240, alpha);   // Cyan
    case TetrominoType::J: return QColor(0, 100, 240, alpha);   // Deep Blue
    case TetrominoType::L: return QColor(240, 160, 0, alpha);   // Orange
    case TetrominoType::O: return QColor(240, 240, 0, alpha);   // Yellow
    case TetrominoType::S: return QColor(0, 220, 50, alpha);    // Green
    case TetrominoType::T: return QColor(160, 0, 240, alpha);   // Purple/Magenta
    case TetrominoType::Z: return QColor(240, 40, 40, alpha);   // Red
    default: return QColor(40, 40, 50, alpha);
    }
}

// 4-cell coordinate offsets for each tetromino at rotation state 0 (bounding box 4x4 or 3x3)
// Coordinates are given in relative (x, y) where x is horizontal [0..3], y is vertical [0..3]
inline QVector<QPoint> getInitialShape(TetrominoType type) {
    switch (type) {
    case TetrominoType::I:
        // Center row in 4x4
        return { QPoint(0, 1), QPoint(1, 1), QPoint(2, 1), QPoint(3, 1) };
    case TetrominoType::J:
        return { QPoint(0, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1) };
    case TetrominoType::L:
        return { QPoint(2, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1) };
    case TetrominoType::O:
        return { QPoint(1, 0), QPoint(2, 0), QPoint(1, 1), QPoint(2, 1) };
    case TetrominoType::S:
        return { QPoint(1, 0), QPoint(2, 0), QPoint(0, 1), QPoint(1, 1) };
    case TetrominoType::T:
        return { QPoint(1, 0), QPoint(0, 1), QPoint(1, 1), QPoint(2, 1) };
    case TetrominoType::Z:
        return { QPoint(0, 0), QPoint(1, 0), QPoint(1, 1), QPoint(2, 1) };
    default:
        return {};
    }
}

// Bounding box size for rotation pivot: 2 for O, 4 for I, 3 for others
inline int getBoundingBoxSize(TetrominoType type) {
    if (type == TetrominoType::O) return 2;
    if (type == TetrominoType::I) return 4;
    return 3;
}

// SRS wall kick data for J, L, S, T, Z pieces
// Format: kick offsets (dx, dy) for 4 rotation transitions:
// 0->1, 1->0, 1->2, 2->1, 2->3, 3->2, 3->0, 0->3
inline QVector<QPoint> getSrsKicksJLSTZ(int fromRot, int toRot) {
    if (fromRot == 0 && toRot == 1) return { {0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} };
    if (fromRot == 1 && toRot == 0) return { {0, 0}, { 1, 0}, { 1,  1}, {0,-2}, { 1,-2} };
    if (fromRot == 1 && toRot == 2) return { {0, 0}, { 1, 0}, { 1,  1}, {0,-2}, { 1,-2} };
    if (fromRot == 2 && toRot == 1) return { {0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2} };
    if (fromRot == 2 && toRot == 3) return { {0, 0}, { 1, 0}, { 1, -1}, {0, 2}, { 1, 2} };
    if (fromRot == 3 && toRot == 2) return { {0, 0}, {-1, 0}, {-1,  1}, {0,-2}, {-1,-2} };
    if (fromRot == 3 && toRot == 0) return { {0, 0}, {-1, 0}, {-1,  1}, {0,-2}, {-1,-2} };
    if (fromRot == 0 && toRot == 3) return { {0, 0}, { 1, 0}, { 1, -1}, {0, 2}, { 1, 2} };
    return { {0, 0} };
}

// SRS wall kick data for I piece
inline QVector<QPoint> getSrsKicksI(int fromRot, int toRot) {
    if (fromRot == 0 && toRot == 1) return { {0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1,-2} };
    if (fromRot == 1 && toRot == 0) return { {0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1, 2} };
    if (fromRot == 1 && toRot == 2) return { {0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2, 1} };
    if (fromRot == 2 && toRot == 1) return { {0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2,-1} };
    if (fromRot == 2 && toRot == 3) return { {0, 0}, { 2, 0}, {-1, 0}, { 2, -1}, {-1, 2} };
    if (fromRot == 3 && toRot == 2) return { {0, 0}, {-2, 0}, { 1, 0}, {-2,  1}, { 1,-2} };
    if (fromRot == 3 && toRot == 0) return { {0, 0}, { 1, 0}, {-2, 0}, { 1,  2}, {-2,-1} };
    if (fromRot == 0 && toRot == 3) return { {0, 0}, {-1, 0}, { 2, 0}, {-1, -2}, { 2, 1} };
    return { {0, 0} };
}

} // namespace Tetris

#endif // TETRISCONSTANTS_H
