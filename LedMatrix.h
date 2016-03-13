#ifndef LEDMATRIX_H
#define LEDMATRIX_H

#include <vector>

class Point
{
public:
  Point(int x, int y) : x(x), y(y) {}

  int x, y;
};

class LedMatrix
{
public:
    virtual ~LedMatrix()  {}

    void addStrip(Point start, Point end, unsigned int length);
    void drawGrabRegion(int width, int height);


private:
    // Hold the Position of our Capture points
    std::vector <Point> pos;

    // Variables
    unsigned int size;
    Point _start;
    Point _end;

};


#endif // LEDMATRIX_H
