#pragma once

#include "AI.h"
#include "Player.h"

class Game
{
	std::shared_ptr<Ai> computer;
	std::shared_ptr<Player> player;
	ArrayMesh desk; // массив мешей из сцены

public:
	Game(); 
	bool makeMovePlayer(std::vector<int>& moves, int x, int y, Mesh* output);
	bool makeMoveAi(Coordinate2D* from, Coordinate2D* to);
	bool checkEndgame();
	void setNewArrayMesh(const ArrayMesh& am);
};