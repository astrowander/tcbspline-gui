#ifndef TSBSPLINE_H
#define TSBSPLINE_H
#include "qcustomplot.h"
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>

enum ActionType {addPoint, deletePoint, movePoint};



struct Action
{
    ActionType actionType;
    double key, value, dx, dy;

    Action(ActionType m_actionType = addPoint, double m_key = 0, double m_value = 0, double m_dx = 0, double m_dy = 0)
        : actionType(m_actionType), key(m_key), value(m_value), dx(m_dx), dy(m_dy) {}

    friend Action operator!(const Action& other)
    {
        Action temp;
        temp.key = other.key;
        temp.value = other.value;
        switch (other.actionType)
        {
        case addPoint:
            temp.actionType = deletePoint;
            break;
        case deletePoint:
            temp.actionType = addPoint;
            break;
        case movePoint:
            temp.actionType = movePoint;
            temp.key += other.dx;
            temp.value += other.dy;
            temp.dx = -other.dx;
            temp.dy = -other.dy;
            break;
        }
        return temp;
    }
};

class TSBSpline
{
private:
    int currentAction;
    QCPGraph* currentGraph;
    QVector<Action> actions;
    QVector<double> keys, values;
public:
    TSBSpline(QCPGraph* m_currentGraph) : currentGraph(m_currentGraph), currentAction(-1) {}
    double interpolate(double x);
    QVector<double> &getKeys() { return keys; }
    QVector<double> &getValues() { return values; }
    void addAction(ActionType actionType, double key, double value = 0, double dx = 0, double dy = 0);

    void doAction(const Action& action);
    void setKeys(const QVector<double> &m_keys) {keys = m_keys;}
    void setValues(const QVector<double> &m_values) {values = m_values;}

    void redo();
    void undo();

    void exportToGraph();
};

#endif // TSBSPLINE_H
