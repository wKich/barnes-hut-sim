#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWindow>
#include <QCoreApplication>
#include <QOpenGLContext>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QVector2D>

#include <QThread>

#include <QDebug>

#include "calcthread.h"

class MainWindow : public QWindow
{
    Q_OBJECT
public:
    explicit MainWindow();
    ~MainWindow();

    void render();

protected:
    bool event(QEvent *event);
    void exposeEvent(QExposeEvent *event);
    void keyPressEvent(QKeyEvent *event);

signals:
    void needCalc();

public slots:
    void renderLater();
    void renderNow();

private:
    //Rendering
    bool m_update_pending;

    QOpenGLContext* m_context;
    QOpenGLShaderProgram* m_program;
    QOpenGLBuffer* m_glBuffer;

    int m_vertexAttr;
    int m_colorUni;
    int m_matrixUni;

    void initialize();

    CalcThread* calc;
    QuadTree* root;
    QVector<Planet> planets;
    int counter;
};

#endif // MAINWINDOW_H
