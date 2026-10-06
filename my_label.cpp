#include "my_label.h"

my_label::my_label(QWidget *parent) : QLabel(parent), x(0), y(0)
{
    this->setMouseTracking(true);
}

void my_label::mouseMoveEvent(QMouseEvent *ev)
{
    QPoint pos = ev->position().toPoint();
    if (pos.x() >= 0 && pos.y() >= 0 && pos.x() < this->width() && pos.y() < this->height()) {
        emit sendMousePosition(pos);
    }
}

void my_label::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        QPoint pos = ev->position().toPoint();
        x = pos.x();
        y = pos.y();
        emit sendMousePosition(pos);   // make sure the logical position is fresh
        emit Mouse_Pos();
    }
}

void my_label::mouseReleaseEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton) {
        emit Mouse_Released();
    }
}