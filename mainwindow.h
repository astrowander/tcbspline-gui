#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "pointredactor.h"
#include "qcustomplot.h"
#include "tsbspline.h"
#include <memory>
#include <QMainWindow>
#include <QMessageBox>
#include <QMouseEvent>
//#include <QShortcut>

namespace Ui {
class MainWindow;
}

void msgBox(const QString& prompt);

enum SelectionMode {noSelection, pointSelected, pointDragging, pointRedacting, plotDragging};

struct SplineData
{
    QCPCurve* copiedGraph;
    QCPCurve* copiedInterpolatedGraph;
    QCPCurve* currentGraph;
    QCPCurve* interpolatedGraph;
    QCPCurve* selectionGraph;
    TSBSpline* spline;
    TSBSpline* copiedSpline;
    SplineData() : copiedGraph(nullptr), copiedInterpolatedGraph(nullptr), currentGraph(nullptr),
        interpolatedGraph(nullptr), selectionGraph(nullptr), spline(nullptr), copiedSpline(nullptr) {}
    ~SplineData()
    {
        //if (spline != nullptr) delete spline;
        //if (copiedSpline != nullptr) delete copiedSpline;
    }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    double oldX, oldY, oldTension;
    double selectedKey;
    int nSplines, activeSpline;
    PointRedactor* pointRedactor;

    QVector<SplineData> splinesData;
    QShortcut *undoShortcut, *redoShortcut, *saveFileShortcut, *openFileShortcut;
    SelectionMode selectionMode;    
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
    void addNewSpline();
    void setActiveSpline(int i);
    void clearAllSplines();

    void on_pushButton_clicked();

    void on_spinBox_valueChanged(int arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();



protected:
    bool haltIfError(bool ok, const QString &message);
    void addNewGraph(QCPCurve* &graphPtr, QCPCurve::LineStyle lineStyle, QColor color, double pointSize = 0.0);
    void pixelsToCoords(double x, double y, double& key, double& value);
    void resizeEvent(QResizeEvent* event);

};

#endif // MAINWINDOW_H
