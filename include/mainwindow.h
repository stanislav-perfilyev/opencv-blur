#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

namespace Ui {
    class MainWindow;
}

/**
 * @class MainWindow
 * @brief Main application window for image blur effect
 *
 * Provides UI for loading images and applying blur effect with adjustable intensity
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    /**
     * @brief Constructs the MainWindow
     * @param parent Parent widget (default: nullptr)
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destructor for MainWindow
     */
    ~MainWindow();

private slots:
    /**
     * @brief Slot called when Browse button is clicked
     * Opens file dialog to select an image file
     */
    void onBrowseButtonClicked();

    /**
     * @brief Slot called when blur slider value changes
     * @param value New blur radius value (0-10)
     */
    void onBlurSliderValueChanged(int value);

private:
    Ui::MainWindow *ui;                           ///< UI pointer
    QImage sourceImage;                           ///< Loaded source image

    /**
     * @brief Applies blur effect to an image
     * @param source Source image to blur
     * @param blurRadius Blur intensity (0-10)
     * @return Blurred image
     */
    static QImage blurImage(QImage source, int blurRadius);

    /**
     * @brief Updates the displayed image with blur effect
     * @param blurRadius Blur intensity value
     */
    void updateDisplayImage(int blurRadius);
};

#endif // MAINWINDOW_H

