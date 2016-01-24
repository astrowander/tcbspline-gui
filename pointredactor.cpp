#include "pointredactor.h"
#include "ui_pointredactor.h"

PointRedactor::PointRedactor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PointRedactor)
{
    ui->setupUi(this);    
}

PointRedactor::~PointRedactor()
{
    delete ui;
}

void PointRedactor::setInterface(double tension, double continuity, double bias)
{
    _tension = tension;
    _continuity = continuity;
    _bias = bias;
    ui->lineEdit_tension->setText(QString::number(_tension));
    ui->slider_tension->setValue(int(_tension*100 + 0.5));
    ui->lineEdit_continuity->setText(QString::number(_continuity));
    ui->slider_continuity->setValue(int(_continuity*100 + 0.5));
    ui->lineEdit->setText(QString::number(_tension));
    ui->horizontalSlider->setValue(int(_tension*100 + 0.5));
}

void PointRedactor::on_slider_tension_valueChanged(int value)
{
    _tension = value/100.0;
    ui->lineEdit_tension->setText(QString::number(_tension));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::on_lineEdit_tension_returnPressed()
{
    bool ok;
    _tension = ui->lineEdit_tension->text().toDouble(&ok);
    if (!ok) return;
    if (_tension < -1) _tension = -1;
    if (_tension > 1) _tension = 1;
    ui->lineEdit_tension->setText(QString::number(_tension));
    ui->slider_tension->setValue(int(_tension*100 + 0.5));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::on_slider_continuity_valueChanged(int value)
{
    _continuity = value/100.0;
    ui->lineEdit_continuity->setText(QString::number(_continuity));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::on_horizontalSlider_valueChanged(int value)
{
    _bias = value/100.0;
    ui->lineEdit->setText(QString::number(_bias));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::on_lineEdit_continuity_returnPressed()
{
    bool ok;
    _continuity = ui->lineEdit_continuity->text().toDouble(&ok);
    if (!ok) return;
    if (_continuity < -1) _continuity = -1;
    if (_continuity > 1) _continuity = 1;
    ui->lineEdit_continuity->setText(QString::number(_continuity));
    ui->slider_continuity->setValue(int(_continuity*100 + 0.5));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::on_lineEdit_returnPressed()
{
    bool ok;
    _bias = ui->lineEdit->text().toDouble(&ok);
    if (!ok) return;
    if (_bias < -1) _bias = -1;
    if (_bias > 1) _bias = 1;
    ui->lineEdit->setText(QString::number(_bias));
    ui->horizontalSlider->setValue(int(_bias*100 + 0.5));
    emit parametersChanged(_tension, _continuity, _bias);
}

void PointRedactor::mousePressEvent(QMouseEvent *event)
{
    offset = event->pos();
}

void PointRedactor::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        this->move(mapToParent(event->pos() - offset));
    }
}
