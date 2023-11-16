#include "image_loader.hpp"
#include <iostream>

void ImageLoader::loadImage(const QString &imagePath, const QSize &targetSize) {

  QFileInfo fileInfo(imagePath);
  QPixmap imagePixmap;

  if (fileInfo.suffix().toLower() == "nef") {

    m_rawProcessor.open_file(imagePath.toLocal8Bit().data());
    m_rawProcessor.unpack();
    m_rawProcessor.dcraw_process();

    libraw_processed_image_t *processed_image =
        m_rawProcessor.dcraw_make_mem_image();

    QImage image(processed_image->data, processed_image->width,
                 processed_image->height, QImage::Format_RGB888);

    // Convert QImage to QPixmap and display it
    imagePixmap = QPixmap::fromImage(image);

    m_rawProcessor.dcraw_clear_mem(processed_image);
  } else {
    QImageReader imageReader(imagePath);
    imageReader.setAutoTransform(true);

    QImage image = imageReader.read();
    if (image.isNull()) {
      /// TODO: Show warning message
      // QMessageBox::warning(this, "Error", "Failed to open the image.");
      return;
    }

    imagePixmap = QPixmap::fromImage(image);
  }

  emit imageLoaded(imagePixmap);
}