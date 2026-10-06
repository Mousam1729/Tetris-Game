#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QMouseEvent>
#include <QEvent>

class my_label : public QLabel
{
    Q_OBJECT
public:
    explicit my_label(QWidget *parent = nullptr);
    void mouseMoveEvent(QMouseEvent *ev) override;
    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    int x, y;

signals:
    void Mouse_Pos();                    // left button pressed
    void Mouse_Released();               // left button released
    void sendMousePosition(QPoint&);     // mouse moved / pressed (label coordinates)
};

#endif // MY_LABEL_H