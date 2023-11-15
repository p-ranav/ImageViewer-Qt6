#include "main_window.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.setWindowTitle("Resizable Collapsible Sidebar");

    mainWindow.show();

    return app.exec();
}

#include "main.moc"