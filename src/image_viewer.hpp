#pragma once
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPinchGesture>
#include <QtWidgets>
#include <iostream>
#include <QMenu>
#include <QAction>
#include <QClipboard>
#include <optional>

class ImageViewer : public QGraphicsView {
  Q_OBJECT
  
  QGraphicsScene m_scene;
  QGraphicsPixmapItem m_item;

signals:
    void copyRequested();

private:
  std::optional<QSize> getMainWindowSize() const;

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
  void contextMenuEvent(QContextMenuEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent *event) override;
};
