#ifndef TSBSPLINE_H
#define TSBSPLINE_H
#include "qcustomplot.h"
#include <QFile>
#include <QFileDialog>
#include <QList>
#include <QPointF>
#include <QString>
#include <QStringList>
#include <QTextStream>
#include <QVector>

enum ActionType {addPoint, deletePoint, movePoint, redactPoint};

struct Action
{
    ActionType actionType;
    int n;
    double key, value, dx, dy, tension, continuity, bias, dtension, dcontinuity, dbias;

    Action(ActionType m_actionType = addPoint, int m_n = 0, double m_key = 0, double m_value = 0, double m_tension = 0, double m_continuity = 0.0, double m_bias = 0.0, double m_dx = 0, double m_dy = 0, double m_dtension = 0, double m_dcontinuity = 0.0, double m_dbias = 0.0)
        : actionType(m_actionType), n(m_n), key(m_key), value(m_value), tension(m_tension), continuity(m_continuity), bias(m_bias), dx(m_dx), dy(m_dy), dtension(m_dtension), dcontinuity(m_dcontinuity), dbias(m_dbias) {}

    friend Action operator!(const Action& other)
    {
        Action temp;
        temp.n = other.n;
        temp.key = other.key;
        temp.value = other.value;
        temp.tension = other.tension;
        temp.continuity = other.continuity;
        temp.bias = other.bias;
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
        case redactPoint:
            temp.actionType = redactPoint;
            temp.tension += other.dtension;
            temp.dtension -= other.dtension;
            temp.continuity += other.dcontinuity;
            temp.dcontinuity -= other.dcontinuity;
            temp.bias += other.dbias;
            temp.dbias -= other.dbias;
            break;
        }
        return temp;
    }
};

class TSBPoint
{
private:
    double tpos, xpos, ypos, _tension, _continuity, _bias;
public:
    TSBPoint(double m_x = 0.0, double m_y = 0.0, double m_t = 0.0, double m_tension = 0.0, double m_continuity = 0.0, double m_bias = 0.0) : xpos(m_x), ypos(m_y), tpos(m_t), _tension(m_tension), _continuity(m_continuity), _bias(m_bias) {}
    static bool lessThan(const TSBPoint& p1, const TSBPoint& p2) {return p1.tpos < p2.tpos;}
    QString print() {return QString::number(tpos) + ";" + QString::number(xpos) + ";" + QString::number(ypos) + ";" + QString::number(_tension) + ";" + QString::number(_continuity) + ";" + QString::number(_bias) + "\n";}


    double tension() {return _tension;}
    double continuity() {return _continuity;}
    double bias() {return _bias;}
    double t() {return tpos;}
    double x() {return xpos;}
    double y() {return ypos;}

    void setTension(double m_tension) {_tension = m_tension;}
    void setContinuity(double m_continuity) {_continuity = m_continuity;}
    void setBias(double m_bias) {_bias = m_bias;}
    void setT(double m_t) {tpos = m_t;}
    void setX(double m_x) {xpos = m_x;}
    void setY(double m_y) {ypos = m_y;}

    friend TSBPoint operator+(const TSBPoint& p1, const TSBPoint& p2)
    {
        return TSBPoint(p1.xpos+p2.xpos, p1.ypos + p2.ypos);
    }

    friend TSBPoint operator-(const TSBPoint& p1, const TSBPoint& p2)
    {
        return TSBPoint(p1.xpos - p2.xpos, p1.ypos - p2.ypos);
    }

    friend TSBPoint operator*(double m, const TSBPoint& p1)
    {
        return TSBPoint(m*p1.xpos, m*p1.ypos);
    }

    friend TSBPoint operator/(const TSBPoint& p1, double m)
    {
        return TSBPoint(p1.xpos/m, p1.ypos/m);
    }
};

class TSBSpline
{
private:
    int currentAction;
    const double dt = 1;
    QCPCurve* currentGraph;
    QVector<Action> actions;
    QVector<TSBPoint> points;

    double H0(double s) {return 2*s*s*s - 3*s*s + 1;}
    double H1(double s) {return -2*s*s*s + 3*s*s;}
    double H2(double s) {return s*s*s - 2*s*s + s;}
    double H3(double s) {return s*s*s -s*s;}

public:
    TSBSpline(QCPCurve* m_currentGraph) : currentGraph(m_currentGraph), currentAction(-1) {}
    TSBPoint& getPointAt(int i) {return points[i];}
    TSBPoint interpolate(double t);
    QVector<double> getKeys();
    QVector<double> getTimes();
    QVector<double> getValues();
    QVector<TSBPoint>& getPoints() {return points;}
    void addAction(ActionType actionType, int n = 0, double key = 0, double value = 0, double tension = 0,  double continuity = 0,  double bias = 0, double dx = 0, double dy = 0, double dtension = 0, double dcontinuity = 0, double dbias = 0);
    void doAction(const Action& action);
    void clearActions() {actions.clear(); currentAction = -1;}

    void setPoints(const QVector<TSBPoint> &m_points) {points = m_points;}
    void setPoint(int n, double x, double y) {points[n].setX(x); points[n].setY(y);}
    void setParametersAt(int n, double tension, double continuity, double bias);

    void redo();
    void undo();

    void makeInterpolatedGraph(QCPCurve* curve, QCPAxis* xAxis, QCPAxis* yAxis);


    int size() {return points.size();}
    int searchByKeyValue(double key, double value, double eps);
};

#endif // TSBSPLINE_H
