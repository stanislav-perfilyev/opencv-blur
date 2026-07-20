#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QFuture>
#include <QImage>
#include <QMainWindow>

#include <memory>

Q_DECLARE_LOGGING_CATEGORY(lcBlur)

namespace Ui {
class MainWindow;
}

/**
 * @class MainWindow
 * @brief Main application window for image blur effect.
 *
 * Provides UI for loading images and applying a box-blur effect with
 * adjustable intensity via a slider.  The blur runs asynchronously on a
 * thread-pool so the UI stays responsive.
 *
 * Non-copyable and non-movable (Qt widget ownership semantics).
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(MainWindow)

public:
    /**
     * @brief Constructs the MainWindow.
     * @param parent Parent widget (default: nullptr).
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /** @brief Destructor. */
    ~MainWindow() override;

    /**
     * @brief Applies a two-pass box blur to @p source.
     *
     * Exposed as public to allow unit testing without a running QApplication.
     *
     * @param source     Input image (not modified).
     * @param blurRadius Blur intensity in [0, 10]; 0 returns the original.
     * @return Blurred copy of @p source.
     */
    [[nodiscard]] static QImage blurImage(QImage source, int blurRadius);

private slots:
    /**
     * @brief Opens a file dialog to select an image file.
     */
    void onBrowseButtonClicked();

    /**
     * @brief Re-applies the blur whenever the slider moves.
     * @param value New blur radius in [0, 10].
     */
    void onBlurSliderValueChanged(int value);

private:
    /**
     * @brief Launches an async blur and updates the image label on completion.
     *
     * Cancels any in-flight blur before starting a new one so rapid slider
     * movements do not pile up work on the thread pool.
     *
     * @param blurRadius Blur intensity to apply.
     */
    void updateDisplayImage(int blurRadius);

    std::unique_ptr<Ui::MainWindow> ui_;      ///< UI pointer (owned)
    QImage                    sourceImage_;  ///< Loaded source image
    QFuture<void>             blurFuture_;  ///< Handle to the current async blur task
};

#endif // MAINWINDOW_H
