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
#include "vertical_sidebar.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow() = default;

public slots:
  void openImage();
  void quickExportAsPng();
  void copyToClipboard();
  void onImageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap);
  void onNoMoreImagesLeft();

protected:
  bool event(QEvent* event) override;
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
  void loadImage(const QString &imagePath);
  void nextImage(const QPixmap &currentPixmap);
  void previousImage(const QPixmap &currentPixmap);
  void deleteCurrentImage();

private:
  void zoomIn();
  void zoomOut();

private:
  QLabel canvasLabel;

  ImageLoader *imageLoader;
  QThread *imageLoaderThread;

  bool sidebarVisible{false};
  VerticalSidebar* sidebar;
  ImageViewer *imageViewer;

  QPushButton *button1;
  QPushButton *button2;
};