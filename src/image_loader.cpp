#include "image_loader.hpp"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

std::vector<std::string> getImageFiles(const char *directory) {
  std::vector<std::string> imageFiles;

  for (const auto &entry : fs::directory_iterator(directory)) {
    if (entry.is_regular_file()) {

      std::string extension = entry.path().extension();
      std::transform(extension.begin(), extension.end(), extension.begin(),
                     ::tolower);

      const std::vector<std::string> allowedExtensions = {
          ".jpg", ".jpeg", ".png", ".nef", ".tiff", ".webp"};

      if (std::find(allowedExtensions.begin(), allowedExtensions.end(),
                    extension) != allowedExtensions.end()) {
        imageFiles.push_back(entry.path().string());
      }
    }
  }

  std::sort(imageFiles.begin(), imageFiles.end());

  return imageFiles;
}

void ImageLoader::loadImagePathsIfEmpty(const char *directory,
                                        const char *current_file) {
  if (m_imageFilePaths.empty()) {
    m_imageFilePaths = getImageFiles(directory);

    // Find the index in one line using C++17
    auto it = std::find(m_imageFilePaths.begin(), m_imageFilePaths.end(),
                        current_file);
    int index = (it != m_imageFilePaths.end())
                    ? std::distance(m_imageFilePaths.begin(), it)
                    : -1;

    if (index == -1) {
      // bad
    } else {
      m_currentIndex = index;
    }
  }
}

void ImageLoader::loadNefRaw(const QString &imagePath, QPixmap &imagePixmap,
                             bool half_size) {
  m_rawProcessor.open_file(imagePath.toLocal8Bit().data());

  m_rawProcessor.imgdata.params.half_size = half_size ? 1 : 0;

  m_rawProcessor.unpack();
  m_rawProcessor.dcraw_process();

  libraw_processed_image_t *processed_image =
      m_rawProcessor.dcraw_make_mem_image();

  QImage image(processed_image->data, processed_image->width,
               processed_image->height, QImage::Format_RGB888);

  // Convert QImage to QPixmap and display it
  imagePixmap = QPixmap::fromImage(image);

  m_rawProcessor.dcraw_clear_mem(processed_image);
}

void ImageLoader::loadWithImageReader(const QString &imagePath,
                                      QPixmap &imagePixmap) {
  QImageReader imageReader(imagePath);
  imageReader.setAllocationLimit(0);
  imageReader.setAutoTransform(true);

  QImage image = imageReader.read();
  if (image.isNull()) {
    /// TODO: Show warning message
    // QMessageBox::warning(this, "Error", "Failed to open the image.");
    return;
  }

  imagePixmap = QPixmap::fromImage(image);
}

void ImageLoader::loadImageIntoPixmap(const QString &imagePath,
                                      QPixmap &imagePixmap, bool half_size) {
  QFileInfo fileInfo(imagePath);
  if (fileInfo.suffix().toLower() == "nef") {
    loadNefRaw(imagePath, imagePixmap, half_size);
  } else {
    loadWithImageReader(imagePath, imagePixmap);
  }
}

void ImageLoader::resetImageFilePaths() {
  m_imageFilePaths.clear();
  m_currentIndex = 0;
}

void ImageLoader::loadImage(const QString &imagePath) {

  QFileInfo fileInfo(imagePath);
  QPixmap imagePixmap;

  loadImagePathsIfEmpty(fileInfo.dir().absolutePath().toLocal8Bit().data(),
                        fileInfo.absoluteFilePath().toLocal8Bit().data());
  loadImageIntoPixmap(imagePath, imagePixmap, true);

  emit imageLoaded(imagePixmap);

  // Prefetch next and previous images
  if (m_currentIndex >= 1) {
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true);
  }
  if (m_currentIndex + 1 < m_imageFilePaths.size()) {
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]),
        m_nextPixmap, true);
  }
}

void ImageLoader::previousImage(const QPixmap &currentPixmap) {
  if (m_currentIndex >= 1) {

    emit imageLoaded(m_previousPixmap);

    m_nextPixmap = currentPixmap;

    m_currentIndex -= 1;
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true);
  }
}

void ImageLoader::nextImage(const QPixmap &currentPixmap) {
  if (m_currentIndex + 1 < m_imageFilePaths.size()) {

    emit imageLoaded(m_nextPixmap);

    m_previousPixmap = currentPixmap;

    m_currentIndex += 1;
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]),
        m_nextPixmap, true);
  }
}

QString ImageLoader::getCurrentImageFilePath() {
  return QString::fromStdString(m_imageFilePaths[m_currentIndex]);
}

QPixmap ImageLoader::getCurrentImageFullRes() {
  auto imagePath = m_imageFilePaths[m_currentIndex];

  QPixmap imagePixmap;
  loadImageIntoPixmap(QString::fromStdString(imagePath), imagePixmap, false);

  return imagePixmap;
}