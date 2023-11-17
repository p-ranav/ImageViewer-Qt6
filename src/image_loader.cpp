#include "image_loader.hpp"
#include <filesystem>
#include <iostream>
namespace fs = std::filesystem;

static unsigned entry_count{0};
void *callback_data = NULL;

/** Callback function handling an ExifEntry. */
static void content_foreach_func(ExifEntry *entry, void *callback_data) {
  char buf[2000];
  exif_entry_get_value(entry, buf, sizeof(buf));
  printf("    Entry %u: %s (%s)\n"
         "      Size, Comps: %d, %d\n"
         "      Value: %s\n",
         entry_count, exif_tag_get_name(entry->tag),
         exif_format_get_name(entry->format), entry->size,
         (int)(entry->components),
         exif_entry_get_value(entry, buf, sizeof(buf)));
  ++entry_count;
}

/** Callback function handling an ExifContent (corresponds 1:1 to an IFD). */
static void data_foreach_func(ExifContent *content, void *callback_data) {
  static unsigned content_count;
  entry_count = 0;
  printf("  Content %u: ifd=%d\n", content_count,
         exif_content_get_ifd(content));
  exif_content_foreach_entry(content, content_foreach_func, callback_data);
  ++content_count;
}

void readExifData(const char *filePath) {
  ExifData *d;
  unsigned char *buf;
  unsigned int len;

  const char *fn = filePath;
  std::cout << "File path: " << fn << "\n";
  d = exif_data_new_from_file(fn);
  if (!d) {
    fprintf(stderr, "Could not load data from '%s'!\n", fn);
    return;
  }

  exif_data_save_data(d, &buf, &len);

  if (!buf) {
    fprintf(stderr, "Could not extract EXIF data!\n");
    return;
  }

  exif_data_foreach_content(d, data_foreach_func, callback_data);

  exif_data_unref(d);
}

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

// void my_exif_parser_callback (void *context, int tag, int type, int len,
// unsigned int ord, void *ifp, long long)
// {
//   // // context - pointer to context passed to set_exifparser_handler();
//   // // tag - EXIF/Makernotes tag value
//   // // type - TIFF(EXIF) tag type
//   // // len - tag length
//   // // ord - byte order (II or MM)
//   // // void *ifp - pointer to LibRaw_abstract_datastream, positioned to tag
//   data

//     // std::cout << "Tag: " << tag << std::endl;
//     // std::cout << "Type: " << type << std::endl;
//     // std::cout << "Length: " << len << std::endl;
//     // std::cout << "Byte Order: " << (ord == 0x4949 ? "Little-endian" :
//     "Big-endian") << std::endl;

//   //   // Print the tag data (assuming it's a string)
//   //   char buffer[1024];
//   //   if (len < sizeof(buffer)) {
//   //       if (ifp) {
//   //         libraw_internal_data_t *stream =
//   static_cast<libraw_internal_data_t *>(ifp);
//   //           stream->read(buffer, len, 1);
//   //           buffer[len] = '\0'; // Null-terminate for printing as a string
//   //           std::cout << "Data: " << buffer << std::endl;
//   //       }
//   //   }
//   //   std::cout << "--------------------------------" << std::endl;
// }

ImageLoader::ImageLoader() : QObject() {
  // Register exif callback on m_rawProcessor
  // m_rawProcessor.set_exifparser_handler(my_exif_parser_callback, nullptr);

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
                          bool half_size) {
  m_rawProcessor.open_file(imagePath.toLocal8Bit().data());

  m_rawProcessor.imgdata.params.half_size = half_size ? 1 : 0;

  m_rawProcessor.unpack();
  m_rawProcessor.dcraw_process();

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

  QStringList rawFormats = {"nef", "cr2", "arw", "dng", "orf",
                            "pef", "rw2", "srw", "crw", "raf"};

  if (rawFormats.contains(fileInfo.suffix().toLower())) {
    loadRaw(imagePath, imagePixmap, half_size);
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

  // std::cout << "Uhhh.. " << imagePath.toStdString() << " "
  //           << fileInfo.filePath().toUtf8().constData() << "\n";
  // readExifData(fileInfo.filePath().toUtf8().constData());

  loadImagePathsIfEmpty(fileInfo.dir().absolutePath().toLocal8Bit().data(),
                        fileInfo.absoluteFilePath().toLocal8Bit().data());
  loadImageIntoPixmap(imagePath, imagePixmap, true);

  emit imageLoaded(fileInfo, imagePixmap);

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

bool ImageLoader::hasPrevious() const { return (m_currentIndex >= 1); }

void ImageLoader::previousImage(const QPixmap &currentPixmap) {
  if (hasPrevious()) {

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]));
    emit imageLoaded(fileInfo, m_previousPixmap);

    m_nextPixmap = currentPixmap;

    m_currentIndex -= 1;
    loadImageIntoPixmap(
        QString::fromStdString(m_imageFilePaths[m_currentIndex - 1]),
        m_previousPixmap, true);
  }
}

bool ImageLoader::hasNext() const {
  return (m_currentIndex + 1 < m_imageFilePaths.size());
}

void ImageLoader::nextImage(const QPixmap &currentPixmap) {
  if (hasNext()) {

    QFileInfo fileInfo(
        QString::fromStdString(m_imageFilePaths[m_currentIndex + 1]));
    emit imageLoaded(fileInfo, m_nextPixmap);

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