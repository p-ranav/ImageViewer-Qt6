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
#include <atomic>
#include "IconHelper.hpp"
#include <QSettings>
#include "Preferences.hpp"

class MainWindow : public QMainWindow {
  Q_OBJECT

  static constexpr qreal SCALE_FACTOR = 0.90;

public:
  MainWindow();
  ~MainWindow() = default;

public slots:
  void openImage();
  void copyToClipboard();
  void copyImagePathToClipboard();
  void copyToLocation();
  void onImageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap, const ImageInfo& imageInfo);
  void onNoMoreImagesLeft();
  void quickExportAsPng();
  void showPreferences();

  // Slots for each setting change in the preferences widget
  void settingChangedSlideShowPeriod();
  void settingChangedSlideShowLoop();

protected:
  void closeEvent(QCloseEvent *event) override;
  void resizeEvent(QResizeEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

signals:
  void loadImage(const QString &imagePath);
  void goToStart();
  void goBackward();
  void previousImage(const QPixmap &currentPixmap);
  void nextImage(const QPixmap &currentPixmap);
  void goForward();
  void deleteCurrentImage();

private:
  void zoomIn();
  void zoomOut();
  void slideshowTimerCallback();
  void startSlideshow();
  void confirmAndDeleteCurrentImage();
  void toggleFullScreen();
  qreal getScaleFactor() const;

private:
  QLabel canvasLabel;

  ImageLoader *imageLoader;
  QThread *imageLoaderThread;

  bool m_sidebarVisible{false};
  ImageViewer *imageViewer;

  QWidget* m_toolbarWidget;

  QIcon m_leftArrowIcon;
  QPushButton *m_leftArrowButton;

  QIcon m_rightArrowIcon;
  QPushButton *m_rightArrowButton;

  QWidget * m_centralWidget;
  std::atomic<bool> m_fullScreen{false};

  QTimer *slideshowTimer;
  std::atomic<bool> m_slideshowLoop;

  Preferences *m_preferences;
};