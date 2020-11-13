#include "Player.h"

bool Player::makeMove(const ArrayMesh& desk, std::vector<int>& moves, int x, int y, Mesh* output)const
{
	auto iterMove(moves.begin());
	for (auto iter(desk->begin() + GAME_DESK::ALL_DRAW_ITEM_NO_MOVE); iter != desk->end(); ++iter) // ����, � ����� ������ ������
	{
		if (*iterMove)
		{
			if (iter->rayPick(x, y)) // ��� ��������, �����
			{
				*output = *iter;
				return true;
			}
		}
		++iterMove;
	}
	return false;
}
