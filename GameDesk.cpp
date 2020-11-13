#include "GameDesk.h"

auto GameDesk::getDesk() -> ArrayMesh&
{
	return desk;
}

GameDesk::GameDesk(const ArrayMesh& d) noexcept: desk(d)
{
}
