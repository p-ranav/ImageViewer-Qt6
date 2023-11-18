#include "main_window.hpp"

int main(int argc, char *argv[]) {
  QApplication app(argc, argv);

  app.setStyleSheet("QLabel { color: white; font-size: 14px; }");

  MainWindow mainWindow;
  mainWindow.setWindowTitle("Resizable Collapsible Sidebar");

  app.installEventFilter(&mainWindow);

  mainWindow.show();

  return app.exec();
}

#include "main.moc"