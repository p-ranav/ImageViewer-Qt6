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
#include <QFile>
#include <libexif/exif-data.h>

class ImageLoader : public QObject {
  Q_OBJECT
 
  LibRaw m_rawProcessor;

  std::vector<std::string> m_imageFilePaths;
  std::size_t m_currentIndex{0};

  int m_previousImageWidth;
  int m_previousImageHeight;
  QPixmap m_previousPixmap;
  
  int m_nextImageWidth;
  int m_nextImageHeight;
  QPixmap m_nextPixmap;

  int m_currentImageWidth;
  int m_currentImageHeight;

  void loadImagePathsIfEmpty(const char* directory, const char* current_file);
  void loadRaw(const QString &imagePath, QPixmap& imagePixmap, bool half_size, int& width, int& height);
  void loadWithImageReader(const QString &imagePath, QPixmap& imagePixmap, int& width, int& height);
  void loadImageIntoPixmap(const QString &imagePath, QPixmap& imagePixmap, bool half_size, int& width, int& height);

public:
  ImageLoader();
  void resetImageFilePaths();
  QString getCurrentImageFilePath();
  QPixmap getCurrentImageFullRes();
  bool hasNext() const;
  bool hasPrevious() const;

public slots:
  void loadImage(const QString &imagePath);
  void nextImage(const QPixmap &currentPixmap);
  void previousImage(const QPixmap &currentPixmap);
  void deleteCurrentImage();

signals:
  void imageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap, int imageWidth, int imageHeight);
  void noMoreImagesLeft();
};