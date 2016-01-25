#include "tsbspline.h"



void TSBSpline::doAction(const Action& action)
{
    switch (action.actionType)
    {
    case addPoint:
        //points.append(TSBPoint(action.key, action.value, action.t));
        points.insert(action.n, TSBPoint(action.key, action.value, action.n));
        for (int j = action.n + 1; j < points.size(); ++j) {
            points[j].setT(points[j].t() + 1.0);
        }
        //qSort(points.begin(), points.end(), TSBPoint::lessThan);
        break;

    case deletePoint:
    {
        points.removeAt(action.n);
        for (int j = action.n; j<points.size(); ++j)
        {
            points[j].setT(points[j].t() - 1.0);
        }
        break;
    }
    case movePoint: {
        points[action.n].setX(points[action.n].x() +  action.dx);
        points[action.n].setY(points[action.n].y() +  action.dy);
        //qSort(points.begin(), points.end(), TSBPoint::lessThan);
        break;
    }
    case redactPoint: {
        points[action.n].setTension(points[action.n].tension() + action.dtension);
        points[action.n].setContinuity(points[action.n].continuity() + action.dcontinuity);
        points[action.n].setBias(points[action.n].bias() + action.dbias);
        break;
    }
    }
    currentGraph->setData(getTimes(), getKeys(), getValues());
}

void TSBSpline::setParametersAt(int n, double tension, double continuity, double bias)
{
    points[n].setTension(tension);
    points[n].setContinuity(continuity);
    points[n].setBias(bias);
}

void TSBSpline::redo()
{
    if (currentAction >= actions.size()-1) return;
    doAction(actions[++currentAction]);
}

void TSBSpline::undo()
{
    if (currentAction < 0) return;
    doAction(!actions[currentAction--]);
}

void TSBSpline::makeInterpolatedGraph(QCPCurve *curve, QCPAxis *xAxis, QCPAxis *yAxis)
{
    curve->clearData();
    if (points.size() >= 2) {
        TSBPoint point;
        double dt = 0.05;
        for (double t = 0.0; t <= double(points.size() - 1) + dt / 2; t+=dt) {
               point = interpolate(t);
               curve->addData(t, point.x(), point.y());
        }
    }
}

int TSBSpline::searchByKeyValue(double key, double value, double eps)
{
    for (int i=0; i<points.size(); ++i)
    {
        if (fabs(points[i].x()-key) < eps && fabs(points[i].y() - value) < eps) return i;
    }
    return -1;
}


TSBPoint TSBSpline::interpolate(double t)
{
    /*for (int i=0; i< points.size() - 1; ++i) {
        if (t >= points[i].x() && t <= points[i+1].x()) {
            return points[i].y() + (points[i+1].y() - points[i].y()) * (t - points[i].x()) / (points[i+1].x() - points[i].x());
        }
    }*/
    int i = floor(t / dt), last = points.size()-1;
    if (i == last) --i;
    TSBPoint T1I, T1O, T2I, T2O;
    if (i==0) {
        T1I = (1 - points[i].tension()) * (1 + points[i].continuity()) * (1 - points[i].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i].tension()) * (1 - points[i].continuity()) * (1 + points[i].bias()) * (points[i+1] - points[i]) / 2 ;
        T1O = (1 - points[i].tension()) * (1 - points[i].continuity()) * (1 - points[i].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i].tension()) * (1 + points[i].continuity()) * (1 + points[i].bias()) * (points[i+1] - points[i]) / 2 ;

    }
    else {
        T1I = (1 - points[i].tension()) * (1 + points[i].continuity()) * (1 - points[i].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i].tension()) * (1 - points[i].continuity()) * (1 + points[i].bias()) * (points[i] - points[i-1]) / 2 ;
        T1O = (1 - points[i].tension()) * (1 - points[i].continuity()) * (1 - points[i].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i].tension()) * (1 + points[i].continuity()) * (1 + points[i].bias()) * (points[i] - points[i-1]) / 2 ;
    }

    if (i==last - 1) {
        T2I = (1 - points[i+1].tension()) * (1 + points[i+1].continuity()) * (1 - points[i+1].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i+1].tension()) * (1 - points[i+1].continuity()) * (1 + points[i+1].bias()) * (points[i+1] - points[i]) / 2 ;
        T2O = (1 - points[i+1].tension()) * (1 - points[i+1].continuity()) * (1 - points[i+1].bias()) * (points[i+1] - points[i]) / 2 +
              (1 - points[i+1].tension()) * (1 + points[i+1].continuity()) * (1 + points[i+1].bias()) * (points[i+1] - points[i]) / 2 ;
    }
    else {
        T2I = (1 - points[i+1].tension()) * (1 + points[i+1].continuity()) * (1 - points[i+1].bias()) * (points[i+2] - points[i+1]) / 2 +
              (1 - points[i+1].tension()) * (1 - points[i+1].continuity()) * (1 + points[i+1].bias()) * (points[i+1] - points[i]) / 2 ;
        T2O = (1 - points[i+1].tension()) * (1 - points[i+1].continuity()) * (1 - points[i+1].bias()) * (points[i+2] - points[i+1]) / 2 +
              (1 - points[i+1].tension()) * (1 + points[i+1].continuity()) * (1 + points[i+1].bias()) * (points[i+1] - points[i]) / 2 ;
    }

    double s = (t - i*dt) / dt;
    return H0(s) * points[i] + H1(s) * points[i+1] + H2(s) * dt * T1O + H3(s) * dt * T2I;
}

QVector<double> TSBSpline::getKeys()
{
    QVector<double> temp;
    foreach (TSBPoint p, points) temp.append(p.x());
    return temp;
}

QVector<double> TSBSpline::getTimes()
{
    QVector<double> temp;
    foreach (TSBPoint p, points) temp.append(p.t());
    return temp;
}

QVector<double> TSBSpline::getValues()
{
    QVector<double> temp;
    foreach (TSBPoint p, points) temp.append(p.y());
    return temp;
}

void TSBSpline::addAction(ActionType actionType, int n, double key, double value, double tension, double continuity, double bias, double dx, double dy, double dtension, double dcontinuity, double dbias)
{
    actions.resize(currentAction+1);
    actions.append(Action(actionType, n, key, value, tension, continuity, bias, dx, dy, dtension, dcontinuity, dbias));
    doAction(actions[++currentAction]);
}
