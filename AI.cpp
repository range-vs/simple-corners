#include "AI.h"

std::vector<Coordinate2D> Ai::getAllMoves(const ArrayMesh& desk, const Coordinate2D& from)const
{
	std::vector<Coordinate2D> output;
	std::set<Coordinate2D> tmp;
	std::array< Coordinate2D, 4> urdl // up, right, down, left
	{ {
		{from.row-1, from.column},
		{from.row, from.column+1},
		{from.row + 1, from.column},
		{from.row , from.column-1}
	} };
	// проверка на допустимые границы поля
	for (auto&& m : urdl)
		if (m.row >= GAME_DESK::BLACK_CORNER && m.column >= GAME_DESK::BLACK_CORNER && m.row <= GAME_DESK::WHITE_CORNER && m.column <= GAME_DESK::WHITE_CORNER)
			tmp.insert(m);
	if (tmp.empty())
		return output;
	// проверка, что ход - не белая фигура
	std::set<Coordinate2D> errorMove;
	for (auto iterMove(tmp.begin()); iterMove != tmp.end(); ++iterMove) // перебираем все возможные ходы
	{
		for (auto iterAll(desk->begin() + GAME_DESK::DESK); iterAll != desk->end() - GAME_DESK::MAX_MOVE; ++iterAll) // проверка, что ходы не пешки
		{
			if (*iterMove == iterAll->getCoordinates())
			{
				errorMove.insert(*iterMove);
				break;
			}
		}
	}
	// берем разницу множеств и возвращаем её
	std::set_symmetric_difference(tmp.begin(), tmp.end(), errorMove.begin(), errorMove.end(), std::back_inserter(output));
	return output;
}

double Ai::сost(const Coordinate2D& board, const std::set<Coordinate2D>& player)const
{
	Coordinate2D aim(GAME_DESK::WHITE_CORNER, GAME_DESK::WHITE_CORNER);
	auto cost(0.0);
	for(auto&& currentPiece: player)
		cost += aim.distance(currentPiece);
	return cost;
}

bool Ai::makeMove(const ArrayMesh& desk, Coordinate2D* from, Coordinate2D* to)const
{
	auto cost(0.0);
	auto minCost(std::numeric_limits<double>::max());
	std::vector<Coordinate2D> minFrom; // лучшие откуда ходить
	std::vector<Coordinate2D> minTo; // лучшие куда ходить
	std::set<Coordinate2D> black; // все черные

	std::for_each(desk->begin(), desk->end(), [&black](auto& m) // получаем все черные
		{
			if (m.getName() == L"black_shape")
				black.insert(m.getCoordinates());
		}
	);
	for(auto&& piece: black) // проверяем каждую черную
	{
		for(auto&& to: getAllMoves(desk, piece)) // получаем все возможные ходы текущей черной фигуры
		{
			auto blackCopy(black); // получаем копию всех черных
			blackCopy.erase(blackCopy.find(piece)); // удаляем старую позицию фигуры
			blackCopy.insert(to); // добавляем новую
			cost = сost(to, blackCopy); // оцениваем этот ход
			if (cost < minCost) 
			{
				minCost = cost;
				minFrom.clear();
				minFrom.push_back(piece);
				minTo.clear();
				minTo.push_back(to);
			}
			else if (cost == minCost) 
			{
				minFrom.push_back(piece);
				minTo.push_back(to);
			}
		}
	}
	if (minFrom.empty()) // все ходы провальные
		return false;

	// Выбираем случайный ход из списка ходов с наилучшими показателяими (мин оценка)
	auto index(rand() % (minFrom.size() - 1 - 0 + 1) + 0);
	*from = minFrom[index];
	*to = minTo[index];
	return true;
}
