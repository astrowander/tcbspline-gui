#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->customPlot,SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mouseClickedOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleasedOverPlot(QMouseEvent*)));
    addNewGraph(selectionGraph, Qt::GlobalColor::green, 10.0);
    currentGraph=nullptr;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mouseMoveOverPlot(QMouseEvent *mouseEvent)
{
    selectionGraph->clearData();
    double key, value, eps;
    eps = 5*(ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower) /
           ui->customPlot->width() ;
    pixelsToCoords(mouseEvent->x(), mouseEvent->y(), key, value);

    switch (selectionMode)
    {
    case pointDragging:
    {
        selectionGraph->removeData(selectedKey);
        currentGraph->removeData(selectedKey);
        selectionGraph->addData(key, value);
        currentGraph->addData(key, value);
        selectedKey = key;
        break;
    }
    default:
    {
        selectionMode = noSelection;
        ui->label_test->setText(QString::number(key) + " " + QString::number(value));
        if (currentGraph == nullptr) return;
        for(QCPData data : *currentGraph->data())
        {
            if (fabs(data.key-key) < eps && fabs(data.value - value) < eps) {
                qDebug() << "Выделена точка: x = " << QString::number(data.key) << ", y = " << QString::number(data.value);
                selectionGraph->addData(data.key, data.value);
                selectionMode = pointSelected;
                selectedKey = data.key;
                break;
            }
        }
    }
    }
    ui->customPlot->replot();
}

void MainWindow::mouseClickedOverPlot(QMouseEvent *mouseEvent)
{
    switch (selectionMode)
    {
    case noSelection:
    {
        if (ui->customPlot->graphCount() == 1) {
            addNewGraph(currentGraph, Qt::GlobalColor::red, 5.0);
        }
        double key, value;
        pixelsToCoords(mouseEvent->x(), mouseEvent->y(), key, value);
        currentGraph->addData(key, value);
        break;
    }
    case pointSelected:
    {
        if (mouseEvent->button() == Qt::LeftButton) {
            selectionMode = pointDragging;
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            currentGraph->removeData(selectedKey);
            selectionGraph->clearData();
            selectionMode = noSelection;
        }
        break;
    }
    default:{}
    }
    ui->customPlot->replot();
}

void MainWindow::mouseReleasedOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode == pointDragging)
        selectionMode = pointSelected;
}

void MainWindow::pixelsToCoords(double x, double y, double &key, double &value)
{
    key = ui->customPlot->xAxis->pixelToCoord(x);
    value = ui->customPlot->yAxis->pixelToCoord(y);
}

void MainWindow::addNewGraph(QCPGraph* &graphPtr, Qt::GlobalColor color, double pointSize)
{
    ui->customPlot->addGraph(ui->customPlot->xAxis, ui->customPlot->yAxis);
    graphPtr = ui->customPlot->graph(ui->customPlot->graphCount() - 1);
    //lastGraph->setLineStyle(QCPGraph::LineStyle::lsNone);
    QCPScatterStyle scatterStyle;
    scatterStyle.setPen(QPen(QColor(color)));
    scatterStyle.setSize(pointSize);
    scatterStyle.setShape(QCPScatterStyle::ScatterShape::ssDisc);
    graphPtr->setScatterStyle(scatterStyle);
}
