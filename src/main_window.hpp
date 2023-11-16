#pragma once
#include "image_loader.hpp"
#include <QApplication>
#include <QCloseEvent>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScreen>
#include <QSplitter>
#include <QThread>
#include <QVBoxLayout>
#include <QWidget>
#include <QToolBar>
#include <QMainWindow>
#include <QToolButton>
#include <QMenuBar>
#include "image_viewer.hpp"
#include <chrono>
#include <iostream>

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow() = default;

public slots:
  void openImage();

  void onImageLoaded(const QPixmap &imagePixmap, const QPixmap &imagePixmapScaled, int width, int height);

protected:
  void closeEvent(QCloseEvent *event) override;

signals:
  void loadImage(const QString &imagePath, const QSize &targetSize);

private:

  QToolBar *toolBar;

  QLabel canvasLabel;
  
  QPixmap originalPixmap;
  int originalWidth;
  int originalHeight;

  QPixmap scaledPixmap;
  QPixmap fixedPixmap;

  ImageLoader *imageLoader;
  QThread *imageLoaderThread;

  ImageViewer *imageViewer;
  std::chrono::time_point<std::chrono::high_resolution_clock> load_start_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> load_end_time;
};