#pragma once

#include "Mesh.h"

class Player
{
public:
	bool makeMove(const ArrayMesh& desk, std::vector<int>& moves, int x, int y, Mesh* output)const;
};