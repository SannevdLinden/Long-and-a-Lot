#include "graphicsviewzoom.h"
#include <QWheelEvent>

GraphicsViewZoom::GraphicsViewZoom(QWidget *parent):
    QGraphicsView(parent)
{

}

void GraphicsViewZoom::wheelEvent(QWheelEvent *event){
    float angle = event->angleDelta().y();
    if(angle > 0)
    {
       scale(1.1,1.1);
    } else
    {
      scale(0.9,0.9);
    }
}


void GraphicsViewZoom::mouseReleaseEvent(QMouseEvent *mouseEvent){
    if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::ControlModifier){
            emit boxSelection(false);
    }
    QGraphicsView::mouseReleaseEvent(mouseEvent);
}

void GraphicsViewZoom::mousePressEvent(QMouseEvent* mouseEvent)
{
    if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::ControlModifier){
            emit boxSelection(true);
    }

    QGraphicsView::mousePressEvent(mouseEvent);
}


