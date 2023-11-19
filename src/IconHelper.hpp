#pragma once
#include <QIcon>
#include <QString>
#include <QPixmap>
#include <QPainter>

static inline QIcon createColorIcon(const QString &imagePath, const QColor &color,
                                  int size) {
  QPixmap pixmap(imagePath);
  pixmap =
      pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);

  return QIcon(pixmap);
}