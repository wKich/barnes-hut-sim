#include "quadtree.h"

QuadTree::~QuadTree()
{
    if (northWest != nullptr)
        delete northWest;
    if (northEast != nullptr)
        delete northEast;
    if (southWest != nullptr)
        delete southWest;
    if (southEast != nullptr)
        delete southEast;
}

QuadTree *QuadTree::createTree(QVector<Planet> &ps, const QRectF &rect)
{
    QuadTree* root = new QuadTree(rect);
    for (int i = 0; i < ps.size(); i++) {
        root->addPlanet(&ps[i]);
    }
    return root;
}

void QuadTree::getRoot(QuadTree*& root)
{
    while(root->parent != nullptr) {
        root = root->parent;
    }
}

QRectF QuadTree::getBoundary() const
{
    return boundary;
}

void QuadTree::drawBoundaries()
{
    glBegin(GL_LINE_LOOP);
    glVertex2f(boundary.topLeft().x(), boundary.topLeft().y());
    glVertex2f(boundary.bottomLeft().x(), boundary.bottomLeft().y());
    glVertex2f(boundary.bottomRight().x(), boundary.bottomRight().y());
    glVertex2f(boundary.topRight().x(), boundary.topRight().y());
    glEnd();
    if (northWest != nullptr)
        northWest->drawBoundaries();
    if (northEast != nullptr)
        northEast->drawBoundaries();
    if (southWest != nullptr)
        southWest->drawBoundaries();
    if (southEast != nullptr)
        southEast->drawBoundaries();
}

void QuadTree::calcForce(Planet *p, const int &deepLevel)
{
    if (isLeaf) {
        if (planet != p) {
            PointDouble2D distance = planet->currentPosition - p->currentPosition;
            p->currentForce += distance / distance.lenght() * planet->mass * p->mass / distance.squareLenght();
        }
    } else {
        if (((100.0 / pow(2, deepLevel)) / ((centerOfMass - p->currentPosition).lenght())) < 0.5) {
            PointDouble2D distance = centerOfMass - p->currentPosition;
            p->currentForce += distance / distance.lenght() * mass * p->mass / distance.squareLenght();
        } else {
            if (!northWest->isLeaf || northWest->planet != nullptr)
                northWest->calcForce(p, deepLevel + 1);
            if (!northEast->isLeaf || northEast->planet != nullptr)
                northEast->calcForce(p, deepLevel + 1);
            if (!southWest->isLeaf || southWest->planet != nullptr)
                southWest->calcForce(p, deepLevel + 1);
            if (!southEast->isLeaf || southEast->planet != nullptr)
                southEast->calcForce(p, deepLevel + 1);
        }
    }
}

void QuadTree::calcPosition(Planet *p, const float &deltaT)
{
    p->currentSpeed += p->currentForce / p->mass * deltaT;
    p->currentPosition += p->currentSpeed * deltaT;
    p->currentForce = PointDouble2D();
    if (p->leaf->boundary.contains(p->currentPosition.toPointF()) == false) {
        QuadTree* leaf = p->leaf;
        leaf->planet = nullptr;
        leaf->mass = 0;
        leaf->centerOfMass = leaf->boundary.center();
        leaf->parent->movePlanet(p);
    }
}

QuadTree::QuadTree(const QRectF& rect, QuadTree *p) :
    northWest(nullptr),
    northEast(nullptr),
    southWest(nullptr),
    southEast(nullptr),
    parent(p),
    isLeaf(true),
    boundary(rect),
    mass(0),
    centerOfMass(boundary.center()),
    planet(nullptr)
{
}

void QuadTree::addPlanet(Planet* p)
{
    //BUG if planet outside root boundary
    if (isLeaf) {
        if (!planet) {
            planet = p;
            planet->leaf = this;
            mass = planet->mass;
            centerOfMass = planet->currentPosition;
        } else {
            subdivide();
            putPlanetInChild(p);
            putPlanetInChild(planet);
            planet = nullptr;
            mass += p->mass;
            calcCenterOfMass();
        }
    } else {
        putPlanetInChild(p);
        mass += p->mass;
        calcCenterOfMass();
    }
}

void QuadTree::subdivide()
{
    northWest = new QuadTree(QRectF(boundary.topLeft(), boundary.center()), this);
    southEast = new QuadTree(QRectF(boundary.center(), boundary.bottomRight()), this);
    northEast = new QuadTree(QRectF(northWest->boundary.topRight(), southEast->boundary.topRight()), this);
    southWest = new QuadTree(QRectF(northWest->boundary.bottomLeft(), southEast->boundary.bottomLeft()), this);
    isLeaf = false;
}

void QuadTree::putPlanetInChild(Planet *p)
{
    if (northWest->boundary.contains(p->currentPosition.toPointF())) {
        northWest->addPlanet(p);
    } else if (northEast->boundary.contains(p->currentPosition.toPointF())) {
        northEast->addPlanet(p);
    } else if (southWest->boundary.contains(p->currentPosition.toPointF())) {
        southWest->addPlanet(p);
    } else {
        southEast->addPlanet(p);
    }
}

void QuadTree::calcCenterOfMass()
{
    centerOfMass = (northWest->centerOfMass * northWest->mass +
                    northEast->centerOfMass * northEast->mass +
                    southWest->centerOfMass * southWest->mass +
                    southEast->centerOfMass * southEast->mass) / mass;
}

void QuadTree::movePlanet(Planet *p)
{
    if (boundary.contains(p->currentPosition.toPointF())) {
        putPlanetInChild(p);
        calcCenterOfMass();
    } else {
        if (northWest->isLeaf && northEast->isLeaf &&
            southWest->isLeaf && southEast->isLeaf)
        {
            int planets = 0;
            QuadTree* child = nullptr;
            if (northWest->planet != nullptr) {
                planets++;
                child = northWest;
            }
            if (northEast->planet != nullptr) {
                planets++;
                child = northEast;
            }
            if (southWest->planet != nullptr) {
                planets++;
                child = southWest;
            }
            if (southEast->planet != nullptr) {
                planets++;
                child = southEast;
            }

            if (planets == 1) {
                planet = child->planet;
                planet->leaf = this;
                planets--;
            }
            if (planets == 0)
            {
                delete northWest;
                delete northEast;
                delete southWest;
                delete southEast;
                northWest = nullptr;
                northEast = nullptr;
                southWest = nullptr;
                southEast = nullptr;
                isLeaf = true;
                if (planet != nullptr) {
                    mass = planet->mass;
                    centerOfMass = planet->currentPosition;
                } else {
                    mass = 0;
                    centerOfMass = boundary.center();
                }
            } else {
                mass -= p->mass;
                calcCenterOfMass();
            }
        } else {
            mass -= p->mass;
            calcCenterOfMass();
        }

        if (parent != nullptr) {
            parent->movePlanet(p);
        } else {
            if (boundary.right() < p->currentPosition.x() || boundary.bottom() < p->currentPosition.y()) {
                parent = new QuadTree(QRectF(boundary.topLeft(), boundary.size() * 2));
                parent->northWest = this;
                parent->northEast = new QuadTree(QRectF(boundary.topRight(), boundary.size()), parent);
                parent->southWest = new QuadTree(QRectF(boundary.bottomLeft(), boundary.size()), parent);
                parent->southEast = new QuadTree(QRectF(boundary.bottomRight(), boundary.size()), parent);
            } else {
                parent = new QuadTree(QRectF(QPointF(boundary.left() - boundary.width(), boundary.top() - boundary.height()), boundary.size() * 2));
                parent->northWest = new QuadTree(QRectF(parent->boundary.topLeft(), boundary.size()), parent);
                parent->northEast = new QuadTree(QRectF(parent->northWest->boundary.topRight(), boundary.size()), parent);
                parent->southWest = new QuadTree(QRectF(parent->northWest->boundary.bottomLeft(), boundary.size()), parent);
                parent->southEast = this;
            }
            parent->mass = mass + p->mass;
            parent->isLeaf = false;
            parent->movePlanet(p);
        }
    }
}
