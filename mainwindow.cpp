#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), currentGraph(nullptr), spline(nullptr)
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
    addNewGraph(selectionGraph, QCPGraph::LineStyle::lsNone, Qt::GlobalColor::green, 10.0);
    addNewGraph(interpolatedGraph, QCPGraph::LineStyle::lsLine, Qt::GlobalColor::black);
    addNewGraph(currentGraph,  QCPGraph::LineStyle::lsNone, Qt::GlobalColor::red, 5.0);
    spline = new TSBSpline(currentGraph);
}

MainWindow::~MainWindow()
{
    delete spline;
    delete ui;
}

void MainWindow::mouseMoveOverPlot(QMouseEvent *mouseEvent)
{
    selectionGraph->clearData();
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
        selectionGraph->addData(key, value);
        copiedGraph->addData(key, value);
        selectedKey = key;
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
    default:
    {
        selectionMode = noSelection;

        if (currentGraph == nullptr) return;
        for(QCPData data : *currentGraph->data())
        {
            if (fabs(data.key-key) < eps && fabs(data.value - value) < eps) {
                selectionGraph->addData(data.key, data.value);
                selectionMode = pointSelected;
                selectedKey = data.key;
                break;
            }
        }
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
            spline->addAction(addPoint, key, value);
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
            oldX = selectedKey;
            oldY = currentGraph->data()->value(selectedKey).value;
            selectionMode = pointDragging;
            addNewGraph(copiedGraph, QCPGraph::LineStyle::lsNone, Qt::GlobalColor::red, 5.0);
            copiedGraph->setData(spline->getKeys(), spline->getValues());
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            //currentGraph->removeData(selectedKey);
            spline->addAction(deletePoint, selectedKey, currentGraph->data()->value(selectedKey).value);
            selectionGraph->clearData();
            selectionMode = noSelection;
        }
        break;
    }
    default:{}
    }
    replot();
}

void MainWindow::mouseReleasedOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode == pointDragging) {
        spline->addAction(movePoint, oldX, oldY, selectedKey - oldX, copiedGraph->data()->value(selectedKey).value - oldY);
        selectionMode = pointSelected;
        ui->customPlot->removeGraph(copiedGraph);
    }
    else if (selectionMode == plotDragging) {
        selectionMode = noSelection;
    }
    replot();
}

void MainWindow::mouseWheeledOverPlot(QWheelEvent *mouseEvent)
{
    qDebug() << ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower;
    qDebug() << mouseEvent->delta();
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

}

void MainWindow::addNewGraph(QCPGraph* &graphPtr, QCPGraph::LineStyle lineStyle, Qt::GlobalColor color = Qt::GlobalColor::red, double pointSize)
{
    ui->customPlot->addGraph(ui->customPlot->xAxis, ui->customPlot->yAxis);
    graphPtr = ui->customPlot->graph(ui->customPlot->graphCount() - 1);
    graphPtr->setLineStyle(lineStyle);
    graphPtr->setPen(QPen(QColor(color)));
    QCPScatterStyle scatterStyle;
    scatterStyle.setPen(QPen(QColor(color)));
    scatterStyle.setSize(pointSize);
    scatterStyle.setShape(QCPScatterStyle::ScatterShape::ssDisc);
    graphPtr->setScatterStyle(scatterStyle);
}

void MainWindow::replot()
{
    interpolatedGraph->clearData();
    if (spline->getKeys().size() >= 2)
    {
        double key, value;
        for (int i = 0; i < ui->customPlot->width(); ++i)
        {
            key = ui->customPlot->xAxis->pixelToCoord(i);
            if (key < spline->getKeys().first() || key > spline->getKeys().last()) continue;
            value = spline->interpolate(key);
            interpolatedGraph->addData(key, value);
        }
    }
    ui->customPlot->replot();
}

void MainWindow::inputFromFile()
{
    QVector<double> keys, values;
    QFile inputFile(QFileDialog::getOpenFileName(this, "Выберите файл для загрузки данных", "/home/astrowander/libs", "CSV files (*.csv)"));
    if (!inputFile.open(QIODevice::ReadOnly)) {
        msgBox("Невозможно открыть файл");
        return;
    }
    while (!inputFile.atEnd()) {
        QString ss = inputFile.readLine();
        QStringList words = ss.split(";");
        if (words.size() < 2) {
            msgBox( "Данные в файле неверного формата");
            return;
        }

        bool ok;
        double key = words[0].toDouble(&ok), value = words[1].toDouble(&ok);
        if (!ok) {
            msgBox( "Данные в файле неверного формата");
            return;
        }
        keys.append(key);
        values.append(value);
    }
    spline->setKeys(keys);
    spline->setValues(values);
    currentGraph->setData(keys, values);
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
    QVector<double> keys(spline->getKeys());
    QVector<double> values(spline->getValues());
    for (int i=0; i<keys.size(); ++i) {
        outputStream << QString::number(keys[i]) << ";" << QString::number(values[i]) << "\n";
    }
    outputFile.close();
}



void msgBox(const QString &prompt)
{
    QMessageBox messageBox;
    messageBox.setText(prompt);
    messageBox.exec();
}
