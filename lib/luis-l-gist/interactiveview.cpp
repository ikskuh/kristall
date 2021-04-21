#include "interactiveview.hpp"

#define VIEW_CENTER viewport()->rect().center()
#define VIEW_WIDTH viewport()->rect().width()
#define VIEW_HEIGHT viewport()->rect().height()

InteractiveView::InteractiveView(QWidget * parent) :
    QGraphicsView(parent)
{
//    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setMaxSize();
    centerOn(0, 0);

    zoomDelta = 0.1;
    panSpeed = 4;
    _doMousePanning = false;
    _doKeyZoom = false;
    _scale = 1.0;

    panButton = Qt::MiddleButton;
    zoomKey = Qt::Key_Z;

    /* QGraphicsView turns it on for some reason,
     * switching it back off to let BrowserTab handle it -krkk */
    setAcceptDrops(false);
}

qreal InteractiveView::getScale() const
{
    return _scale;
}

void InteractiveView::keyPressEvent(QKeyEvent * event)
{
    qint32 key = event->key();

    if(key == zoomKey){
        _doKeyZoom = true;
    }

    if (_doKeyZoom){
        if (key == Qt::Key_Up)
            zoomIn();

        else if (key == Qt::Key_Down)
            zoomOut();
    }

    else{
        QGraphicsView::keyPressEvent(event);
    }
}

void InteractiveView::keyReleaseEvent(QKeyEvent * event)
{
    if (event->key() == zoomKey){
        _doKeyZoom = false;
    }

    QGraphicsView::keyReleaseEvent(event);
}

void InteractiveView::mouseMoveEvent(QMouseEvent * event)
{
    if (_doMousePanning){
        QPointF mouseDelta = mapToScene(event->pos()) - mapToScene(_lastMousePos);
        pan(mouseDelta);
    }

    QGraphicsView::mouseMoveEvent(event);
    _lastMousePos = event->pos();
}

void InteractiveView::mousePressEvent(QMouseEvent * event)
{
    if (event->button() == panButton){
        _lastMousePos = event->pos();
        _doMousePanning = true;
    }

    QGraphicsView::mousePressEvent(event);
}

void InteractiveView::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->button() == panButton){
        _doMousePanning = false;
    }

    QGraphicsView::mouseReleaseEvent(event);
}

void InteractiveView::wheelEvent(QWheelEvent *event)
{
    QPoint scrollAmount = event->angleDelta();

    // Apply zoom.
    scrollAmount.y() > 0 ? zoomIn() : zoomOut();
}

void InteractiveView::setMaxSize()
{
    setSceneRect(INT_MIN/2, INT_MIN/2, INT_MAX, INT_MAX);
}

void InteractiveView::zoom(float scaleFactor)
{
    scale(scaleFactor, scaleFactor);
    _scale *= scaleFactor;
}

void InteractiveView::zoomIn()
{
    zoom(1 + zoomDelta);
}

void InteractiveView::zoomOut()
{
    zoom (1 - zoomDelta);
}

void InteractiveView::pan(QPointF delta)
{
    // Scale the pan amount by the current zoom.
    delta *= _scale;
    delta *= panSpeed;

    // Have panning be anchored from the mouse.
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    QPoint newCenter(VIEW_WIDTH / 2 - delta.x(),  VIEW_HEIGHT / 2 - delta.y());
    centerOn(mapToScene(newCenter));

    // For zooming to anchor from the view center.
    setTransformationAnchor(QGraphicsView::AnchorViewCenter);
}
