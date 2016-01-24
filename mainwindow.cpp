#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), currentGraph(nullptr), spline(nullptr), copiedSpline(nullptr)
{
    ui->setupUi(this);
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(undo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y), this, SLOT(redo()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(inputFromFile()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(outputToFile()));
    connect(ui->customPlot,SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mouseClickedOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleasedOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheeledOverPlot(QWheelEvent*)));
    connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(mouseDoubleClickedOverPlot(QMouseEvent*)));
    addNewGraph(selectionGraph, QCPCurve::LineStyle::lsNone, Qt::GlobalColor::green, 10.0);
    addNewGraph(interpolatedGraph, QCPCurve::LineStyle::lsLine, Qt::GlobalColor::black);
    addNewGraph(currentGraph,  QCPCurve::LineStyle::lsNone, Qt::GlobalColor::red, 5.0);
    spline = new TSBSpline(currentGraph);
}

MainWindow::~MainWindow()
{
    delete spline;
    delete ui;
}

void MainWindow::mouseMoveOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode!=pointRedacting) selectionGraph->clearData();
    double key, value, eps;
    eps = 5*(ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower) /
           ui->customPlot->width() ;
    pixelsToCoords(mouseEvent->x(), mouseEvent->y(), key, value);
    ui->statusBar->showMessage("x = " + QString::number(key) + ", y = " + QString::number(value));
    switch (selectionMode)
    {
    case pointDragging:
    {
        selectionGraph->removeData(selectedKey);
        copiedGraph->removeData(selectedKey);
        selectionGraph->addData(selectedKey, key, value);
        copiedGraph->addData(selectedKey, key, value);
        copiedSpline->setPoint(int(selectedKey), key, value);
        break;
    }
    case plotDragging:
    {
        ui->customPlot->xAxis->moveRange(oldX - key);
        ui->customPlot->yAxis->moveRange(oldY - value);
        oldX = key;
        oldY = value;
        break;
    }
    case pointRedacting:
    {
        break;
    }
    default:
    {
        selectionMode = noSelection;

        if (currentGraph == nullptr) return;
        int nSelected = spline->searchByKeyValue(key, value, eps);
        if (nSelected!=-1) {
            TSBPoint& point = spline->getPointAt(nSelected);
            selectionGraph->addData(point.t(), point.x(), point.y());
            selectionMode = pointSelected;
            selectedKey = point.t();
        }
        break;
    }
    }
    replot();
}

void MainWindow::mouseClickedOverPlot(QMouseEvent *mouseEvent)
{
    switch (selectionMode)
    {
    case noSelection:
    {
        double key, value;
        pixelsToCoords(mouseEvent->x(), mouseEvent->y(), key, value);
        if (mouseEvent->button() == Qt::LeftButton) {
            spline->addAction(addPoint, spline->size(), key, value);
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            oldX = key;
            oldY = value;
            selectionMode = plotDragging;
        }
        break;
    }
    case pointSelected:
    {
        if (mouseEvent->button() == Qt::LeftButton) {
            oldX = currentGraph->data()->value(selectedKey).key;
            oldY = currentGraph->data()->value(selectedKey).value;
            addNewGraph(copiedGraph,  QCPCurve::LineStyle::lsNone, QColor(255,255,255, 128), 5.0);
            addNewGraph(copiedInterpolatedGraph, QCPCurve::LineStyle::lsLine, QColor(0,0,0,128));
            copiedGraph->addData(spline->getTimes(), spline->getKeys(), spline->getValues());
            copiedSpline = new TSBSpline(copiedGraph);
            copiedSpline->setPoints(spline->getPoints());
            selectionMode = pointDragging;
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            //currentGraph->removeData(selectedKey);
            spline->addAction(deletePoint, selectedKey, currentGraph->data()->value(selectedKey).key, currentGraph->data()->value(selectedKey).value, spline->getPointAt(selectedKey).tension(), spline->getPointAt(selectedKey).continuity(), spline->getPointAt(selectedKey).bias());
            selectionGraph->clearData();
            selectionMode = noSelection;
        }
        break;
    }
    case  pointRedacting:
    {
        pointRedactor->hide();
        delete pointRedactor;
        spline->addAction(redactPoint, selectedKey, oldX, oldY, spline->getPointAt(selectedKey).tension(),  spline->getPointAt(selectedKey).continuity(), spline->getPointAt(selectedKey).bias(), 0, 0,
                          copiedSpline->getPointAt(selectedKey).tension() - spline->getPointAt(selectedKey).tension(), copiedSpline->getPointAt(selectedKey).continuity() - spline->getPointAt(selectedKey).continuity(), copiedSpline->getPointAt(selectedKey).bias() - spline->getPointAt(selectedKey).bias());
        ui->customPlot->removePlottable(copiedGraph);
        ui->customPlot->removePlottable(copiedInterpolatedGraph);
        delete copiedSpline;
        copiedSpline = nullptr;
        selectionMode = noSelection;
        selectionGraph->clearData();
    }
    default:{}
    }
    replot();
}

void MainWindow::mouseDoubleClickedOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode == pointSelected) {
        selectionMode = pointRedacting;
        pointRedactor = new PointRedactor(this);
        pointRedactor->setGeometry(50, 50, 160, 240);
        pointRedactor->show();

        addNewGraph(copiedGraph,  QCPCurve::LineStyle::lsNone, QColor(255,255,255, 128), 5.0);
        addNewGraph(copiedInterpolatedGraph, QCPCurve::LineStyle::lsLine, QColor(0,0,0,128));
        copiedGraph->addData(spline->getTimes(), spline->getKeys(), spline->getValues());
        copiedSpline = new TSBSpline(copiedGraph);
        copiedSpline->setPoints(spline->getPoints());

        connect(pointRedactor, SIGNAL(parametersChanged(double, double, double)), this, SLOT(parametersChanged(double, double, double)));
        pointRedactor->setInterface(spline->getPointAt(selectedKey).tension(), spline->getPointAt(selectedKey).continuity(), spline->getPointAt(selectedKey).bias());
    }
}

void MainWindow::mouseReleasedOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode == pointDragging) {
        spline->addAction(movePoint, selectedKey, oldX, oldY, spline->getPointAt(selectedKey).tension(), spline->getPointAt(selectedKey).continuity(), spline->getPointAt(selectedKey).bias(), copiedGraph->data()->value(selectedKey).key - oldX, copiedGraph->data()->value(selectedKey).value - oldY);
        ui->customPlot->removePlottable(copiedGraph);
        ui->customPlot->removePlottable(copiedInterpolatedGraph);
        delete copiedSpline;
        copiedSpline = nullptr;
        selectionMode = pointSelected;       
    }
    else if (selectionMode == plotDragging) {
        selectionMode = noSelection;
    }

    replot();
}

void MainWindow::mouseWheeledOverPlot(QWheelEvent *mouseEvent)
{
    double xcenter, ycenter, xscale, yscale;
    pixelsToCoords(mouseEvent->x(), mouseEvent->y(), xcenter, ycenter);
    xscale = (ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower) * pow(1.1765, -mouseEvent->delta()/120);
    yscale = (ui->customPlot->yAxis->range().upper - ui->customPlot->yAxis->range().lower) * pow(1.1765, -mouseEvent->delta()/120);
    ui->customPlot->xAxis->setRange(xcenter - xscale / 2, xcenter + xscale / 2);
    ui->customPlot->yAxis->setRange(ycenter - yscale / 2, ycenter + yscale / 2);
    replot();
}

void MainWindow::undo()
{
    spline->undo();
    replot();
}

void MainWindow::redo()
{
    spline->redo();
    replot();
}

void MainWindow::pixelsToCoords(double x, double y, double &key, double &value)
{
    key = ui->customPlot->xAxis->pixelToCoord(x);
    value = ui->customPlot->yAxis->pixelToCoord(y);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->customPlot->setGeometry(10, 10, event->size().width() - 20, event->size().height() -70);
}

void MainWindow::addNewGraph(QCPCurve* &graphPtr, QCPCurve::LineStyle lineStyle, QColor color = Qt::GlobalColor::red, double pointSize)
{
    //ui->customPlot->addPlottable(graphPtr);
    graphPtr = new QCPCurve(ui->customPlot->xAxis, ui->customPlot->yAxis);
    graphPtr->setLineStyle(lineStyle);
    graphPtr->setPen(QPen(QColor(color)));
    QCPScatterStyle scatterStyle;
    scatterStyle.setPen(QPen(QColor(color)));
    scatterStyle.setSize(pointSize);
    scatterStyle.setShape(QCPScatterStyle::ScatterShape::ssDisc);
    graphPtr->setScatterStyle(scatterStyle);
    ui->customPlot->addPlottable(graphPtr);
}

void MainWindow::replot()
{
    spline->makeInterpolatedGraph(interpolatedGraph, ui->customPlot->xAxis, ui->customPlot->yAxis);
    if (copiedSpline!=nullptr) copiedSpline->makeInterpolatedGraph(copiedInterpolatedGraph, ui->customPlot->xAxis, ui->customPlot->yAxis);
    ui->customPlot->replot();
}

void MainWindow::parametersChanged(double tension, double continuity, double bias)
{
    copiedSpline->setParametersAt(int(selectedKey), tension, continuity, bias);
    replot();
}

void MainWindow::inputFromFile()
{
    QVector<TSBPoint> points;
    currentGraph->clearData();
    QFile inputFile(QFileDialog::getOpenFileName(this, "Выберите файл для загрузки данных", "/home/astrowander/libs", "CSV files (*.csv)"));
    if (!inputFile.open(QIODevice::ReadOnly)) {
        msgBox("Невозможно открыть файл");
        return;
    }
    int i = -1;
    while (!inputFile.atEnd()) {
        QString ss = inputFile.readLine();
        QStringList words = ss.split(";");
        if (words.size() < 6) {
            msgBox( "Данные в файле неверного формата");
            return;
        }

        bool ok;
        double t = words[0].toDouble(&ok), key = words[1].toDouble(&ok), value = words[2].toDouble(&ok);
        double tension = words[3].toDouble(&ok), continuity = words[4].toDouble(&ok), bias = words[5].toDouble(&ok);
        if (!ok) {
            msgBox( "Данные в файле неверного формата");
            return;
        }
        points.append(TSBPoint(key, value, t, tension, continuity, bias));
        currentGraph->addData(t, key, value);
    }
    spline->clearActions();
    spline->setPoints(points);
    ui->customPlot->rescaleAxes();
    replot();
}

void MainWindow::outputToFile()
{
    QFile outputFile(QFileDialog::getSaveFileName(this, "Выберите файл для сохранения данных", "/home/astrowander/libs", "CSV files (*.csv)"));
    QTextStream outputStream(&outputFile);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        msgBox("Невозможно открыть файл");
        return;
    }
    QVector<TSBPoint>& points = spline->getPoints();
    for (int i=0; i<points.size(); ++i) {
        outputStream << points[i].print();
    }
    outputFile.close();
}



void msgBox(const QString &prompt)
{
    QMessageBox messageBox;
    messageBox.setText(prompt);
    messageBox.exec();
}
