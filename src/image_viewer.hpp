#pragma once
#include <QLabel>
#include <QPixmap>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QPoint>
#include <QPinchGesture>
#include <QtWidgets>

class ImageViewer : public QGraphicsView {
    QGraphicsScene m_scene;
    QGraphicsPixmapItem m_item;
	qreal m_widthScale;
	qreal m_heightScale;
public:
    ImageViewer() {
        setScene(&m_scene);
        m_scene.addItem(&m_item);
        setDragMode(QGraphicsView::ScrollHandDrag);
        setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        setResizeAnchor(QGraphicsView::AnchorViewCenter);
    }
    void setPixmap(const QPixmap &pixmap) {
        m_item.setPixmap(pixmap);

        // Enable smooth transformation by setting interpolation mode
        m_item.setTransformationMode(Qt::SmoothTransformation);
        m_item.pixmap().setDevicePixelRatio(devicePixelRatioF());
        m_item.pixmap().setDevicePixelRatio(pixmap.devicePixelRatioF());
        m_item.pixmap().setDevicePixelRatio(1.0);
        m_item.pixmap().setDevicePixelRatio(devicePixelRatioF());

        auto offset = -QRectF(pixmap.rect()).center();
        m_item.setOffset(offset);
        setSceneRect(offset.x()*4, offset.y()*4, -offset.x()*8, -offset.y()*8);
        translate(1, 1);

		QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();
		QSize canvasSize = primaryScreenGeometry.size() * 0.80;

		// Convert width to qreal
		qreal widthAsQReal = static_cast<qreal>(canvasSize.width());

		// Convert height to qreal
		qreal heightAsQReal = static_cast<qreal>(canvasSize.height());

		m_heightScale = heightAsQReal / pixmap.height();
		m_widthScale = widthAsQReal / pixmap.width();

		scale(m_heightScale);
    }
    void scale(qreal s) { QGraphicsView::scale(s, s); }
    QSize sizeHint() const override { return {400, 400}; }
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