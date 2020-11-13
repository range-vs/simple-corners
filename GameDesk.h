#pragma once

#include "Mesh.h"

#include <memory>
#include <set>
#include <queue>

class GameDesk
{
	ArrayMesh desk; // доска со всеми фигурами -> смещение: [1 .. 18]
public:
	GameDesk(const ArrayMesh& d)noexcept;

	ArrayMesh& getDesk();

};