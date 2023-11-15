#pragma once
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QThread>
#include <QImageReader>
#include <QImage>
#include <QMessageBox>

class ImageLoader : public QObject {
  Q_OBJECT

public slots:
  void loadImage(const QString &imagePath, const QSize &targetSize);

signals:
  void imageLoaded(const QPixmap &imagePixmap, const QPixmap &imagePixmapScaled, int width, int height);
};