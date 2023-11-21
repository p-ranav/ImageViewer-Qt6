#include "ImageLoader.hpp"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

std::vector<QString> getImageFiles(const char *directory) {
  std::vector<QString> imageFiles;

  for (const auto &entry : fs::directory_iterator(directory)) {
    if (entry.is_regular_file()) {

      std::string extension = entry.path().extension();
      std::transform(extension.begin(), extension.end(), extension.begin(),
                     ::tolower);

      const std::vector<std::string> allowedExtensions = {
          ".jpg", ".jpeg", ".png", ".nef", ".heic", ".tiff", ".webp"};

      if (std::find(allowedExtensions.begin(), allowedExtensions.end(),
                    extension) != allowedExtensions.end()) {
        imageFiles.push_back(QString::fromStdString(entry.path().string()));
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

ImageInfo ImageLoader::loadRaw(const QString &imagePath, QPixmap &imagePixmap,
                               bool half_size) {

  ImageInfo result;

  m_rawProcessor.open_file(imagePath.toLocal8Bit().data());

  m_rawProcessor.imgdata.params.half_size = half_size ? 1 : 0;

  m_rawProcessor.unpack();
  m_rawProcessor.dcraw_process();

  // Access the image resolution
  result.width = m_rawProcessor.imgdata.sizes.raw_width;
  result.height = m_rawProcessor.imgdata.sizes.raw_height;

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

  return result;
}

ImageInfo ImageLoader::loadWithImageReader(const QString &imagePath,
                                           QPixmap &imagePixmap) {

  ImageInfo result;

  QImageReader imageReader(imagePath);
  imageReader.setAllocationLimit(0);
  imageReader.setAutoTransform(true);

  QImage image = imageReader.read();
  if (image.isNull()) {
    /// TODO: Show warning message
    // QMessageBox::warning(this, "Error", "Failed to open the image.");
  } else {
    imagePixmap = QPixmap::fromImage(image);
    result.width = imagePixmap.width();
    result.height = imagePixmap.height();
  }

  return result;
}

ImageInfo ImageLoader::loadImageIntoPixmap(const QString &imagePath,
                                           QPixmap &imagePixmap,
                                           bool half_size) {
  QFileInfo fileInfo(imagePath);

  QStringList rawFormats = {"nef", "cr2", "arw", "dng", "orf",
                            "pef", "rw2", "srw", "crw", "raf"};

  if (rawFormats.contains(fileInfo.suffix().toLower())) {
    return loadRaw(imagePath, imagePixmap, half_size);
  } else {
    return loadWithImageReader(imagePath, imagePixmap);
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
  m_currentImageInfo = loadImageIntoPixmap(imagePath, imagePixmap, true);

  emit imageLoaded(fileInfo, imagePixmap, m_currentImageInfo);

  // Prefetch next and previous images
  if (m_currentIndex >= 1) {
    m_previousImageInfo = loadImageIntoPixmap(
        m_imageFilePaths[m_currentIndex - 1], m_previousPixmap, true);
  }
  if (m_currentIndex + 1 < m_imageFilePaths.size()) {
    m_nextImageInfo = loadImageIntoPixmap(m_imageFilePaths[m_currentIndex + 1],
                                          m_nextPixmap, true);
  }
}

void ImageLoader::goToStart() {
  m_currentIndex = 0;
  loadImage(m_imageFilePaths[m_currentIndex]);
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

  loadImage(m_imageFilePaths[m_currentIndex]);
}

void ImageLoader::previousImage(const QPixmap &currentPixmap) {
  if (hasPrevious()) {

    QFileInfo fileInfo(m_imageFilePaths[m_currentIndex - 1]);

    m_nextPixmap = currentPixmap;
    m_nextImageInfo = m_currentImageInfo;
    m_currentImageInfo = m_previousImageInfo;

    m_currentIndex -= 1;

    emit imageLoaded(fileInfo, m_previousPixmap, m_previousImageInfo);

    m_previousImageInfo = loadImageIntoPixmap(
        m_imageFilePaths[m_currentIndex - 1], m_previousPixmap, true);
  }
}

bool ImageLoader::hasNext() const {
  return (m_imageFilePaths.size() > 0 &&
          m_currentIndex + 1 < m_imageFilePaths.size());
}

void ImageLoader::nextImage(const QPixmap &currentPixmap) {
  if (hasNext()) {

    QFileInfo fileInfo(m_imageFilePaths[m_currentIndex + 1]);

    m_previousPixmap = currentPixmap;
    m_previousImageInfo = m_currentImageInfo;
    m_currentImageInfo = m_nextImageInfo;

    m_currentIndex += 1;

    emit imageLoaded(fileInfo, m_nextPixmap, m_nextImageInfo);

    if (m_currentIndex + 1 < m_imageFilePaths.size()) {
      m_nextImageInfo = loadImageIntoPixmap(
          m_imageFilePaths[m_currentIndex + 1], m_nextPixmap, true);
    }
  }
}

void ImageLoader::goForward() {
  m_currentIndex += 10;

  const auto maxImageFiles = m_imageFilePaths.size() - 1;

  if (m_currentIndex > maxImageFiles) {
    m_currentIndex = maxImageFiles;
  }

  loadImage(m_imageFilePaths[m_currentIndex]);
}

QString ImageLoader::getCurrentImageFilePath() {
  return m_imageFilePaths[m_currentIndex];
}

QPixmap ImageLoader::getCurrentImageFullRes() {
  auto imagePath = m_imageFilePaths[m_currentIndex];

  QPixmap imagePixmap;
  m_currentImageInfo = loadImageIntoPixmap(imagePath, imagePixmap, false);

  return imagePixmap;
}

void ImageLoader::deleteCurrentImage() {

  auto imagePath = m_imageFilePaths[m_currentIndex];

  // Delete image
  QFile file(imagePath);
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

    QFileInfo fileInfo(m_imageFilePaths[m_currentIndex]);

    // Emit nextPixmap as current pixmap
    emit imageLoaded(fileInfo, m_nextPixmap, m_nextImageInfo);

    m_currentImageInfo = m_nextImageInfo;

    if (m_currentIndex + 1 < m_imageFilePaths.size()) {
      // Prefetch load a new image into nextPixmap
      m_nextImageInfo = loadImageIntoPixmap(
          m_imageFilePaths[m_currentIndex + 1], m_nextPixmap, true);
    }
  } else if (m_imageFilePaths.size() > 0) {
    /// Previous condition was not true

    /// At least one more image available in m_imageFilePaths
    /// We were at the last image in the list

    /// Set the previous pixmap as the new current

    m_currentIndex -= 1;

    QFileInfo fileInfo(m_imageFilePaths[m_currentIndex]);

    // Emit previousPixmap as current pixmap
    emit imageLoaded(fileInfo, m_previousPixmap, m_previousImageInfo);

    m_currentImageInfo = m_previousImageInfo;

    // Prefetch load a new image into previousPixmap
    m_previousImageInfo = loadImageIntoPixmap(
        m_imageFilePaths[m_currentIndex - 1], m_previousPixmap, true);

  } else {
    emit noMoreImagesLeft();
  }
}

void ImageLoader::sortAscending() {
  if (m_currentSortOrderAscending) {
    // Do nothing
  } else {
    m_currentSortOrderAscending = true;

    const auto currentImagePath = m_imageFilePaths[m_currentIndex];

    // Sort descending order
    std::sort(m_imageFilePaths.begin(), m_imageFilePaths.end());

    // update m_currentIndex
    updateCurrentIndexAfterSort(currentImagePath);

    // loadImage and prefetch new next/prev images
    loadImage(m_imageFilePaths[m_currentIndex]);
  }
}

void ImageLoader::updateCurrentIndexAfterSort(const QString &currentImagePath) {
  auto it = std::find(m_imageFilePaths.begin(), m_imageFilePaths.end(),
                      currentImagePath);
  int index = (it != m_imageFilePaths.end())
                  ? std::distance(m_imageFilePaths.begin(), it)
                  : -1;

  if (index == -1) {
    // bad
  } else {
    m_currentIndex = index;
  }
}

void ImageLoader::sortDescending() {
  if (m_currentSortOrderAscending) {
    m_currentSortOrderAscending = false;

    const auto currentImagePath = m_imageFilePaths[m_currentIndex];

    // Sort descending order
    std::sort(m_imageFilePaths.rbegin(), m_imageFilePaths.rend());

    // update m_currentIndex
    updateCurrentIndexAfterSort(currentImagePath);

    // loadImage and prefetch new next/prev images
    loadImage(m_imageFilePaths[m_currentIndex]);
  }
}

QString ImageLoader::getHeaderLabel() const {
  return QString("%1 / %2")
      .arg(m_currentIndex + 1)
      .arg(m_imageFilePaths.size());
}