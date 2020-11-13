#include "Game.h"

Game::Game()
{
    computer = std::make_shared<Ai>();
    player = std::make_shared<Player>();
};

bool Game::makeMovePlayer(std::vector<int>& moves, int x, int y, Mesh* output)
{
    return player->makeMove(desk, moves, x, y, output);
}

bool Game::makeMoveAi(Coordinate2D* from, Coordinate2D* to)
{
    return computer->makeMove(desk, from, to);
}

bool Game::checkEndgame()
{
    std::set<Coordinate2D> user
    {
        {0,0}, {0, 1}, {0, 2},
        {1,0}, {1, 1}, {1, 2},
        {2,0}, {2, 1}, {2, 2}
    };
    std::set<Coordinate2D> ai
    {
        {5,5}, {5, 6}, {5, 7},
        {6,5}, {6, 6}, {6, 7},
        {7,5}, {7, 6}, {7, 7}
    };

    for (auto iter(desk->begin() + GAME_DESK::DESK); iter != desk->end() - GAME_DESK::MAX_MOVE; ++iter)
    {
        std::set<Coordinate2D>::iterator iterRemove;
        if (iter->getName() == L"black_shape")
        {
            iterRemove = ai.find(iter->getCoordinates());
            if(iterRemove != ai.end())
                ai.erase(iterRemove);
        }
        else
        {
            iterRemove = user.find(iter->getCoordinates());
            if (iterRemove != user.end())
                user.erase(iterRemove);
        }
    }
    if (user.empty() && ai.empty())
    {
        MessageBox(NULL, L"Draw! End game...", L"Message", MB_OK);
        return true;
    }
    else if (user.empty())
    {
        MessageBox(NULL, L"User win! End game...", L"Message", MB_OK);
        return true;
    }
    else if (ai.empty())
    {
        MessageBox(NULL, L"Computer win! End game...", L"Message", MB_OK);
        return true;
    }
    return false;
}

void Game::setNewArrayMesh(const ArrayMesh& am)
{
    desk = am;
}
