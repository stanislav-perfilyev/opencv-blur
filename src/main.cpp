#include <QApplication>
#include <QImageReader>
#include <QDebug>
#include <QCoreApplication>
#include "../include/mainwindow.h"

/**
 * @brief Main entry point of the application
 *
 * Initializes the Qt application and sets up plugin search paths
 * for image format support (JPEG, PNG, GIF, etc.)
 *
 * @param argc Number of command line arguments
 * @param argv Command line arguments
 * @return Application exit code
 */
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // Set plugin search paths
    // Search for plugins in application directory and 'plugins' folder
    a.addLibraryPath(QCoreApplication::applicationDirPath() + "/plugins");
    a.addLibraryPath(QCoreApplication::applicationDirPath());

    // Verify that Qt supports JPEG format
    qDebug() << "Plugin paths:" << a.libraryPaths();
    qDebug() << "Supported image formats:" << QImageReader::supportedImageFormats();

    MainWindow window;
    window.show();

    return QApplication::exec();
}

