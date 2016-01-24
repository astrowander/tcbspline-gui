#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pointredactor.h"
#include "qcustomplot.h"
#include "tsbspline.h"
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
//#include <QShortcut>

namespace Ui {
class MainWindow;
}

void msgBox(const QString& prompt);

enum SelectionMode {noSelection, pointSelected, pointDragging, pointRedacting, plotDragging};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    double oldX, oldY, oldTension;
    double selectedKey;
    PointRedactor* pointRedactor;
    QCPData* selectedPoint;
    QCPCurve* copiedGraph;
    QCPCurve* copiedInterpolatedGraph;
    QCPCurve* currentGraph;
    QCPCurve* interpolatedGraph;
    QCPCurve* selectionGraph;
    SelectionMode selectionMode;
    TSBSpline *spline, *copiedSpline;
    Ui::MainWindow *ui;

private slots:
    void mouseClickedOverPlot(QMouseEvent* mouseEvent);
    void mouseDoubleClickedOverPlot(QMouseEvent* mouseEvent);
    void mouseMoveOverPlot(QMouseEvent* mouseEvent);
    void mouseReleasedOverPlot(QMouseEvent* mouseEvent);
    void mouseWheeledOverPlot(QWheelEvent* mouseEvent);
    void undo();
    void redo();

    void inputFromFile();
    void outputToFile();
    void replot();

    void parametersChanged(double tension, double continuity, double bias);

protected:
    void addNewGraph(QCPCurve* &graphPtr, QCPCurve::LineStyle lineStyle, QColor color, double pointSize = 0.0);
    void pixelsToCoords(double x, double y, double& key, double& value);
    void resizeEvent(QResizeEvent* event);

};

#endif // MAINWINDOW_H
