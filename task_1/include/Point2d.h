#pragma once
#include <cmath>

class Point2d
{
public:
    Point2d(double xx, double yy);
    virtual ~Point2d(void);
    double x() const { return m_dX; }
    double y() const { return m_dY; }
private:
    double m_dX;
    double m_dY;
};

double distance(const Point2d& p1, const Point2d& p2);