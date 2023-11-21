#include "MainWindow.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  MainWindow mainWindow;
  mainWindow.setWindowTitle("Resizable Collapsible Sidebar");

  app.installEventFilter(&mainWindow);

  mainWindow.show();

  return app.exec();
}

#include "main.moc"