#include <QApplication>
#include <QWidget>
#include <QScreen>
#include <QLabel>
#include <QVBoxLayout>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Create the main window
    QWidget mainWindow;
    mainWindow.setWindowTitle("Hello, World!");

    // Get the primary screen geometry
    QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();

    // Set the window size to take up the full screen
    mainWindow.setGeometry(primaryScreenGeometry);

    // Create a QLabel with the text
    QLabel label("Hello, World!");

    // Create a layout to manage the widgets
    QVBoxLayout layout;
    layout.addWidget(&label);

    // Set the layout for the main window
    mainWindow.setLayout(&layout);

    // Show the main window
    mainWindow.show();

    return app.exec();
}
