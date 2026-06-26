#include "../include/mainwindow.h"
#include "ui_blur.h"
#include <QFileDialog>
#include <QPixmap>
#include <QDebug>
#include <QImageReader>
#include <QMessageBox>
#include <QtConcurrentRun>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Application closes when main window is closed
    setAttribute(Qt::WA_QuitOnClose, true);

    // Set initial slider value
    ui->blurSlider->setValue(0);

    // Connect signals to slots
    connect(ui->browseButton, &QPushButton::clicked, this, &MainWindow::onBrowseButtonClicked);
    connect(ui->blurSlider, QOverload<int>::of(&QSlider::valueChanged),
            this, &MainWindow::onBlurSliderValueChanged);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onBrowseButtonClicked() {
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "",
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;JPEG Files (*.jpg *.jpeg);;PNG Files (*.png);;All Files (*)"));

    if (!fileName.isEmpty()) {
        qDebug() << "Loading image from:" << fileName;

        // Create QImageReader for better error diagnostics
        QImageReader reader(fileName);
        reader.setAutoTransform(true);

        sourceImage = reader.read();

        if (sourceImage.isNull()) {
            QString errorMsg = QString("Failed to load image.\nFile: %1\nError: %2")
                    .arg(fileName, reader.errorString());
            qDebug() << errorMsg;
            ui->imageLabel->setText(errorMsg);
            QMessageBox::warning(this, "Error", errorMsg);
        } else {
            qDebug() << "Image loaded successfully. Size:" << sourceImage.width() << "x" << sourceImage.height();
            ui->blurSlider->setValue(0);
            updateDisplayImage(0);
        }
    }
}

void MainWindow::onBlurSliderValueChanged(int value) {
    if (!sourceImage.isNull()) {
        updateDisplayImage(value);
    }
}

QImage MainWindow::blurImage(QImage source, int blurRadius) {
    if (source.isNull() || blurRadius <= 0) {
        return source;
    }

    // Increase blur radius for better effect
    int radius = blurRadius * 2;

    QImage result = source.copy();
    int width = source.width();
    int height = source.height();

    // First pass - horizontal blur
    QImage horizontal(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            int count = 0;

            // Get pixels in horizontal band
            for (int dx = -radius; dx <= radius; ++dx) {
                int nx = qBound(0, x + dx, width - 1);
                QColor color(source.pixel(nx, y));
                sumR += color.red();
                sumG += color.green();
                sumB += color.blue();
                sumA += color.alpha();
                count++;
            }

            // Average the values
            QColor blurredColor(
                sumR / count,
                sumG / count,
                sumB / count,
                sumA / count
            );
            horizontal.setPixelColor(x, y, blurredColor);
        }
    }

    // Second pass - vertical blur
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0;
            int count = 0;

            // Get pixels in vertical band
            for (int dy = -radius; dy <= radius; ++dy) {
                int ny = qBound(0, y + dy, height - 1);
                QColor color(horizontal.pixel(x, ny));
                sumR += color.red();
                sumG += color.green();
                sumB += color.blue();
                sumA += color.alpha();
                count++;
            }

            // Average the values
            QColor blurredColor(
                sumR / count,
                sumG / count,
                sumB / count,
                sumA / count
            );
            result.setPixelColor(x, y, blurredColor);
        }
    }

    return result;
}

void MainWindow::updateDisplayImage(int blurRadius) {
    if (sourceImage.isNull()) {
        return;
    }

    // Apply blur asynchronously to prevent UI freezing
    QtConcurrent::run([this, blurRadius]() {
        QImage blurred = blurImage(sourceImage, blurRadius);

        // Scale image to label size
        QPixmap pixmap = QPixmap::fromImage(blurred).scaled(
            ui->imageLabel->width(),
            ui->imageLabel->height(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        );

        // Update UI in main thread
        QMetaObject::invokeMethod(ui->imageLabel, [this, pixmap]() {
            ui->imageLabel->setPixmap(pixmap);
        }, Qt::QueuedConnection);
    });
}

