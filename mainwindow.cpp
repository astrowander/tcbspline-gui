#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), nSplines(0), selectionMode(noSelection)
{
    ui->setupUi(this);
    undoShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Z), this, SLOT(undo()));
    redoShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Y), this, SLOT(redo()));
    openFileShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_O), this, SLOT(inputFromFile()));
    saveFileShortcut = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this, SLOT(outputToFile()));
    connect(ui->action_new, SIGNAL(triggered(bool)), this, SLOT(clearAllSplines()));
    connect(ui->action_save, SIGNAL(triggered(bool)), this, SLOT(outputToFile()));
    connect(ui->action_load, SIGNAL(triggered(bool)), this, SLOT(inputFromFile()));
    connect(ui->action_quit, SIGNAL(triggered(bool)), this, SLOT(close()));

    connect(ui->action_undo, SIGNAL(triggered(bool)), this, SLOT(undo()));
    connect(ui->action_redo, SIGNAL(triggered(bool)), this, SLOT(redo()));

    connect(ui->customPlot,SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(mouseMoveOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mousePress(QMouseEvent*)), this, SLOT(mouseClickedOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mouseRelease(QMouseEvent*)), this, SLOT(mouseReleasedOverPlot(QMouseEvent*)));
    connect(ui->customPlot,SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheeledOverPlot(QWheelEvent*)));
    connect(ui->customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(mouseDoubleClickedOverPlot(QMouseEvent*)));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::mouseMoveOverPlot(QMouseEvent *mouseEvent)
{
    //if (!splinesData.size()) return;
    if (selectionMode!=pointRedacting && splinesData.size()) splinesData[activeSpline].selectionGraph->clearData();
    double key, value, eps;
    eps = 5*(ui->customPlot->xAxis->range().upper - ui->customPlot->xAxis->range().lower) /
           ui->customPlot->width() ;
    pixelsToCoords(mouseEvent->x(), mouseEvent->y(), key, value);
    ui->statusBar->showMessage("x = " + QString::number(key) + ", y = " + QString::number(value));
    switch (selectionMode)
    {
    case pointDragging:
    {
        splinesData[activeSpline].selectionGraph->removeData(selectedKey);
        splinesData[activeSpline].copiedGraph->removeData(selectedKey);
        splinesData[activeSpline].selectionGraph->addData(selectedKey, key, value);
        splinesData[activeSpline].copiedGraph->addData(selectedKey, key, value);
        splinesData[activeSpline].copiedSpline->setPoint(int(selectedKey), key, value);
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
        if (!splinesData.size()) return;
        selectionMode = noSelection;        
        if (splinesData[activeSpline].currentGraph == nullptr) return;
        int nSelected = splinesData[activeSpline].spline->searchByKeyValue(key, value, eps);
        if (nSelected!=-1) {
            TSBPoint& point = splinesData[activeSpline].spline->getPointAt(nSelected);
            splinesData[activeSpline].selectionGraph->addData(point.t(), point.x(), point.y());
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
            if (!splinesData.size()) return;
            splinesData[activeSpline].spline->addAction(addPoint, splinesData[activeSpline].spline->size(), key, value);
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
            oldX = splinesData[activeSpline].currentGraph->data()->value(selectedKey).key;
            oldY = splinesData[activeSpline].currentGraph->data()->value(selectedKey).value;
            addNewGraph(splinesData[activeSpline].copiedGraph,  QCPCurve::LineStyle::lsNone, QColor(255,255,255, 128), 5.0);
            addNewGraph(splinesData[activeSpline].copiedInterpolatedGraph, QCPCurve::LineStyle::lsLine, QColor(0,0,0,128));
            splinesData[activeSpline].copiedGraph->addData(splinesData[activeSpline].spline->getTimes(), splinesData[activeSpline].spline->getKeys(), splinesData[activeSpline].spline->getValues());
            splinesData[activeSpline].copiedSpline = new TSBSpline(splinesData[activeSpline].copiedGraph);
            splinesData[activeSpline].copiedSpline->setPoints(splinesData[activeSpline].spline->getPoints());
            selectionMode = pointDragging;
        }
        else if (mouseEvent->button() == Qt::RightButton) {
            //splinesData[activeSpline].currentGraph->removeData(selectedKey);
            splinesData[activeSpline].spline->addAction(deletePoint, selectedKey, splinesData[activeSpline].currentGraph->data()->value(selectedKey).key, splinesData[activeSpline].currentGraph->data()->value(selectedKey).value, splinesData[activeSpline].spline->getPointAt(selectedKey).tension(), splinesData[activeSpline].spline->getPointAt(selectedKey).continuity(), splinesData[activeSpline].spline->getPointAt(selectedKey).bias());
            splinesData[activeSpline].selectionGraph->clearData();
            selectionMode = noSelection;
        }
        break;
    }
    case  pointRedacting:
    {
        pointRedactor->hide();
        delete pointRedactor;
        splinesData[activeSpline].spline->addAction(redactPoint, selectedKey, oldX, oldY, splinesData[activeSpline].spline->getPointAt(selectedKey).tension(),  splinesData[activeSpline].spline->getPointAt(selectedKey).continuity(), splinesData[activeSpline].spline->getPointAt(selectedKey).bias(), 0, 0,
                          splinesData[activeSpline].copiedSpline->getPointAt(selectedKey).tension() - splinesData[activeSpline].spline->getPointAt(selectedKey).tension(), splinesData[activeSpline].copiedSpline->getPointAt(selectedKey).continuity() - splinesData[activeSpline].spline->getPointAt(selectedKey).continuity(), splinesData[activeSpline].copiedSpline->getPointAt(selectedKey).bias() - splinesData[activeSpline].spline->getPointAt(selectedKey).bias());
        ui->customPlot->removePlottable(splinesData[activeSpline].copiedGraph);
        ui->customPlot->removePlottable(splinesData[activeSpline].copiedInterpolatedGraph);
        delete splinesData[activeSpline].copiedSpline;
        splinesData[activeSpline].copiedSpline = nullptr;
        selectionMode = noSelection;
        splinesData[activeSpline].selectionGraph->clearData();
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
        pointRedactor->setGeometry(200, 50, 160, 240);
        pointRedactor->show();

        addNewGraph(splinesData[activeSpline].copiedGraph,  QCPCurve::LineStyle::lsNone, QColor(255,255,255, 128), 5.0);
        addNewGraph(splinesData[activeSpline].copiedInterpolatedGraph, QCPCurve::LineStyle::lsLine, QColor(0,0,0,128));
        splinesData[activeSpline].copiedGraph->addData(splinesData[activeSpline].spline->getTimes(), splinesData[activeSpline].spline->getKeys(), splinesData[activeSpline].spline->getValues());
        splinesData[activeSpline].copiedSpline = new TSBSpline(splinesData[activeSpline].copiedGraph);
        splinesData[activeSpline].copiedSpline->setPoints(splinesData[activeSpline].spline->getPoints());

        connect(pointRedactor, SIGNAL(parametersChanged(double, double, double)), this, SLOT(parametersChanged(double, double, double)));
        pointRedactor->setInterface(splinesData[activeSpline].spline->getPointAt(selectedKey).tension(), splinesData[activeSpline].spline->getPointAt(selectedKey).continuity(), splinesData[activeSpline].spline->getPointAt(selectedKey).bias());
    }
}

void MainWindow::mouseReleasedOverPlot(QMouseEvent *mouseEvent)
{
    if (selectionMode == pointDragging) {
        splinesData[activeSpline].spline->addAction(movePoint, selectedKey, oldX, oldY, splinesData[activeSpline].spline->getPointAt(selectedKey).tension(), splinesData[activeSpline].spline->getPointAt(selectedKey).continuity(), splinesData[activeSpline].spline->getPointAt(selectedKey).bias(), splinesData[activeSpline].copiedGraph->data()->value(selectedKey).key - oldX, splinesData[activeSpline].copiedGraph->data()->value(selectedKey).value - oldY);
        ui->customPlot->removePlottable(splinesData[activeSpline].copiedGraph);
        ui->customPlot->removePlottable(splinesData[activeSpline].copiedInterpolatedGraph);
        delete splinesData[activeSpline].copiedSpline;
        splinesData[activeSpline].copiedSpline = nullptr;
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
    splinesData[activeSpline].spline->undo();
    replot();
}

void MainWindow::redo()
{
    splinesData[activeSpline].spline->redo();
    replot();
}

void MainWindow::pixelsToCoords(double x, double y, double &key, double &value)
{
    key = ui->customPlot->xAxis->pixelToCoord(x);
    value = ui->customPlot->yAxis->pixelToCoord(y);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    ui->customPlot->setGeometry(180, 10, event->size().width() - 200, event->size().height() -70);
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
    if (splinesData.size()) {
        splinesData[activeSpline].spline->makeInterpolatedGraph(splinesData[activeSpline].interpolatedGraph, ui->customPlot->xAxis, ui->customPlot->yAxis);
        if (splinesData[activeSpline].copiedSpline!=nullptr) splinesData[activeSpline].copiedSpline->makeInterpolatedGraph(splinesData[activeSpline].copiedInterpolatedGraph, ui->customPlot->xAxis, ui->customPlot->yAxis);
    }
    ui->customPlot->replot();
}

void MainWindow::parametersChanged(double tension, double continuity, double bias)
{
    splinesData[activeSpline].copiedSpline->setParametersAt(int(selectedKey), tension, continuity, bias);
    replot();
}

void MainWindow::addNewSpline()
{
    splinesData.append(SplineData());
    activeSpline = splinesData.size() - 1;
    addNewGraph(splinesData[activeSpline].selectionGraph, QCPCurve::LineStyle::lsNone, Qt::GlobalColor::green, 10.0);
    addNewGraph(splinesData[activeSpline].interpolatedGraph, QCPCurve::LineStyle::lsLine, Qt::GlobalColor::black);
    addNewGraph(splinesData[activeSpline].currentGraph,  QCPCurve::LineStyle::lsNone, Qt::GlobalColor::red, 5.0);
    splinesData[activeSpline].spline = new TSBSpline(splinesData[activeSpline].currentGraph);
    ui->spinBox->setMaximum(activeSpline);
    ui->spinBox->setValue(activeSpline);
}

void MainWindow::setActiveSpline(int i)
{
    QCPScatterStyle scatterStyle;
    scatterStyle.setPen(QColor(255, 0, 0, 255));
    scatterStyle.setSize(splinesData[activeSpline].currentGraph->scatterStyle().size());
    scatterStyle.setShape(splinesData[activeSpline].currentGraph->scatterStyle().shape());

    splinesData[i].currentGraph->setScatterStyle(scatterStyle);
    splinesData[i].interpolatedGraph->setPen(QColor(0, 0, 0, 255));

    scatterStyle.setPen(QColor(255, 0, 0, 64));
    for (int j=0; j < splinesData.size(); ++j) {
        if (j==i) continue;
        splinesData[j].currentGraph->setScatterStyle(scatterStyle);
        splinesData[j].interpolatedGraph->setPen(QColor(0, 0, 0, 64));
    }
    activeSpline = i;
    replot();
}

void MainWindow::clearAllSplines()
{
    for (int i=0; i<splinesData.size(); ++i)
    {
        splinesData[i].currentGraph->clearData();
        splinesData[i].interpolatedGraph->clearData();
    }
    splinesData.clear();
    ui->spinBox->setMaximum(0);
    ui->spinBox->setEnabled(false);
    ui->pushButton_2->setEnabled(false);
    replot();
}

void MainWindow::inputFromFile()
{
    clearAllSplines();
    QFile inputFile(QFileDialog::getOpenFileName(this, "Выберите файл для загрузки данных", "/home/astrowander/libs", "CSV files (*.csv)"));
    if (!haltIfError(inputFile.open(QIODevice::ReadOnly), "Невозможно открыть файл")) return;

    QString ss = inputFile.readLine();
    QStringList words;
    bool ok[6];
    nSplines = ss.toInt(&ok[0]);
    if (!haltIfError(ok[0] && (nSplines > 0),  "Данные в файле неверного формата")) return;

    for (int i=0; i < nSplines; ++i) {
        ss = inputFile.readLine();
        int nPoints = ss.toInt(&ok[0]);
        if (!haltIfError(ok[0],  "Данные в файле неверного формата")) return;
        addNewSpline();
        QVector<TSBPoint>& points = splinesData[i].spline->getPoints();
        for (int j=0; j<nPoints; ++j) {
            ss = inputFile.readLine();
            words = ss.split(';');
            if (!haltIfError(words.size() == 6, "Данные в файле неверного формата")) return;
            double t = words[0].toDouble(&ok[0]), key = words[1].toDouble(&ok[1]), value = words[2].toDouble(&ok[2]);
            double tension = words[3].toDouble(&ok[3]), continuity = words[4].toDouble(&ok[4]), bias = words[5].toDouble(&ok[5]);
            if (!haltIfError(ok[0] && ok[1] && ok[2] && ok[3] && ok[4] && ok[5],  "Данные в файле неверного формата")) return;
            points.append(TSBPoint(key, value, t, tension, continuity, bias));
            splinesData[i].currentGraph->addData(t, key, value);
        }
        splinesData[i].spline->makeInterpolatedGraph(splinesData[i].interpolatedGraph, ui->customPlot->xAxis, ui->customPlot->yAxis);
    }
    ui->customPlot->rescaleAxes();

    if (!ui->spinBox->isEnabled()) ui->spinBox->setEnabled(true);
    if (!ui->pushButton_2->isEnabled()) ui->pushButton_2->setEnabled(true);
    ui->spinBox->setMaximum(nSplines - 1);
    setActiveSpline(nSplines - 1);
    //replot();
}

void MainWindow::outputToFile()
{
    QFile outputFile(QFileDialog::getSaveFileName(this, "Выберите файл для сохранения данных", "/home/astrowander/libs", "CSV files (*.csv)"));
    QTextStream outputStream(&outputFile);
    if (!haltIfError(outputFile.open(QIODevice::WriteOnly), "Невозможно открыть файл")) return;
    outputStream << QString::number(splinesData.size()) << "\n";

    for (int i=0; i < splinesData.size(); ++i) {
         outputStream << QString::number(splinesData[i].spline->size()) << "\n";
         QVector<TSBPoint>& points = splinesData[i].spline->getPoints();
         for (int i=0; i<points.size(); ++i) {
             outputStream << points[i].print();
         }
    }
    outputFile.close();
}



void msgBox(const QString &prompt)
{
    QMessageBox messageBox;
    messageBox.setText(prompt);
    messageBox.exec();
}

void MainWindow::on_pushButton_clicked()
{
    if (!ui->spinBox->isEnabled()) ui->spinBox->setEnabled(true);
    if (!ui->pushButton_2->isEnabled()) ui->pushButton_2->setEnabled(true);
    addNewSpline();
    //ui->spinBox->setMaximum(splinesData.size() - 1);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    if (arg1>=0 && arg1 < splinesData.size()) setActiveSpline(arg1);
    this->setFocus();
}

void MainWindow::on_pushButton_2_clicked()
{
    ui->customPlot->removePlottable(splinesData[activeSpline].currentGraph);
    ui->customPlot->removePlottable(splinesData[activeSpline].interpolatedGraph);
    splinesData.removeAt(activeSpline);
    ui->spinBox->setValue(--activeSpline);
    ui->spinBox->setMaximum(splinesData.size() - 1);
    if (!(splinesData.size())) {
        ui->spinBox->setEnabled(false);
        ui->pushButton_2->setEnabled(false);
    }
    replot();
}

bool MainWindow::haltIfError(bool ok, const QString& message)
{
    if (!ok) {
        msgBox(message);
        return false;
    }
    return true;
}

void MainWindow::on_pushButton_3_clicked()
{
    clearAllSplines();
}
