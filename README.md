# Tetris - Computer Graphics Lab Project

A full-featured, guideline-accurate Tetris game developed in C++ using Qt Widgets and custom raster graphics algorithms. Designed for the Computer Graphics Lab curriculum, bridging discrete raster coordinate systems with authentic game mechanics.

---

## 🎯 Computer Graphics Lab Features

This project directly incorporates concepts and conventions from the Computer Graphics Lab assignments (specifically referencing Assignment 1, Assignment 2, and Assignment 7):

1. **Discrete Raster Grid Representation**:
   - The playfield is rendered as a discrete 10x20 raster matrix (with 4 hidden buffer rows for spawning above the visible ceiling).
   - Each cell corresponds to a discrete raster unit rendered through custom coordinate transformation equations.

2. **Coordinate Transformations**:
   - **Screen Coordinates $(X_{screen}, Y_{screen})$**: Raw pixel coordinates relative to the canvas widget.
   - **Matrix Grid Coordinates $(Col, Row)$**: Discrete integer grid indices ($0 \le Col < 10$, $0 \le Row < 20$).
   - **Cartesian Coordinates $(X_{cart}, Y_{cart})$**: Modeled after **Assignment 1**, placing the Cartesian origin $(0, 0)$ at the center of the playfield with inverted $Y$ axis:
     $$\begin{aligned}
     X_{cart} &= Col - \lfloor \text{Width} / 2 \rfloor \\
     Y_{cart} &= \lfloor \text{Height} / 2 \rfloor - Row
     \end{aligned}$$
   - **Interactive Coordinate Inspector**: Hovering the mouse over any cell dynamically updates both the discrete matrix position and Cartesian coordinates in the left-hand status panel.

3. **Custom Raster Rendering Styles**:
   - **Beveled 3D (Default)**: Multi-polygon shading with directional light source (specular top/left highlight, ambient bottom/right shadow, and vibrant core).
   - **Classic Flat**: Crisp retro arcade block rasterization with contrasting cell borders.
   - **Neon Glow**: High-tech cyber aesthetic with darkened cell bodies, bright neon boundary strokes, and center raster points.

4. **Real-time Raster Adjustments**:
   - **Cell Size Scaling**: Dynamic scaling from 18px up to 38px per cell.
   - **Grid Lines Toggle**: Enable or disable discrete raster scanlines.
   - **Ghost Piece Toggle**: Real-time projection of hard-drop impact point.

---

## 🕹️ Authentic Tetris Mechanics

- **Guideline Tetrominoes**: All 7 standard pieces ($I, J, L, O, S, T, Z$) with official guideline colors:
  - **I**: Cyan (`#00F0F0`)
  - **J**: Deep Blue (`#0064F0`)
  - **L**: Orange (`#F0A000`)
  - **O**: Yellow (`#F0F000`)
  - **S**: Green (`#00DC32`)
  - **T**: Purple / Magenta (`#A000F0`)
  - **Z**: Red (`#F02828`)
- **Super Rotation System (SRS)**: Standard wall kicks for all pieces (including the 4x4 bounding box kicks for the $I$-tetromino), enabling smooth rotation near matrix boundaries and obstacles.
- **7-Bag Randomizer**: Randomly shuffles bags of 7 pieces to prevent piece starvation.
- **Ghost Piece**: Translucent real-time drop projection showing where the current tetromino will land.
- **Hold Queue**: Swap the current piece into the hold container with `C` or `Shift` (limited to 1 swap per piece drop).
- **Next Queue**: Multi-piece preview canvas displaying upcoming tetrominoes.
- **Lock Delay**: 500 ms grace period upon touching ground with up to 15 move/rotation resets.
- **Line Clear Animation**: Raster flash animation before collapsed rows shift down.

---

## 🏆 Scoring System

The scoring system mirrors official Tetris guidelines and scales with the current level:

| Action | Points Awarded |
| :--- | :--- |
| **Single (1 Line)** | $100 \times \text{Level}$ |
| **Double (2 Lines)** | $300 \times \text{Level}$ |
| **Triple (3 Lines)** | $500 \times \text{Level}$ |
| **Tetris (4 Lines)** | $800 \times \text{Level}$ |
| **Back-to-Back Tetris** | $1200 \times \text{Level}$ |
| **Combo Bonus** | $50 \times \text{Combo Count} \times \text{Level}$ |
| **Soft Drop** | $1 \text{ pt} / \text{cell}$ |
| **Hard Drop** | $2 \text{ pts} / \text{cell}$ |

- **Level Progression**: Increases by 1 for every 10 lines cleared.
- **Dynamic Gravity**: Drop interval accelerates according to:
  $$\Delta t = \max(60\text{ ms}, 800 - (\text{Level} - 1) \times 65\text{ ms})$$
- **High Score Persistence**: Saved to local application settings (`QSettings`) across game sessions.

---

## 🎮 Controls

| Action | Primary Key | Alternative Key |
| :--- | :--- | :--- |
| **Move Left** | `←` (Left Arrow) | `A` |
| **Move Right** | `→` (Right Arrow) | `D` |
| **Soft Drop** | `↓` (Down Arrow) | `S` |
| **Rotate Clockwise** | `↑` (Up Arrow) | `W`, `X` |
| **Rotate Counter-Clockwise** | `Z` | `Ctrl` |
| **Hard Drop** | `Spacebar` | — |
| **Hold Piece** | `C` | `Shift` |
| **Pause / Resume** | `P` | `Escape` |
| **Restart Game** | `R` | — |

*Note: All buttons on the user interface have `NoFocus` enabled so pressing keyboard keys immediately controls the active tetromino without needing to click on the board first.*

---

## 📁 Project Structure

```
group_project/
├── TetrisProject.pro     # Qt qmake project file
├── main.cpp              # Application entry point & High-DPI configuration
├── mainwindow.h          # Main window header
├── mainwindow.cpp        # Main window logic, button signals, key handler
├── mainwindow.ui         # Qt Designer XML interface with dark modern theme
├── tetrisconstants.h     # Dimensions, colors, tetromino shapes, SRS tables
├── tetrisengine.h        # Game engine header (bag, logic, timers, SRS kicks)
├── tetrisengine.cpp      # Game mechanics, gravity, scoring, collision detection
├── tetriscanvas.h        # Raster playfield widget header
├── tetriscanvas.cpp      # Discrete raster rendering, bevels, flash animation
├── previewcanvas.h       # Hold / Next piece mini-canvas header
├── previewcanvas.cpp     # Preview renderer
└── README.md             # Project documentation
```

---

## 🚀 How to Build and Run

### Option 1: Using Qt Creator (Recommended)
1. Open **Qt Creator**.
2. Select **File → Open File or Project...** and choose [`TetrisProject.pro`](file:///f:/academic%20CS/3rd%20year/3.1/cg_lab/group_project/TetrisProject.pro).
3. Select your installed Qt kit (e.g., Qt 6.x MinGW 64-bit).
4. Click **Run** (`Ctrl+R` or the green play button) to build and launch the application.

### Option 2: Command Line (MinGW / Powershell)
Run the following commands in Powershell:
```powershell
# Set PATH to Qt and MinGW binaries
$env:PATH = "F:\Qt\6.11.1\mingw_64\bin;F:\Qt\Tools\mingw1310_64\bin;" + $env:PATH

# Navigate to project directory
cd "F:\academic CS\3rd year\3.1\cg_lab\group_project"

# Create build directory
mkdir build -Force
cd build

# Generate Makefile with qmake
qmake ..\TetrisProject.pro

# Compile with mingw32-make
mingw32-make -j4

# Launch the game!
.\release\TetrisGame.exe
```
