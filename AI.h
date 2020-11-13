#pragma once

#define NOMINMAX

#include <set>
#include <vector>

#include "Mesh.h"

class Ai
{

	std::vector<Coordinate2D> getAllMoves(const ArrayMesh& desk, const Coordinate2D& from)const; // получаем все возможные ходы
	double сost(const Coordinate2D& board, const std::set<Coordinate2D>& player)const; // оценка хода компа
public:
	bool makeMove(const ArrayMesh& desk, Coordinate2D* from, Coordinate2D* to)const;

};