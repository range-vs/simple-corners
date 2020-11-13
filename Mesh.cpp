#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(const std::wstring& n, UINT sv, int r, int c): name(n), shiftVertex(sv), coords(r, c)
{
}

UINT Mesh::getShiftVertex() const noexcept
{
	return shiftVertex;
}

const std::wstring& Mesh::getName() const noexcept
{
	return name;
}

std::array<VertexSprite, 4>& Mesh::getVertex()
{
	return vertex;
}

bool Mesh::rayPick(int x, int y) const
{
	return (x >= vertex.at(0).pos.x && x <= vertex.at(2).pos.x) && (y >= vertex.at(0).pos.y && y <= vertex.at(1).pos.y); // так квадрат никогда не поворачивается
}

DirectX::XMFLOAT2 Mesh::getCenter() const noexcept
{
	return { (vertex.at(0).pos.x + vertex.at(2).pos.x) / 2,  (vertex.at(0).pos.y + vertex.at(1).pos.y) / 2 };
}

Coordinate2D& Mesh::getCoordinates() noexcept
{
	return coords;
}
