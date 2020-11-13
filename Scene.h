#pragma once

#define NOMINMAX

#include "Exceptions.h"
#include "ShadersBuffer.h"
#include "TexturesBuffer.h"
#include "Mesh.h"
#include "Game.h"

class SceneParameters
{
	int w;
	int h;

public:
	int getClientWidth()const noexcept;
	int getClientHeight()const noexcept;

	void setClientWidth(int w)noexcept;
	void setClientHeight(int h)noexcept;
};

struct ConstantBufferMain
{
	DirectX::XMMATRIX ortho;
};

class Direct3D11Scene
{
	std::unique_ptr<ShadersBuffer> shadersBuffer;
	std::unique_ptr<TexturesBuffer> texturesBuffer;
	ComPtr<ID3D11Device> d3dDevice;
	ComPtr<ID3D11DeviceContext> d3dImmediateContext;

	ComPtr<ID3D11InputLayout> inputLayout;
	ComPtr<ID3D11Buffer> constBuff;
	ComPtr<ID3D11Buffer> vertexBufferScene;

	// game
	std::unique_ptr<Game> game;
	ArrayMesh meshBuffer;
	// no delete!!!
		Mesh* meshMouseOver;
		Mesh* meshMouseClick;
	//
	std::vector<int> moves; // shifts moves (in meshBuffer), -1 -> no draw
	bool statusNewGame;

	void transformCoordinateToViewport(int w, int h, int* x, int* y)const noexcept;
	void recreateDynamicVertexBuffer();
	void recalculateSceneCoordinates(int w, int h);
	void calculateMovePlayer(float size_shape);
	void calculateMoveAi(const Coordinate2D& from, const Coordinate2D& to, float size_shape);
	void recalculateMoveCoordinates(float size_shape);
	DESK_DIR checkPositionMove(const Coordinate2D& from, const Coordinate2D& to);
	Mesh createMoveMesh(const DESK_DIR& dir, Mesh m, Coordinate2D coord, float size_shape);
	void newGame();

public:
	Direct3D11Scene();
	Direct3D11Scene(const Direct3D11Scene&) = delete;
	Direct3D11Scene& operator=(const Direct3D11Scene&) = delete;
	Direct3D11Scene(const ComPtr<ID3D11Device>& d3dDevice, const ComPtr<ID3D11DeviceContext>& d3dImmediateContext);
	void init(SceneParameters rp);
	void resizeViewport(int w, int h);

	bool update(int w, int h);

	~Direct3D11Scene();
	void clear();

	void checkMouseHoverChess(int w, int h, int x, int y);
	void checkMouseClickChess(int w, int h, int x, int y);
	void render(const ComPtr<ID3D11RenderTargetView>& d3dRenderTargetView,
		const ComPtr<ID3D11DepthStencilView>& d3dDepthStencilView,
		const DirectX::XMMATRIX& ortho);
};
