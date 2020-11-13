#pragma once

#include <string>
#include <array>
#include <memory>
#include <vector>

#include "d3d11_headers.h"
#include "GameHelper.h"

struct VertexSprite
{
	DirectX::XMFLOAT2 pos;
	DirectX::XMFLOAT2 uv;
};

class Mesh
{
	std::wstring name;
	UINT shiftVertex;
	std::array<VertexSprite, 4> vertex;
	Coordinate2D coords;

public:
	Mesh();
	Mesh(const std::wstring& n, UINT sv, int r, int c);
	UINT getShiftVertex()const noexcept;
	const std::wstring& getName()const noexcept;
	std::array<VertexSprite, 4>& getVertex();
	bool rayPick(int x, int y)const;
	DirectX::XMFLOAT2 getCenter()const noexcept;
	Coordinate2D& getCoordinates()noexcept;

};


using ArrayMesh = std::shared_ptr<std::vector<Mesh>>;