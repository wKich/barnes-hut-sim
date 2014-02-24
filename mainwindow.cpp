#include "mainwindow.h"

MainWindow::MainWindow() :
    m_update_pending(false),
    m_context(nullptr),
    m_program(nullptr),
    m_glBuffer(nullptr)
{
    setSurfaceType(QWindow::OpenGLSurface);

    planets.append(Planet(PointDouble2D(100, 100), PointDouble2D(0, 0), 100000));
    planets.append(Planet(PointDouble2D(150, 100), PointDouble2D(0, 50), 100));
    planets.append(Planet(PointDouble2D(50, 100), PointDouble2D(0, -50), 100));
    planets.append(Planet(PointDouble2D(100, 50), PointDouble2D(30, 0), 200));
    planets.append(Planet(PointDouble2D(100, 150), PointDouble2D(-30, 0), 200));

    root = QuadTree::createTree(planets, QRectF(0, 0, 200, 200));

    calc = new CalcThread(root);
    QThread* thread = new QThread();
    calc->moveToThread(thread);
    calc->setPlanets(planets);
    connect(this, SIGNAL(needCalc()), calc, SLOT(start()));
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    connect(thread, SIGNAL(finished()), calc, SLOT(deleteLater()));
    thread->start();
}

MainWindow::~MainWindow()
{
    connect(calc, SIGNAL(stopped()), calc->thread(), SLOT(quit()));
    calc->stop();
    delete root;
}

void MainWindow::render()
{
    QRectF boundary = root->getBoundary();

    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, width(), height());

    QMatrix4x4 matrix;
    matrix.ortho(boundary.left(), boundary.right(), boundary.bottom(), boundary.top(), 0.0f, 1.0f);

    m_program->bind();
    m_program->setUniformValue(m_matrixUni, matrix);

    m_glBuffer->bind();
    for (int i = 0; i < planets.size(); i++) {
        QVector2D vec = planets.at(i).currentPosition.toVector2D();
        m_glBuffer->write((counter + i * 60 * 3600) * sizeof(QVector2D), &vec, sizeof(QVector2D));
    }
    m_program->setAttributeBuffer(m_vertexAttr, GL_FLOAT, 0, 2);
    m_program->enableAttributeArray(m_vertexAttr);

    m_program->setUniformValue(m_colorUni, QColor(Qt::white));
    root->drawBoundaries();

    for (int i = 0; i < planets.size(); i++) {
        glDrawArrays(GL_LINE_STRIP, i * 60 * 3600, counter);
    }

    m_glBuffer->release();
    m_program->release();
}

bool MainWindow::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::UpdateRequest:
        renderNow();
        return true;
    default:
        return QWindow::event(event);
    }
}

void MainWindow::exposeEvent(QExposeEvent *event)
{
    Q_UNUSED(event);

    if (isExposed())
        renderNow();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Escape:
        this->close();
        break;
    default:
        break;
    }
}

void MainWindow::renderLater()
{
    if (!m_update_pending) {
        m_update_pending = true;
        QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
    }
}

void MainWindow::renderNow()
{
    if (!isExposed())
        return;

    calc->stop();

    m_update_pending = false;

    if (!m_context) {
        initialize();
    } else {
        m_context->makeCurrent(this);
    }

    render();
    m_context->swapBuffers(this);
    renderLater();

    counter++;
    if (counter >= 60 * 3600)
        counter = 0;

    emit needCalc();
}

void MainWindow::initialize()
{
    counter = 0;

    m_context = new QOpenGLContext(this);
    m_context->setFormat(requestedFormat());
    m_context->create();
    m_context->makeCurrent(this);

    m_program = new QOpenGLShaderProgram(this);
    m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/GLSL/main.vert");
    m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/GLSL/main.frag");
    m_program->link();
    m_vertexAttr = m_program->attributeLocation("qt_Vertex");
    m_colorUni = m_program->uniformLocation("qt_Color");
    m_matrixUni = m_program->uniformLocation("qt_Matrix");

    m_glBuffer = new QOpenGLBuffer;
    m_glBuffer->create();
    m_glBuffer->setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_glBuffer->bind();
    m_glBuffer->allocate(planets.size() * 60 * 3600 * sizeof(QVector2D));
    m_glBuffer->release();
}
