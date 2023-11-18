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

ImageLoader::ImageLoader() : QObject() {
  // m_rawProcessor.imgdata.params.use_auto_wb = 1;
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

void ImageLoader::loadRaw(const QString &imagePath, QPixmap &imagePixmap,
                          bool half_size, int &width, int &height) {
  m_rawProcessor.open_file(imagePath.toLocal8Bit().data());

  m_rawProcessor.imgdata.params.half_size = half_size ? 1 : 0;

  m_rawProcessor.unpack();
  m_rawProcessor.dcraw_process();

  // Access the image resolution
  width = m_rawProcessor.imgdata.sizes.raw_width;
  height = m_rawProcessor.imgdata.sizes.raw_height;
  std::cout << width << " x " << height << "\n";

  libraw_processed_image_t *processed_image =
      m_rawProcessor.dcraw_make_mem_image();

  // Check if the depth is 16 bits
  QImage::Format format = (processed_image->bits == 16) ? QImage::Format_RGB16
                                                        : QImage::Format_RGB888;

  QImage image(processed_image->data, processed_image->width,
               processed_image->height, format);

  // Convert QImage to QPixmap and display it
  imagePixmap = QPixmap::fromImage(image);

  m_rawProcessor.dcraw_clear_mem(processed_image);
}

void ImageLoader::loadWithImageReader(const QString &imagePath,
                                      QPixmap &imagePixmap, int &width,
                                      int &height) {
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
  width = imagePixmap.width();
  height = imagePixmap.height();
}

void ImageLoader::loadImageIntoPixmap(const QString &imagePath,
                                      QPixmap &imagePixmap, bool half_size,
                                      int &width, int &height) {
  QFileInfo fileInfo(imagePath);

  QStringList rawFormats = {"nef", "cr2", "arw", "dng", "orf",
                            "pef", "rw2", "srw", "crw", "raf"};

  if (rawFormats.contains(fileInfo.suffix().toLower())) {
    loadRaw(imagePath, imagePixmap, half_size, width, height);
  } else {
    loadWithImageReader(imagePath, imagePixmap, width, height);
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
  loadImageIntoPixmap(imagePath, imagePixmap, true, m_currentImageWidth,
                      m_currentImageHeight);

  emit imageLoaded(fileInfo, imagePixmap, m_currentImageWidth,
                   m_currentImageHeight);

  // Prefetch next and previous images
  if (m_currentIndex >= 1) {
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true, m_previousImageWidth, m_previousImageHeight);
  }
  if (m_currentIndex + 1 < m_imageFilePaths.size()) {
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]),
        m_nextPixmap, true, m_nextImageWidth, m_nextImageHeight);
  }
}

bool ImageLoader::hasPrevious() const {
  return m_imageFilePaths.size() > 0 && (m_currentIndex >= 1);
}

void ImageLoader::goBackward() {
  if (m_currentIndex >= 10) {
    m_currentIndex -= 10;
  } else {
    m_currentIndex = 0;
  }

  loadImage(QString::fromStdString(m_imageFilePaths[m_currentIndex]));
}

void ImageLoader::previousImage(const QPixmap &currentPixmap) {
  if (hasPrevious()) {

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]));

    m_nextPixmap = currentPixmap;
    m_nextImageWidth = m_currentImageWidth;
    m_nextImageHeight = m_currentImageHeight;

    m_currentImageWidth = m_previousImageWidth;
    m_currentImageHeight = m_previousImageHeight;

    m_currentIndex -= 1;

    emit imageLoaded(fileInfo, m_previousPixmap, m_previousImageWidth,
                     m_previousImageHeight);

    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true, m_previousImageWidth, m_previousImageHeight);
  }
}

bool ImageLoader::hasNext() const {
  return (m_imageFilePaths.size() > 0 &&
          m_currentIndex + 1 < m_imageFilePaths.size());
}

void ImageLoader::nextImage(const QPixmap &currentPixmap) {
  if (hasNext()) {

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]));

    m_previousPixmap = currentPixmap;
    m_previousImageWidth = m_currentImageWidth;
    m_previousImageHeight = m_currentImageHeight;

    m_currentImageWidth = m_nextImageWidth;
    m_currentImageHeight = m_nextImageHeight;

    m_currentIndex += 1;

    emit imageLoaded(fileInfo, m_nextPixmap, m_nextImageWidth,
                     m_nextImageHeight);

    if (m_currentIndex + 1 < m_imageFilePaths.size()) {
      loadImageIntoPixmap(
          QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]),
          m_nextPixmap, true, m_nextImageWidth, m_nextImageHeight);
    }
  }
}

void ImageLoader::goForward() {
  m_currentIndex += 10;

  const auto maxImageFiles = m_imageFilePaths.size() - 1;

  if (m_currentIndex > maxImageFiles) {
    m_currentIndex = maxImageFiles;
  }

  loadImage(QString::fromStdString(m_imageFilePaths[m_currentIndex]));
}

QString ImageLoader::getCurrentImageFilePath() {
  return QString::fromStdString(m_imageFilePaths[m_currentIndex]);
}

QPixmap ImageLoader::getCurrentImageFullRes() {
  auto imagePath = m_imageFilePaths[m_currentIndex];

  QPixmap imagePixmap;
  loadImageIntoPixmap(QString::fromStdString(imagePath), imagePixmap, false,
                      m_currentImageWidth, m_currentImageHeight);

  return imagePixmap;
}

void ImageLoader::deleteCurrentImage() {

  auto imagePath = m_imageFilePaths[m_currentIndex];

  // Delete image
  QFile file(QString::fromStdString(imagePath));
  file.moveToTrash();

  // Delete path from tracked list of paths
  m_imageFilePaths.erase(
      std::remove(m_imageFilePaths.begin(), m_imageFilePaths.end(), imagePath),
      m_imageFilePaths.end());

  // if possible, move to next image
  if (m_imageFilePaths.size() > 0 &&
      m_currentIndex <= m_imageFilePaths.size() - 1) {

    /// more images available
    /// currentIndex now points to the "next" image that
    /// should be shown
    /// no change needed to index

    /// This new image is what used to be nextPixmap
    /// So just emit that and update newPixmap
    /// previousPixmap remains the same

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex]));

    // Emit nextPixmap as current pixmap
    emit imageLoaded(fileInfo, m_nextPixmap, m_nextImageWidth,
                     m_nextImageHeight);

    m_currentImageWidth = m_nextImageWidth;
    m_currentImageHeight = m_nextImageHeight;

    if (m_currentIndex + 1 < m_imageFilePaths.size()) {
      // Prefetch load a new image into nextPixmap
      loadImageIntoPixmap(
          QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]),
          m_nextPixmap, true, m_nextImageWidth, m_nextImageHeight);
    }
  } else if (m_imageFilePaths.size() > 0) {
    /// Previous condition was not true

    /// At least one more image available in m_imageFilePaths
    /// We were at the last image in the list

    /// Set the previous pixmap as the new current

    m_currentIndex -= 1;

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex]));

    // Emit previousPixmap as current pixmap
    emit imageLoaded(fileInfo, m_previousPixmap, m_previousImageWidth,
                     m_previousImageHeight);

    m_currentImageWidth = m_previousImageWidth;
    m_currentImageHeight = m_previousImageHeight;

    // Prefetch load a new image into previousPixmap
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true, m_previousImageWidth, m_previousImageHeight);

  } else {
    emit noMoreImagesLeft();
  }
}

QString ImageLoader::getHeaderLabel() const {
  return QString("%1 / %2")
      .arg(m_currentIndex + 1)
      .arg(m_imageFilePaths.size());
}