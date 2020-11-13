#pragma once

#include "d3d11_headers.h"

#include <map>
#include <filesystem>

namespace fs = std::filesystem;

enum class TypeShader : unsigned
{
	VertexShaderType = 0,
	PixelShaderType,
	ComputerShaderType,
	GeometryShaderTYpe
};

class ShadersBuffer
{
	std::map<std::wstring, ComPtr<ID3D11VertexShader>> vertexShaders;
	std::map<std::wstring, ComPtr<ID3D11PixelShader>> pixelShaders;
	std::map<std::wstring, ComPtr<ID3DBlob>> vertexShadersBlobs;
	std::map<std::wstring, ComPtr<ID3DBlob>> pixelShadersBlobs;

	std::string ver;
	ComPtr<ID3DBlob> compileShaderFromFile(const std::wstring& fileName, LPCSTR entryPoint, LPCSTR shaderModel);
	void load(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring& p, const TypeShader& ts);

public:
	ShadersBuffer(const ShadersBuffer&) = delete;
	ShadersBuffer& operator=(const ShadersBuffer&) = delete;
	ShadersBuffer() = default;
	ShadersBuffer(const std::wstring& ver);

	void init(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring& path);

	void clear();
	~ShadersBuffer();

	ComPtr<ID3D11VertexShader> getVertexShader(const std::wstring& key);
	ComPtr<ID3DBlob> getVertexShaderBlob(const std::wstring& key);
	ComPtr<ID3D11PixelShader> getPixelShader(const std::wstring& key);
	ComPtr<ID3DBlob> getPixelShaderBlob(const std::wstring& key);
};
