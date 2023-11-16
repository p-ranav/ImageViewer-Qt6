#pragma once
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPinchGesture>
#include <QtWidgets>
#include <iostream>

class ImageViewer : public QGraphicsView {
    QGraphicsScene m_scene;
    QGraphicsPixmapItem m_item;
public:
    ImageViewer() {
        setScene(&m_scene);
        m_scene.addItem(&m_item);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }

void setPixmap(const QPixmap &pixmap, int desiredHeight) {
    // Reset transformation before setting the new pixmap
    m_item.resetTransform();

    // Set the QGraphicsPixmapItem's pixmap
    m_item.setPixmap(pixmap);

    // Enable smooth transformation by setting interpolation mode
    m_item.setTransformationMode(Qt::SmoothTransformation);

    // Calculate the scale factor to achieve the desired height
    qreal scaleFactor = static_cast<qreal>(desiredHeight) / pixmap.height();

    std::cout << desiredHeight << " vs " << pixmap.height() << std::endl;

    // Scale the QGraphicsPixmapItem
    QGraphicsView::resetTransform();
    scale(scaleFactor);

    // Center the QGraphicsPixmapItem in the window
    auto offset = -QRectF(pixmap.rect()).center();
    m_item.setOffset(offset);

    // Set the scene rect
    m_scene.setSceneRect(-pixmap.width() / 2.0, -pixmap.height() / 2.0,
                         pixmap.width(), pixmap.height());
}

  QPixmap pixmap() const {
    return m_item.pixmap();
  }

    void scale(qreal s) { QGraphicsView::scale(s, s); }
protected:

    bool event(QEvent *event) override {
        if (event->type() == QEvent::NativeGesture) {
            return nativeGestureEvent(static_cast<QNativeGestureEvent*>(event));
        } else {
            return QGraphicsView::event(event);
        }
    }

    bool nativeGestureEvent(QNativeGestureEvent *event) {
        if (event->gestureType() == Qt::ZoomNativeGesture) {
            qreal scaleFactor = event->value();

            if (scaleFactor > 0.0) {
                // Zooming in (zooming up)
                scale(1.05);
            } else if (scaleFactor < 0.0) {
                // Zooming out (zooming down)
                scale(0.95);
            }

            return true;
        }

        return false;
    }

    void keyPressEvent(QKeyEvent *event) override {
        if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_I) {
            scale(0.8);
        } else if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_P) {
            scale(1.2);
        } else {
            // Call the base class implementation for other key events
            QGraphicsView::keyPressEvent(event);
        }
    }
};