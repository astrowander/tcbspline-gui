#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include "qcustomplot.h"
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

enum SelectionMode {noSelection, pointSelected, pointDragging};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QCPGraph* currentGraph;
    QCPGraph* selectionGraph;
    //QCPData* selectedPoint;
    double selectedKey;
    SelectionMode selectionMode;

private slots:
    void mouseMoveOverPlot(QMouseEvent* mouseEvent);
    void mouseClickedOverPlot(QMouseEvent* mouseEvent);
    void mouseReleasedOverPlot(QMouseEvent* mouseEvent);

protected:
    void pixelsToCoords(double x, double y, double& key, double& value);
    void addNewGraph(QCPGraph* &graphPtr, Qt::GlobalColor color, double pointSize);
};

#endif // MAINWINDOW_H
