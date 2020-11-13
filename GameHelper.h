#pragma once

struct Coordinate2D
{
	int row;
	int column;

	Coordinate2D();
	Coordinate2D(int r, int c);
	bool operator!=(int c)const noexcept;
	bool operator==(int c)const noexcept;
	bool operator!=(const Coordinate2D& c)const noexcept;
	bool operator==(const Coordinate2D& c)const noexcept;
	bool operator<(const Coordinate2D& _Right)const noexcept;
	double distance(const Coordinate2D& coord)const noexcept;
};

enum GAME_DESK
{
	NONE_CELL = -1,
	WHITE_CORNER = 7,
	BLACK_CORNER = 0,
	WHITE_SHAPES = 9,
	BLACK_SHAPES = 9,
	DESK = 1,
	MAX_MOVE = 4,
	ALL_DRAW_ALL = BLACK_SHAPES + WHITE_SHAPES + DESK + MAX_MOVE,
	ALL_DRAW_SHAPES = BLACK_SHAPES + WHITE_SHAPES,
	ALL_DRAW_ITEM_NO_MOVE = BLACK_SHAPES + WHITE_SHAPES + DESK,
	ALL_DRAW_ITEM_MOVE = 4,
	BORDER_DESK = 50,
	SIZE_DESK = 10,
	COUNT_VERTEX = 4
};

enum DESK_DIR
{
	MOVE_NONE = -1,
	MOVE_UP =  0,
	MOVE_RIGHT,
	MOVE_DOWN,
	MOVE_LEFT
};
