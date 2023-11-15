#include "image_loader.hpp"

void ImageLoader::loadImage(const QString &imagePath, const QSize &targetSize) {

  QImageReader imageReader(imagePath);
  imageReader.setAutoTransform(true);

  QImage image = imageReader.read();
  if (image.isNull()) {
    /// TODO: Show warning message
    // QMessageBox::warning(this, "Error", "Failed to open the image.");
    return;
  }

  QPixmap imagePixmap = QPixmap::fromImage(image);
  auto width = imagePixmap.width();
  auto height = imagePixmap.height();
  auto imagePixmapScaled = imagePixmap.scaled(targetSize, Qt::KeepAspectRatio,
                                              Qt::SmoothTransformation);
  emit imageLoaded(imagePixmap, imagePixmapScaled, width, height);
}