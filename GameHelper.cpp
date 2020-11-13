#include "GameHelper.h"

#include <cmath>

Coordinate2D::Coordinate2D():row(0), column(0)
{
}

Coordinate2D::Coordinate2D(int r, int c):row(r),column(c)
{

}

bool Coordinate2D::operator!=(int c)const noexcept
{
    return !(*this == c);
}

bool Coordinate2D::operator==(int c)const noexcept
{
    return row == c && column == c;
}

bool Coordinate2D::operator!=(const Coordinate2D& c) const noexcept
{
    return !(*this == c);
}

bool Coordinate2D::operator==(const Coordinate2D& c) const noexcept
{
    return row == c.row && column == c.column;
}

bool Coordinate2D::operator<(const Coordinate2D& _Right) const noexcept
{
    return row < _Right.row || (!(_Right.row < row) && column < _Right.column);
}

double Coordinate2D::distance(const Coordinate2D& coord) const noexcept
{
    return std::sqrt(std::pow(row - coord.row, 2) + std::pow(column - coord.column, 2));
}
