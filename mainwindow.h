#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

enum SelectionMode {noSelection, pointSelected, pointDragging, plotDragging};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    double oldX, oldY;
    double selectedKey;
    QCPData* selectedPoint;
    QCPGraph* copiedGraph;
    QCPGraph* currentGraph;
    QCPGraph* interpolatedGraph;
    QCPGraph* selectionGraph;
    SelectionMode selectionMode;
    TSBSpline* spline;
    Ui::MainWindow *ui;

private slots:
    void mouseClickedOverPlot(QMouseEvent* mouseEvent);
    void mouseMoveOverPlot(QMouseEvent* mouseEvent);
    void mouseReleasedOverPlot(QMouseEvent* mouseEvent);
    void mouseWheeledOverPlot(QWheelEvent* mouseEvent);
    void undo();
    void redo();

    void inputFromFile();
    void outputToFile();
    void replot();

protected:
    void addNewGraph(QCPGraph* &graphPtr, QCPGraph::LineStyle lineStyle, Qt::GlobalColor color, double pointSize = 0.0);
    void pixelsToCoords(double x, double y, double& key, double& value);
    void resizeEvent(QResizeEvent* event);

};

#endif // MAINWINDOW_H
