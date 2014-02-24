#ifndef CALCTHREAD_H
#define CALCTHREAD_H

#include <QObject>

#include <QDebug>

#include "quadtree.h"

class CalcThread : public QObject
{
    Q_OBJECT
public:
    explicit CalcThread(QuadTree*& r, QObject *parent = 0);
    void setPlanets(QVector<Planet> &p);
    void stop();

signals:
    void stopped();

public slots:
    void start();

private:
    QuadTree*& root;
    QVector<Planet*> planets;
    float deltaT;
    bool running;
};

#endif // CALCTHREAD_H
