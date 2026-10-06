# 🧱 Tetris — Computer Graphics Lab Project

> *A fully playable Tetris game built from scratch in **C++ / Qt 6**, where every move, rotation, and flip is powered by **2D homogeneous transformation matrices** — because why hardcode coordinates when you can do linear algebra?*

---

## ✨ What Is This?

This isn't just another Tetris clone. It's a **Computer Graphics lab assignment** that brings textbook concepts to life:

- **Translation** — moving pieces left, right, and down  
- **Rotation** — spinning pieces 90° around their pivot  
- **Reflection** — flipping a piece into its mirror image (turning an L into a J!)  
- **Scaling** — rendering the next-piece preview at a different size  
- **Matrix concatenation** — chaining all of the above into a single 3×3 matrix per piece  

Every transformation you see on screen is the result of multiplying homogeneous coordinate matrices. No `if-else` spaghetti — just clean, composable math.

---

## 🎮 Controls

| Key | Action |
|-----|--------|
| ← / `A` | Move piece left |
| → / `D` | Move piece right |
| ↑ / `W` | Rotate 90° clockwise |
| ↓ / `S` | Soft drop (+ 1 point per row) |
| `Space` | Hard drop (+ 2 points per row) |
| `F` | Flip / reflect the piece |
| `P` | Pause / Resume |
| `N` | New Game |
| **Left-click** on canvas | Rotate piece |

> **Tip:** You can also hover over the grid to see the logical (x, y) coordinates under your cursor — handy for understanding the coordinate system!

---

## 📁 Project Structure

Here's what each file does and *why* it exists:

```
CG Lab Tetris Game Project/
│
├── 📄 DrawLine.pro            ← Qt project file (think of it as the recipe card)
├── 📄 main.cpp                ← Entry point — boots up Qt and opens the window
│
├── 🧠 mainwindow.h            ← The brain: game state, Mat3 struct, Piece struct
├── 🎮 mainwindow.cpp          ← The muscle: game logic, transforms, rendering
│
├── 🖱️ my_label.h              ← Custom QLabel that talks to the mouse
├── 🖱️ my_label.cpp            ← Emits signals on hover, click, and release
│
├── 🎨 mainwindow.ui           ← The face: UI layout designed in Qt Designer
│
└── 📂 build/                  ← Compiled output (auto-generated, don't touch)
    └── Qt_6_11_1_for_macOS_Debug/
        └── DrawLine.app/      ← The runnable macOS app bundle
```

### A Closer Look at Each File

#### [`main.cpp`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/main.cpp) — *The ignition key* 🔑
Just 10 lines. Creates a `QApplication`, shows the `MainWindow`, and hands control to the Qt event loop. Simple and clean.

#### [`mainwindow.h`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/mainwindow.h) — *The blueprint* 🧠

This is where the core data structures live:

- **`Mat3`** (lines 30–103) — A 3×3 homogeneous transformation matrix with factory methods for:
  - `translation(tx, ty)` — slide things around
  - `rotation(degrees)` — spin around the origin
  - `scaling(sx, sy)` — resize (or reflect, when a scale factor is negative)
  - `rotationAbout(deg, px, py)` — rotate around any arbitrary point
  - `reflectionAboutX(px)` — mirror across a vertical line
  - `operator*` — multiply two matrices (chain transformations)
  - `apply(x, y)` — transform a point and snap to the nearest grid cell

- **`Piece`** — Just a shape type + one `Mat3`. That single matrix encodes *everything* about where the piece is and how it's oriented. Moving? Multiply by a translation matrix. Rotating? Multiply by a rotation matrix. It's all matrix concatenation.

- **Board constants** — 10 × 20 grid, mapped onto a centered coordinate system via `T(X0, Y0)`.

#### [`mainwindow.cpp`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/mainwindow.cpp) — *Where the magic happens* 🎮

535 lines of game logic and rendering, organized into clear sections:

| Section | Lines | What It Does |
|---------|-------|--------------|
| Tetromino data | 16–24 | The 7 classic shapes as model-space coordinates |
| Constructor | 32–71 | Wires up the UI, keyboard, timer, and starts a new game |
| Mouse handling | 80–98 | Hover shows coordinates; click rotates the piece |
| Game logic | 102–293 | New game, spawn, move, rotate, flip, drop, lock, clear lines, scoring, gravity |
| Keyboard | 297–348 | Maps keys → game actions |
| Drawing | 350–509 | Grid, walls, locked blocks, ghost piece, current piece, next-piece preview, pause/game-over overlay |
| UI slots | 513–535 | Button handlers for New Game, Pause, Grid Size |

**Highlights worth reading:**
- **7-bag randomiser** (line 125) — every set of 7 pieces contains each tetromino exactly once, just like the official Tetris guideline.
- **Wall kicks** (lines 194–202) — if a rotation would clip the wall, the game tries shifting the piece ±1 or ±2 columns first.
- **Ghost piece** (lines 474–483) — a translucent preview showing where the piece will land.
- **Next-piece preview** (lines 413–438) — uses its own model → screen pipeline: `V = T(ox, oy) · S(cell, -cell) · T(-minX, -maxY)`.

#### [`my_label.h`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/my_label.h) / [`my_label.cpp`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/my_label.cpp) — *The mouse whisperer* 🖱️

A custom `QLabel` subclass that:
- Tracks mouse movement and emits `sendMousePosition(QPoint&)` on hover
- Emits `Mouse_Pos()` on left-click (triggers piece rotation)
- Emits `Mouse_Released()` on button release

This separation keeps mouse-event plumbing out of the main game logic — clean MVC-ish design.

#### [`mainwindow.ui`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/mainwindow.ui) — *The layout* 🎨

An XML file generated by Qt Designer. Defines the 950 × 640 window with:

| Widget | Purpose |
|--------|---------|
| `frame` (600 × 600 `my_label`) | The game canvas |
| `label_score`, `label_lines`, `label_level` | Score / Lines / Level display |
| `label_next` | Next-piece preview box |
| `spinBox` + `btn_draw_grid` | Adjust grid cell size (10–30 px) |
| `btn_pause`, `clear` | Pause and New Game buttons |
| `mouse_movement` | Live cursor coordinates |
| `label_help` | Controls cheat-sheet |

#### [`DrawLine.pro`](file:///Users/jinnad80/CG%20Lab%20Tetris%20Game%20Project/DrawLine.pro) — *The build recipe* 📄

Qt `.pro` project file. Requires `Qt Core`, `Qt GUI`, `Qt Widgets`, and C++17.

---

## 🧮 The Math Behind It All

Every piece in this game carries a single **3×3 transformation matrix** $M$. Here's how each action maps to matrix math:

| Action | Matrix Operation | Formula |
|--------|-----------------|---------|
| **Spawn** | Translation | $M = T(4,\; 18)$ |
| **Move left** | Translation | $M' = T(-1,\; 0) \cdot M$ |
| **Move right** | Translation | $M' = T(1,\; 0) \cdot M$ |
| **Gravity / soft drop** | Translation | $M' = T(0,\; -1) \cdot M$ |
| **Rotate 90° CW** | Rotation about pivot $p$ | $M' = T(p) \cdot R(-90°) \cdot T(-p) \cdot M$ |
| **Flip (reflect)** | Reflection about $x = p_x$ | $M' = T(p) \cdot S(-1,\; 1) \cdot T(-p) \cdot M$ |
| **Preview scaling** | Scale + flip y + translate | $V = T(o_x,\; o_y) \cdot S(s,\; -s) \cdot T(-\min_x,\; -\max_y)$ |

All of these are **composed by matrix multiplication** — the same foundational concept you learn in any Computer Graphics course.

---

## 🏗️ How to Build & Run

### Prerequisites
- **Qt 6** (tested with Qt 6.11.1)
- A C++17 compatible compiler (Clang, GCC, or MSVC)
- Qt Creator (recommended) or `qmake` on the command line

### Option 1: Qt Creator (easiest)
1. Open `DrawLine.pro` in Qt Creator
2. Select your Qt 6 kit
3. Click the green ▶ **Run** button
4. Enjoy!

### Option 2: Command line
```bash
qmake DrawLine.pro
make            # or mingw32-make on Windows
./DrawLine      # or open DrawLine.app on macOS
```

---

## 🏆 Scoring System

| Event | Points |
|-------|--------|
| Soft drop (per row) | 1 |
| Hard drop (per row) | 2 |
| 1 line cleared | 100 × level |
| 2 lines cleared | 300 × level |
| 3 lines cleared | 500 × level |
| 4 lines (Tetris!) | 800 × level |

**Level up** every 10 lines. Each level makes gravity faster (down to a minimum of 100ms per tick).

---

## 🎓 CG Concepts Demonstrated

This project covers the following topics from a typical Computer Graphics syllabus:

- [x] **Homogeneous coordinates** and 3×3 matrices
- [x] **2D Translation** — piece movement, board-to-screen mapping
- [x] **2D Rotation** — piece rotation around an arbitrary pivot
- [x] **2D Scaling** — next-piece preview rendering
- [x] **2D Reflection** — piece flipping (mirror transformation)
- [x] **Matrix concatenation** — composing multiple transforms into one
- [x] **Coordinate systems** — model space → board space → logical grid → screen pixels
- [x] **Pixel-level rendering** — `setPixel()` for filling grid cells
- [x] **Viewport / window mapping** — centered grid with configurable cell size

---

## 🤝 Built With

- **C++17** — modern language features
- **Qt 6** — cross-platform GUI framework
- **Qt Creator** — IDE and UI designer
- **Pure math** — no game engine, no physics library, just matrices 💪

---

<p align="center">
  <i>Made with ☕ and way too many matrix multiplications.</i>
</p>

