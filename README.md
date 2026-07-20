# opencv-blur

Qt6 desktop application that applies a box-blur effect to images. The blur is a pure-Qt implementation (two-pass separable box filter over `QImage` pixel buffers, no OpenCV dependency).

## Architecture

- `include/mainwindow.h` / `src/mainwindow.cpp` — `MainWindow` with `static QImage blurImage(QImage, int radius)` (horizontal + vertical pass via `QtConcurrent`)
- `src/main.cpp` — application entry point
- `blur.ui` — Qt Designer UI (browse button + blur-radius slider)
- `tests/test_blur.cpp` — GTest suite; operates on `QImage` directly, no display required

## Usage

Launch the application:

```bash
QT_QPA_PLATFORM=offscreen ./build/Blur   # headless/CI
./build/Blur                             # desktop
```

Browse for an image file with the **Browse** button, then drag the slider to adjust the blur radius (0–10).

## Build

Set the `QT_PATH` CMake variable or `QT_PATH` environment variable to your Qt 6 installation root (e.g. `D:/Qt/6.10.2/mingw_64`). On Linux the system Qt6 is used automatically.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release [-DQT_PATH=/path/to/Qt6]
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Requirements

- CMake ≥ 3.16
- C++20 compiler (GCC 12+, Clang 15+)
- Qt 6 (Core, Gui, Widgets, Concurrent)
- Google Test — fetched automatically via CMake FetchContent
