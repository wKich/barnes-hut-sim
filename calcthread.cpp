#include "calcthread.h"

CalcThread::CalcThread(QuadTree *&r, QObject *parent) :
    root(r),
    QObject(parent)
{
    running = false;
}

void CalcThread::setPlanets(QVector<Planet>& p)
{
    for (int i = 0; i < p.size(); i++)
        planets.append(&p[i]);
}

void CalcThread::stop()
{
    running = false;
}

void CalcThread::start()
{
    running = true;
    while(running && deltaT < 1.0) {
        for (int i = 0; i < planets.size(); i++)
            root->calcForce(planets[i]);
        for (int i = 0; i < planets.size(); i++)
            root->calcPosition(planets[i], 0.0001);
        QuadTree::getRoot(root);
        deltaT += 0.006;
    }
    if (running) {
        deltaT -= 1.0;
        running = false;
    } else {
        deltaT = 0.0;
    }

    emit stopped();
}
