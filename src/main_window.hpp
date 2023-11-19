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
#include "ExportWidget.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

  static constexpr qreal SCALE_FACTOR = 0.90;

public:
  MainWindow();
  ~MainWindow() = default;

public slots:
  void openImage();
  void copyToClipboard();
  void onImageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap, const ImageInfo& imageInfo);
  void onNoMoreImagesLeft();

  void quickExportAsPng();
  void exportAs();

protected:
  bool event(QEvent* event) override;
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
  void loadImage(const QString &imagePath);
  void goBackward();
  void previousImage(const QPixmap &currentPixmap);
  void nextImage(const QPixmap &currentPixmap);
  void goForward();
  void deleteCurrentImage();

private:
  void zoomIn();
  void zoomOut();
  static QIcon createColorIcon(const QString &imagePath, const QColor &color, int size);
  void confirmAndDeleteCurrentImage();

private:
  QLabel canvasLabel;

  ImageLoader *imageLoader;
  QThread *imageLoaderThread;

  bool m_sidebarVisible{false};
  ImageViewer *imageViewer;

  QPushButton *m_leftArrowButton;
  QPushButton *m_rightArrowButton;
};