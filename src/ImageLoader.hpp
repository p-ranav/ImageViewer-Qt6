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
#include <QDir>
#include <QFile>
#include <QColorSpace>

#include <libexif/exif-data.h>

#include "ImageInfo.hpp"

#include <vector>
#include <string>

class ImageLoader : public QObject {
  Q_OBJECT
 
  LibRaw m_rawProcessor;

  std::vector<QString> m_imageFilePaths;
  std::size_t m_currentIndex{0};

  QPixmap m_previousPixmap;
  QPixmap m_nextPixmap;

  ImageInfo m_currentImageInfo;
  ImageInfo m_previousImageInfo;
  ImageInfo m_nextImageInfo;

  std::atomic<bool> m_currentSortOrderAscending{true};

  void loadImagePathsIfEmpty(const char* directory, const char* current_file);
  ImageInfo loadRaw(const QString &imagePath, QPixmap& imagePixmap, bool half_size);
  ImageInfo loadWithImageReader(const QString &imagePath, QPixmap& imagePixmap);
  ImageInfo loadImageIntoPixmap(const QString &imagePath, QPixmap& imagePixmap, bool half_size);
  void updateCurrentIndexAfterSort(const QString& currentImagePath);

public:
  ImageLoader();
  void resetImageFilePaths();
  QString getCurrentImageFilePath();
  QPixmap getCurrentImageFullRes();
  bool hasNext() const;
  bool hasPrevious() const;
  QString getHeaderLabel() const;

public slots:
  void loadImage(const QString &imagePath);
  void goToStart();
  void goBackward();
  void previousImage(const QPixmap &currentPixmap);
  void nextImage(const QPixmap &currentPixmap);
  void goForward();
  void deleteCurrentImage();
  void sortAscending();
  void sortDescending();

signals:
  void imageLoaded(const QFileInfo& imageFileInfo, const QPixmap &imagePixmap, const ImageInfo& imageInfo);
  void noMoreImagesLeft();
};