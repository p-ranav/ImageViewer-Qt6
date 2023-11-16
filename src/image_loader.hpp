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

class ImageLoader : public QObject {
  Q_OBJECT
 
  LibRaw m_rawProcessor;

public slots:
  void loadImage(const QString &imagePath, const QSize &targetSize);

signals:
  void imageLoaded(const QPixmap &imagePixmap, const QPixmap &imagePixmapScaled, int width, int height);
};