#pragma once
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QImageReader>
#include <QImage>
#include <QMessageBox>
#include <libraw/libraw.h>
#include <QFileInfo>
#include <vector>
#include <string>
#include <QDir>

class ImageLoader : public QObject {
  Q_OBJECT
 
  LibRaw m_rawProcessor;

  std::vector<std::string> m_imageFilePaths;
  std::size_t m_currentIndex{0};
  QPixmap m_previousPixmap;
  QPixmap m_nextPixmap;

  void loadImagePathsIfEmpty(const char* directory, const char* current_file);
  void loadNefRaw(const QString &imagePath, QPixmap& imagePixmap, bool half_size);
  void loadWithImageReader(const QString &imagePath, QPixmap& imagePixmap);
  void loadImageIntoPixmap(const QString &imagePath, QPixmap& imagePixmap, bool half_size);

public:
  void resetImageFilePaths();
  QString getCurrentImageFilePath();
  QPixmap getCurrentImageFullRes();

public slots:
  void loadImage(const QString &imagePath);
  void nextImage(const QPixmap &currentPixmap);
  void previousImage(const QPixmap &currentPixmap);

signals:
  void imageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap);
};