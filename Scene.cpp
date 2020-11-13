#include "Scene.h"

#include <array>

int SceneParameters::getClientWidth()const noexcept
{
	return w;
}

int SceneParameters::getClientHeight()const noexcept
{
	return h;
}

void SceneParameters::setClientWidth(int w)noexcept
{
	this->w = w;
}

void SceneParameters::setClientHeight(int h)noexcept
{
	this->h = h;
}

void Direct3D11Scene::transformCoordinateToViewport(int w, int h, int* x, int* y) const noexcept
{
	*x -= (w / 2);
	*y = -(*y - (h / 2));
}

void Direct3D11Scene::recreateDynamicVertexBuffer()
{
	// копируем все в буфер вершин
	D3D11_MAPPED_SUBRESOURCE mappedResourceVertexBuffer;
	// мапим буфер вершин
	auto hr(d3dImmediateContext->Map(vertexBufferScene.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResourceVertexBuffer));
	if (FAILED(hr))
		throw ExceptionMemory(L"Error mapped vertex buffer!\n");
	auto mappedMem(static_cast<VertexSprite*>(mappedResourceVertexBuffer.pData));
	if (!mappedMem)
		throw ExceptionMemory(L"Error get vertex array!\n");
	auto indexMappedBuffer(0);
	std::for_each(meshBuffer->begin(), meshBuffer->end(), [&mappedMem, &indexMappedBuffer](auto& m)
		{
			std::for_each(m.getVertex().cbegin(), m.getVertex().cend(), [&mappedMem, &indexMappedBuffer](const auto& v)
				{
					mappedMem[indexMappedBuffer++] = v;
				}
			);
		}
	);
	d3dImmediateContext->Unmap(vertexBufferScene.Get(), 0);
}

void Direct3D11Scene::recalculateSceneCoordinates(int w, int h)
{
	// запас 50 пикселей (рамка)
	auto _min(std::min(w, h));
	auto _max(std::max(w, h));

	// real coords
	float first((_max - _min) / 2.f);
	float second(_max - first);
	float third(0);
	float fours(static_cast<float>(_min));

	// fix for 50 pixels
	first += static_cast<float>(GAME_DESK::BORDER_DESK); // max
	second -= static_cast<float>(GAME_DESK::BORDER_DESK);
	third += static_cast<float>(GAME_DESK::BORDER_DESK); // min
	fours -= static_cast<float>(GAME_DESK::BORDER_DESK);

	// correct game desk
	auto& desk(meshBuffer->at(0));
	desk.getVertex().at(0) = { DirectX::XMFLOAT2(first - (_max / 2), third - (_min / 2)), DirectX::XMFLOAT2(0.f, 1.f) };
	desk.getVertex().at(1) = { DirectX::XMFLOAT2(first - (_max / 2), fours - (_min / 2)), DirectX::XMFLOAT2(0.f, 0.f) };
	desk.getVertex().at(2) = { DirectX::XMFLOAT2(second - (_max / 2), third - (_min / 2)), DirectX::XMFLOAT2(1.f, 1.f) };
	desk.getVertex().at(3) = { DirectX::XMFLOAT2(second - (_max / 2), fours - (_min / 2)), DirectX::XMFLOAT2(1.f, 0.f) };

	// shapes
	float width_desk(_min - static_cast<float>(GAME_DESK::BORDER_DESK) * 2); // 50 * 2 -> borders
	float size_shape(width_desk / static_cast<float>(GAME_DESK::SIZE_DESK)); // 8 -> count row or columns
	for (auto i(1); i <= GAME_DESK::ALL_DRAW_SHAPES; ++i)
	{
		auto coord(meshBuffer->at(i).getCoordinates());
		float posX(first - (_max / 2) + (size_shape * coord.column) + size_shape);
		float posY(fours - (_min / 2) - (size_shape * coord.row) - size_shape);

		meshBuffer->at(i).getVertex().at(0) = { DirectX::XMFLOAT2(posX, posY - size_shape), DirectX::XMFLOAT2(0.f, 1.f) };
		meshBuffer->at(i).getVertex().at(1) = { DirectX::XMFLOAT2(posX, posY), DirectX::XMFLOAT2(0.f, 0.f) };
		meshBuffer->at(i).getVertex().at(2) = { DirectX::XMFLOAT2(posX + size_shape, posY - size_shape), DirectX::XMFLOAT2(1.f, 1.f) };
		meshBuffer->at(i).getVertex().at(3) = { DirectX::XMFLOAT2(posX + size_shape, posY), DirectX::XMFLOAT2(1.f, 0.f) };
	}

	recalculateMoveCoordinates(size_shape);
	recreateDynamicVertexBuffer();
}

void Direct3D11Scene::calculateMovePlayer(float size_shape)
{
	recalculateMoveCoordinates(size_shape);
	// расчёт ходов
	for (auto iter(meshBuffer->begin() + GAME_DESK::ALL_DRAW_ITEM_NO_MOVE); iter != meshBuffer->end(); ++iter) // проверка, что в пределах доски
	{
		if (iter->getCoordinates() == GAME_DESK::NONE_CELL)
			moves.emplace_back(0); // no in desk
		else
			moves.emplace_back(4);
	}
	auto iterMove(moves.begin());
	for (auto iter(meshBuffer->begin() + GAME_DESK::ALL_DRAW_ITEM_NO_MOVE); iter != meshBuffer->end(); ++iter) // проверка что не пешки 
	{
		for (auto iterShapes(meshBuffer->begin() + GAME_DESK::DESK); iterShapes != meshBuffer->end() - GAME_DESK::ALL_DRAW_ITEM_MOVE; ++iterShapes)
		{
			if (/*iter->getCoordinates() != GAME_DESK::NONE_CELL && */iter->getCoordinates() == iterShapes->getCoordinates())
			{
				*iterMove = 0; // is shape!
				break;
			}
		}
		++iterMove;
	}
}

void Direct3D11Scene::calculateMoveAi(const Coordinate2D& from, const Coordinate2D& to, float size_shape)
{
	for (auto iter(meshBuffer->begin() + GAME_DESK::DESK); iter != meshBuffer->end() - GAME_DESK::MAX_MOVE; ++iter) // ищем нужные meshes
	{
		if (iter->getCoordinates() == from)
		{
			auto dir(checkPositionMove(from, to));
			*iter = createMoveMesh(dir, *iter, to, size_shape);
			return;
		}
	}
}

DESK_DIR Direct3D11Scene::checkPositionMove(const Coordinate2D& from, const Coordinate2D& to)
{
	if (to.row < from.row)
		return DESK_DIR::MOVE_UP;
	else if (to.column > from.column)
		return DESK_DIR::MOVE_RIGHT;
	else if (to.row > from.row)
		return DESK_DIR::MOVE_DOWN;
	else if (to.column < from.column)
		return DESK_DIR::MOVE_LEFT;
	return DESK_DIR::MOVE_NONE;
}

Mesh Direct3D11Scene::createMoveMesh(const DESK_DIR& dir, Mesh m, Coordinate2D coord, float size_shape)
{
	Mesh m_out(m.getName(), m.getShiftVertex(), coord.row, coord.column);
	if (dir == DESK_DIR::MOVE_NONE)
		return m; // без изменений
	else if (dir == DESK_DIR::MOVE_UP)
	{
		// up
		m_out.getVertex().at(0) = { m.getVertex().at(1) };
		m_out.getVertex().at(1) = { m.getVertex().at(1) }; m_out.getVertex().at(1).pos.y += size_shape;
		m_out.getVertex().at(2) = { m.getVertex().at(3) };
		m_out.getVertex().at(3) = { m.getVertex().at(3) }; m_out.getVertex().at(3).pos.y += size_shape;
	}
	else if (dir == DESK_DIR::MOVE_RIGHT)
	{
		// right
		m_out.getVertex().at(0) = { m.getVertex().at(2) };
		m_out.getVertex().at(1) = { m.getVertex().at(1) }; m_out.getVertex().at(1).pos.x += size_shape;
		m_out.getVertex().at(2) = { m.getVertex().at(2) }; m_out.getVertex().at(2).pos.x += size_shape;
		m_out.getVertex().at(3) = { m.getVertex().at(1) }; m_out.getVertex().at(3).pos.x += size_shape * 2;
	}
	else if (dir == DESK_DIR::MOVE_DOWN)
	{
		// bottom
		m_out.getVertex().at(0) = { m.getVertex().at(0) }; m_out.getVertex().at(0).pos.y -= size_shape;
		m_out.getVertex().at(1) = { m.getVertex().at(0) };
		m_out.getVertex().at(2) = { m.getVertex().at(2) }; m_out.getVertex().at(2).pos.y -= size_shape;
		m_out.getVertex().at(3) = { m.getVertex().at(2) };
	}
	else if (dir == DESK_DIR::MOVE_LEFT)
	{
		// left
		m_out.getVertex().at(0) = { m.getVertex().at(0) }; m_out.getVertex().at(0).pos.x -= size_shape;
		m_out.getVertex().at(1) = { m.getVertex().at(1) }; m_out.getVertex().at(1).pos.x -= size_shape;
		m_out.getVertex().at(2) = { m.getVertex().at(0) };
		m_out.getVertex().at(3) = { m.getVertex().at(1) };
	}
	m_out.getVertex().at(0).uv = { 0.f, 1.f };
	m_out.getVertex().at(1).uv = { 0.f, 0.f };
	m_out.getVertex().at(2).uv = { 1.f, 1.f };
	m_out.getVertex().at(3).uv = { 1.f, 0.f };
	return m_out;
}

void Direct3D11Scene::newGame()
{
	// create mesh buffer
	meshBuffer = std::make_shared<std::vector<Mesh>>();
	if (!meshBuffer)
		throw ExceptionMemory(L"ArrayMesh is not create, memory not found!\n");
	meshBuffer->emplace_back(L"desk", 0, GAME_DESK::NONE_CELL, GAME_DESK::NONE_CELL);
	auto shift(4);
	for (auto i(0), r(0), c(0); i < GAME_DESK::BLACK_SHAPES; ++i, shift += GAME_DESK::COUNT_VERTEX)
	{
		meshBuffer->emplace_back(L"black_shape", shift, r, c++);
		if (c % 3 == 0)
		{
			++r;
			c = 0;
		}
	}
	for (auto i(0), r(GAME_DESK::WHITE_SHAPES - 4), c(GAME_DESK::WHITE_SHAPES - 4); i < GAME_DESK::WHITE_SHAPES; ++i, shift += GAME_DESK::COUNT_VERTEX)
	{
		meshBuffer->emplace_back(L"white_shape", shift, r, c++);
		if ((c + 1) == GAME_DESK::WHITE_SHAPES)
		{
			++r;
			c = GAME_DESK::WHITE_SHAPES - 4;
		}
	}
	for (auto i(0); i < GAME_DESK::MAX_MOVE; ++i, shift += GAME_DESK::COUNT_VERTEX)
		meshBuffer->emplace_back(L"move_user", shift, GAME_DESK::NONE_CELL, GAME_DESK::NONE_CELL);

	moves.clear();
	meshMouseOver = nullptr;
	meshMouseClick = nullptr;
	statusNewGame = false;
	game->setNewArrayMesh(meshBuffer);
}

void Direct3D11Scene::recalculateMoveCoordinates(float size_shape)
{
	if (!meshMouseClick)
		return;

	meshBuffer->at(meshBuffer->size() - 4) = createMoveMesh(DESK_DIR::MOVE_UP, *meshMouseClick, { meshMouseClick->getCoordinates().row - 1, meshMouseClick->getCoordinates().column }, size_shape);
	meshBuffer->at(meshBuffer->size() - 3) = createMoveMesh(DESK_DIR::MOVE_RIGHT, *meshMouseClick, { meshMouseClick->getCoordinates().row, meshMouseClick->getCoordinates().column + 1 }, size_shape);
	meshBuffer->at(meshBuffer->size() - 2) = createMoveMesh(DESK_DIR::MOVE_DOWN, *meshMouseClick, { meshMouseClick->getCoordinates().row + 1, meshMouseClick->getCoordinates().column }, size_shape);
	meshBuffer->at(meshBuffer->size() - 1) = createMoveMesh(DESK_DIR::MOVE_LEFT, *meshMouseClick, { meshMouseClick->getCoordinates().row, meshMouseClick->getCoordinates().column - 1 }, size_shape);

	// ход на доске? (для индексов колонки и столбца)
	auto tmpDesk(meshBuffer->front());
	tmpDesk.getVertex().at(0).pos.x += size_shape;
	tmpDesk.getVertex().at(0).pos.y += size_shape;
	tmpDesk.getVertex().at(1).pos.x += size_shape;
	tmpDesk.getVertex().at(1).pos.y -= size_shape;
	tmpDesk.getVertex().at(2).pos.x -= size_shape;
	tmpDesk.getVertex().at(2).pos.y -= size_shape;
	tmpDesk.getVertex().at(3).pos.x -= size_shape;
	tmpDesk.getVertex().at(3).pos.y += size_shape;
	for (auto iter(meshBuffer->begin() + GAME_DESK::ALL_DRAW_ITEM_NO_MOVE); iter != meshBuffer->end(); ++iter) // проверка, что в пределах доски
	{
		if (!tmpDesk.rayPick(static_cast<int>(iter->getCenter().x), static_cast<int>(iter->getCenter().y)))
		{
			iter->getCoordinates() = { GAME_DESK::NONE_CELL,GAME_DESK::NONE_CELL };
		}
	}
}

Direct3D11Scene::Direct3D11Scene() : meshMouseOver(nullptr), meshMouseClick(nullptr), statusNewGame(false)
{
}

Direct3D11Scene::Direct3D11Scene(const ComPtr<ID3D11Device>& d3dDevice,
	const ComPtr<ID3D11DeviceContext>& d3dImmediateContext) : d3dDevice(d3dDevice),
	d3dImmediateContext(d3dImmediateContext), statusNewGame(false), meshMouseOver(nullptr), meshMouseClick(nullptr)
{
}

void Direct3D11Scene::init(SceneParameters rp)
{
	// загрузка всех шейдеров
	shadersBuffer = std::make_unique<ShadersBuffer>(L"5_0");
	if (!shadersBuffer)
		throw ExceptionMemory(L"ShadersBuffer is not create, memory not found!\n");
	shadersBuffer->init(d3dDevice, L"shaders/");
	//загрузка всех текстур
	texturesBuffer = std::make_unique<TexturesBuffer>();
	if (!texturesBuffer)
		throw ExceptionMemory(L"TexturesBuffer is not create, memory not found!");
	texturesBuffer->init(d3dDevice, L"textures/");

	// создание ДИНАМИЧЕСКОГО вершинного буфера
	std::vector<D3D11_INPUT_ELEMENT_DESC> layout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	auto vsBlob(shadersBuffer->getVertexShaderBlob(L"shaders/desk_default.vs"));
	auto hr(d3dDevice->CreateInputLayout(layout.data(), static_cast<UINT>(layout.size()), vsBlob->GetBufferPointer(),
		vsBlob->GetBufferSize(), inputLayout.GetAddressOf()));
	if (FAILED(hr))
		throw ExceptionDirect3D11(L"error create input layout!\n");
	constBuff = Direct3D11CreatorBuffer::createBuffer(d3dDevice, static_cast<UINT>(sizeof(ConstantBufferMain)),
		D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DEFAULT, 0);

	// game
	game = std::make_unique<Game>();
	if (!game)
		throw ExceptionMemory(L"Game is not create, memory not found!\n");
	newGame();
	std::vector<VertexSprite> vertices(meshBuffer->size() * GAME_DESK::COUNT_VERTEX);
	vertexBufferScene = Direct3D11CreatorBuffer::createBuffer(d3dDevice, static_cast<UINT>(sizeof(VertexSprite) * vertices.size()),
		D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE, vertices.data());
}

void Direct3D11Scene::resizeViewport(int w, int h)
{
	recalculateSceneCoordinates(w, h);
}

bool Direct3D11Scene::update(int w, int h)
{
	if (statusNewGame)
		newGame();
	recalculateSceneCoordinates(w, h);
	return true;
}

Direct3D11Scene::~Direct3D11Scene()
{
	clear();
}

void Direct3D11Scene::clear()
{

}

void Direct3D11Scene::checkMouseHoverChess(int w, int h, int x, int y)
{
	transformCoordinateToViewport(w, h, &x, &y);
	for (auto m(meshBuffer->begin() + GAME_DESK::DESK); m != meshBuffer->end() - GAME_DESK::MAX_MOVE; ++m)
	{
		if (m->getName() != L"desk" && m->getName() != L"black_shape" && m->rayPick(x, y))
		{
			meshMouseOver = &(*m);
			return;
		}
	}
	meshMouseOver = nullptr;
}

void Direct3D11Scene::checkMouseClickChess(int w, int h, int x, int y)
{
	auto _min(std::min(w, h));
	auto _max(std::max(w, h));
	auto width_desk(_min - GAME_DESK::BORDER_DESK * 2); // 50 * 2 -> borders
	auto size_shape(width_desk / static_cast<float>(GAME_DESK::SIZE_DESK)); // 8 -> count row or columns
	transformCoordinateToViewport(w, h, &x, &y);

	if (meshMouseClick)
	{
		Mesh m;
		if (game->makeMovePlayer(moves, x, y, &m)) // ход юзера
		{
			meshMouseClick->getVertex() = m.getVertex();
			meshMouseClick->getCoordinates() = m.getCoordinates();
			moves.clear();
			Coordinate2D from;
			Coordinate2D to;
			if (game->makeMoveAi(&from, &to)) // ход ПК
				calculateMoveAi(from, to, size_shape);
			calculateMovePlayer(size_shape); // ход юзера: обновляем графен
			recreateDynamicVertexBuffer();
			if (game->checkEndgame())
				statusNewGame = true;
			return;
		}
	}
	moves.clear();
	for (auto m(meshBuffer->begin() + GAME_DESK::DESK); m != meshBuffer->end() - GAME_DESK::MAX_MOVE; ++m)
	{
		if (m->getName() != L"desk" && m->getName() != L"black_shape" && m->rayPick(x, y))
		{
			meshMouseClick = &(*m);
			calculateMovePlayer(size_shape);
			recreateDynamicVertexBuffer();
			return;
		}
	}
	meshMouseClick = nullptr;
}

void Direct3D11Scene::render(const ComPtr<ID3D11RenderTargetView>& d3dRenderTargetView, const ComPtr<ID3D11DepthStencilView>& d3dDepthStencilView, const DirectX::XMMATRIX& ortho)
{
	std::array<float, 4> color{ 0.0f, 0.0f, 1.0f, 1.0f };
	d3dImmediateContext->ClearRenderTargetView(d3dRenderTargetView.Get(), color.data());
	d3dImmediateContext->ClearDepthStencilView(d3dDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	// рисуем игру

	d3dImmediateContext->IASetInputLayout(inputLayout.Get());
	// установка вершинного буфера
	UINT stride(sizeof(VertexSprite));
	auto offset(0u);
	// установка топологии
	d3dImmediateContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// конст
	d3dImmediateContext->VSSetConstantBuffers(0, 1, constBuff.GetAddressOf());
	//d3dImmediateContext->PSSetConstantBuffers(0, 1, c.GetAddressOf());

	// установка шейдеров
	d3dImmediateContext->VSSetShader(shadersBuffer->getVertexShader(L"shaders/desk_default.vs").Get(), nullptr, 0);
	d3dImmediateContext->PSSetShader(shadersBuffer->getPixelShader(L"shaders/desk_default.ps").Get(), nullptr, 0);

	// верш
	d3dImmediateContext->IASetVertexBuffers(0, 1, vertexBufferScene.GetAddressOf(), &stride, &offset);
	ConstantBufferMain cbm{ ortho };
	d3dImmediateContext->UpdateSubresource(constBuff.Get(), 0, nullptr, &cbm, 0, 0);

	auto shift(0);
	// рисуем
	d3dImmediateContext->PSSetShaderResources(0, 1, texturesBuffer->getTexture(L"textures/desk.dds").GetAddressOf());
	d3dImmediateContext->Draw(GAME_DESK::COUNT_VERTEX, shift);
	shift += GAME_DESK::COUNT_VERTEX;

	d3dImmediateContext->PSSetShaderResources(0, 1, texturesBuffer->getTexture(L"textures/shape_black.dds").GetAddressOf());
	for (auto i(0); i < GAME_DESK::BLACK_SHAPES; ++i, shift += GAME_DESK::COUNT_VERTEX)
		d3dImmediateContext->Draw(GAME_DESK::COUNT_VERTEX, shift);

	d3dImmediateContext->PSSetShaderResources(0, 1, texturesBuffer->getTexture(L"textures/shape_white.dds").GetAddressOf());
	for (auto i(0); i < GAME_DESK::WHITE_SHAPES; ++i, shift += GAME_DESK::COUNT_VERTEX)
		d3dImmediateContext->Draw(GAME_DESK::COUNT_VERTEX, shift);

	if (meshMouseClick)
	{
		d3dImmediateContext->PSSetShaderResources(0, 1, texturesBuffer->getTexture(L"textures/shape_mouse_hover.dds").GetAddressOf());
		d3dImmediateContext->Draw(GAME_DESK::COUNT_VERTEX, meshMouseClick->getShiftVertex());
	}

	if (meshMouseOver)
	{
		d3dImmediateContext->PSSetShader(shadersBuffer->getPixelShader(L"shaders/shape_mouse_hover.ps").Get(), nullptr, 0);
		d3dImmediateContext->PSSetShaderResources(0, 1, texturesBuffer->getTexture(L"textures/shape_black.dds").GetAddressOf());
		d3dImmediateContext->Draw(GAME_DESK::COUNT_VERTEX, meshMouseOver->getShiftVertex());
	}

	d3dImmediateContext->PSSetShader(shadersBuffer->getPixelShader(L"shaders/move.ps").Get(), nullptr, 0);
	for (auto&& m : moves)
	{
		d3dImmediateContext->Draw(m, shift); // draw move
		shift += GAME_DESK::COUNT_VERTEX;
	}
}

// залить на гит, описание, и пара скринов