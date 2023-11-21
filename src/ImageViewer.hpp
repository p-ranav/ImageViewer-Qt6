#pragma once
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPinchGesture>
#include <QtWidgets>
#include <QMenu>
#include <QAction>
#include <QClipboard>

#include <iostream>
#include <optional>

class ImageViewer : public QGraphicsView {
  Q_OBJECT
  
  QGraphicsScene m_scene;
  QGraphicsPixmapItem m_item;

  static constexpr inline qreal ZOOM_IN_SCALE = 1.04;
  static constexpr inline qreal ZOOM_OUT_SCALE = 0.96;

signals:
    void copyRequested();
    void deleteRequested();

public:
  ImageViewer(QWidget *parent = nullptr);
  void setPixmap(const QPixmap &pixmap, int desiredWidth, int desiredHeight);
  QPixmap pixmap() const;
  void scale(qreal s);
  void resize(int desiredWidth, int desiredHeight);
  void zoomIn();
  void zoomOut();

protected:
  bool event(QEvent *event) override;
  void wheelEvent(QWheelEvent *event) override;
  bool nativeGestureEvent(QNativeGestureEvent *event);
  void keyPressEvent(QKeyEvent *event) override;
};
