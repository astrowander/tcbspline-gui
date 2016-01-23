#include "tsbspline.h"



void TSBSpline::doAction(const Action& action)
{
    switch (action.actionType)
    {
    case addPoint:
        keys.append(action.key);
        qSort(keys);
        values.insert(keys.indexOf(action.key), action.value);
        break;

    case deletePoint:
        values.removeAt(keys.indexOf(action.key));
        keys.removeOne(action.key);
        break;

    case movePoint: {
        int n = keys.indexOf(action.key);
        keys[n] += action.dx;
        values[n] += action.dy;
        break;
    }
    }
    currentGraph->setData(keys, values);
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


double TSBSpline::interpolate(double x)
{
    //if (x< keys.first() || x > keys.last()) return NAN;
    for (int i=0; i< keys.size() - 1; ++i) {
        if (x >= keys[i] && x <= keys[i+1]) {
            return values[i] + (values[i+1] - values[i]) * (x - keys[i]) / (keys[i+1] - keys[i]);
        }
    }
}

void TSBSpline::addAction(ActionType actionType, double key, double value, double dx, double dy)
{
    actions.append(Action(actionType, key, value, dx, dy));
    doAction(actions[++currentAction]);
}
