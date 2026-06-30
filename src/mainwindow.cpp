#include "../include/mainwindow.h"
#include "ui_blur.h"

#include <QFileDialog>
#include <QFuture>
#include <QImageReader>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QMetaObject>
#include <QPixmap>
#include <QtConcurrentRun>

Q_LOGGING_CATEGORY(lcBlur, "app.blur")

// ── Constructor / Destructor ───────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui_(new Ui::MainWindow)
{
    ui_->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, true);
    ui_->blurSlider->setValue(0);

    connect(ui_->browseButton, &QPushButton::clicked,
            this, &MainWindow::onBrowseButtonClicked);
    connect(ui_->blurSlider, QOverload<int>::of(&QSlider::valueChanged),
            this, &MainWindow::onBlurSliderValueChanged);
}

MainWindow::~MainWindow()
{
    // Wait for any in-flight blur to finish before destroying the UI.
    if (blurFuture_.isRunning())
        blurFuture_.waitForFinished();
    delete ui_;
}

// ── Slots ──────────────────────────────────────────────────────────────────

void MainWindow::onBrowseButtonClicked()
{
    const QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("Open Image"), {},
        tr("Image Files (*.png *.jpg *.jpeg *.bmp *.gif);;"
           "JPEG Files (*.jpg *.jpeg);;PNG Files (*.png);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    qCDebug(lcBlur) << "Loading image from:" << fileName;

    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    sourceImage_ = reader.read();

    if (sourceImage_.isNull()) {
        const QString msg = tr("Failed to load image.\nFile: %1\nError: %2")
                                .arg(fileName, reader.errorString());
        qCWarning(lcBlur) << msg;
        ui_->imageLabel->setText(msg);
        QMessageBox::warning(this, tr("Error"), msg);
        return;
    }

    qCInfo(lcBlur) << "Image loaded:" << sourceImage_.width()
                   << "x" << sourceImage_.height();
    ui_->blurSlider->setValue(0);
    updateDisplayImage(0);
}

void MainWindow::onBlurSliderValueChanged(int value)
{
    if (!sourceImage_.isNull())
        updateDisplayImage(value);
}

// ── Private helpers ────────────────────────────────────────────────────────

QImage MainWindow::blurImage(QImage source, int blurRadius)
{
    if (source.isNull() || blurRadius <= 0)
        return source;

    const int radius = blurRadius * 2;
    const int width  = source.width();
    const int height = source.height();

    // First pass — horizontal blur
    QImage horizontal(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0, count = 0;
            for (int dx = -radius; dx <= radius; ++dx) {
                const QColor c(source.pixel(qBound(0, x + dx, width - 1), y));
                sumR += c.red();   sumG += c.green();
                sumB += c.blue();  sumA += c.alpha();
                ++count;
            }
            horizontal.setPixelColor(x, y,
                QColor(sumR / count, sumG / count, sumB / count, sumA / count));
        }
    }

    // Second pass — vertical blur
    QImage result(width, height, QImage::Format_ARGB32);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int sumR = 0, sumG = 0, sumB = 0, sumA = 0, count = 0;
            for (int dy = -radius; dy <= radius; ++dy) {
                const QColor c(horizontal.pixel(x, qBound(0, y + dy, height - 1)));
                sumR += c.red();   sumG += c.green();
                sumB += c.blue();  sumA += c.alpha();
                ++count;
            }
            result.setPixelColor(x, y,
                QColor(sumR / count, sumG / count, sumB / count, sumA / count));
        }
    }

    return result;
}

void MainWindow::updateDisplayImage(int blurRadius)
{
    if (sourceImage_.isNull())
        return;

    // Cancel the previous task if it is still running so rapid slider moves
    // do not pile up work on the thread pool.
    if (blurFuture_.isRunning())
        blurFuture_.cancel();

    blurFuture_ = QtConcurrent::run([this, blurRadius]() {
        const QImage blurred = blurImage(sourceImage_, blurRadius);

        const QPixmap pixmap = QPixmap::fromImage(blurred).scaled(
            ui_->imageLabel->width(),
            ui_->imageLabel->height(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        QMetaObject::invokeMethod(ui_->imageLabel, [this, pixmap]() {
            ui_->imageLabel->setPixmap(pixmap);
        }, Qt::QueuedConnection);
    });
}
