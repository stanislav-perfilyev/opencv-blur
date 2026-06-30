/**
 * @file test_blur.cpp
 * @brief Unit tests for MainWindow::blurImage() — pure static function.
 *
 * No QApplication is needed because blurImage operates only on QImage (pixel
 * buffer), which is fully usable without a display connection.
 */
#include <gtest/gtest.h>
#include "mainwindow.h"

// ── Helpers ───────────────────────────────────────────────────────────────

/// Build a solid-color 20x20 image for predictable blur tests.
static QImage makeSolid(int w, int h, QRgb color, QImage::Format fmt = QImage::Format_RGB32)
{
    QImage img(w, h, fmt);
    img.fill(color);
    return img;
}

// ── Zero-radius (identity) ────────────────────────────────────────────────

TEST(BlurImage, ZeroRadiusReturnsSamePixels)
{
    const QImage src = makeSolid(10, 10, qRgb(100, 150, 200));
    const QImage out = MainWindow::blurImage(src, 0);

    ASSERT_EQ(out.size(), src.size());
    // Every pixel must be unchanged
    for (int y = 0; y < src.height(); ++y)
        for (int x = 0; x < src.width(); ++x)
            EXPECT_EQ(out.pixel(x, y), src.pixel(x, y))
                << "Mismatch at (" << x << ", " << y << ")";
}

// ── Output size ────────────────────────────────────────────────────────────

TEST(BlurImage, OutputSizeEqualsInputSize)
{
    const QImage src = makeSolid(30, 15, qRgb(255, 0, 0));
    const QImage out = MainWindow::blurImage(src, 5);
    EXPECT_EQ(out.width(),  src.width());
    EXPECT_EQ(out.height(), src.height());
}

TEST(BlurImage, SinglePixelImageSurvivesBlur)
{
    const QImage src = makeSolid(1, 1, qRgb(128, 64, 32));
    const QImage out = MainWindow::blurImage(src, 3);
    ASSERT_FALSE(out.isNull());
    EXPECT_EQ(out.size(), src.size());
}

// ── Solid colour is preserved under any blur radius ───────────────────────

TEST(BlurImage, SolidRedPreservedAtRadius1)
{
    const QImage src = makeSolid(20, 20, qRgb(255, 0, 0));
    const QImage out = MainWindow::blurImage(src, 1);
    // Interior pixels of a solid image are unchanged after any box-blur
    for (int y = 1; y < out.height() - 1; ++y)
        for (int x = 1; x < out.width() - 1; ++x)
            EXPECT_EQ(qRed(out.pixel(x, y)), 255)
                << "Red channel changed at (" << x << ", " << y << ")";
}

TEST(BlurImage, SolidGreenPreservedAtRadius5)
{
    const QImage src = makeSolid(30, 30, qRgb(0, 200, 0));
    const QImage out = MainWindow::blurImage(src, 5);
    // Interior: 11 pixels from each edge
    for (int y = 6; y < out.height() - 6; ++y)
        for (int x = 6; x < out.width() - 6; ++x)
            EXPECT_EQ(qGreen(out.pixel(x, y)), 200);
}

// ── Blur actually modifies a checkerboard ────────────────────────────────

TEST(BlurImage, BlurSmoothesCheckerboard)
{
    // Build a 4x4 checkerboard: black (0,0,0) / white (255,255,255)
    QImage src(4, 4, QImage::Format_RGB32);
    for (int y = 0; y < 4; ++y)
        for (int x = 0; x < 4; ++x)
            src.setPixel(x, y, ((x + y) % 2 == 0) ? qRgb(0,0,0) : qRgb(255,255,255));

    const QImage out = MainWindow::blurImage(src, 1);
    // Centre pixels should be somewhere between 0 and 255
    const int centre = qRed(out.pixel(1, 1));
    EXPECT_GT(centre, 0)   << "Blur left pixel pure black (no smoothing)";
    EXPECT_LT(centre, 255) << "Blur left pixel pure white (no smoothing)";
}

// ── Negative / edge radii do not crash ────────────────────────────────────

TEST(BlurImage, NegativeRadiusDoesNotCrash)
{
    const QImage src = makeSolid(10, 10, qRgb(42, 42, 42));
    EXPECT_NO_THROW({
        const QImage out = MainWindow::blurImage(src, -1);
        EXPECT_FALSE(out.isNull());
    });
}

TEST(BlurImage, VeryLargeRadiusDoesNotCrash)
{
    const QImage src = makeSolid(5, 5, qRgb(10, 20, 30));
    EXPECT_NO_THROW({
        const QImage out = MainWindow::blurImage(src, 100);
        EXPECT_EQ(out.size(), src.size());
    });
}

// ── Null image ────────────────────────────────────────────────────────────

TEST(BlurImage, NullImageReturnsNullOrEmpty)
{
    EXPECT_NO_THROW({
        const QImage out = MainWindow::blurImage(QImage{}, 3);
        // Contract: must not crash; result may be null/empty.
        (void)out;
    });
}
