#include "LedMatrix.h"

void LedMatrix::addStrip(Point start, Point end, unsigned int length)
{
    // Setup Positioning
    _start = start;
    _end = end;

    float width = end.x - start.x;
    float height = end.y - start.y;

    float deltaX = width / length;
    float deltaY = height / length;

    for (unsigned int i = 0; i < length; i++)
    {
        pos.push_back(Point(start.x + i*deltaX,start.y + i*deltaY));
    }

}
