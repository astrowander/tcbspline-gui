#ifndef POINTREDACTOR_H
#define POINTREDACTOR_H

#include <QMouseEvent>
#include <QWidget>

namespace Ui {
class PointRedactor;
}

class PointRedactor : public QWidget
{
    Q_OBJECT

public:
    explicit PointRedactor(QWidget *parent = 0);
    ~PointRedactor();
    void setInterface(double tension, double continuity, double bias);

private slots:
    void on_slider_tension_valueChanged(int value);

    void on_lineEdit_tension_returnPressed();

    void on_slider_continuity_valueChanged(int value);

    void on_horizontalSlider_valueChanged(int value);

    void on_lineEdit_continuity_returnPressed();

    void on_lineEdit_returnPressed();

    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent *event);

private:
    Ui::PointRedactor *ui;
    double _tension, _continuity, _bias;
    QPoint offset;

signals:
    void parametersChanged(double tension, double continuity, double bias);
};

#endif // POINTREDACTOR_H
