#pragma once

#include "d3d11_headers.h"

#include "Exceptions.h"

#include <map>
#include <string>
#include <array>

#include <filesystem>

namespace fs = std::filesystem;

class TexturesBuffer
{
	std::map<std::wstring, ComPtr<ID3D11ShaderResourceView>> bufferTextures;
	std::map<std::wstring, ComPtr<ID3D11SamplerState>> bufferSamples;

	void add(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring path);
	bool isImg(std::wstring& ex);

public:
	TexturesBuffer();
	TexturesBuffer(const TexturesBuffer&) = delete;
	TexturesBuffer& operator=(const TexturesBuffer&) = delete;
	void init(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring path);

	auto getTexture(const std::wstring& s)->ComPtr<ID3D11ShaderResourceView>&;
	auto getSampler(const std::wstring& s)->ComPtr<ID3D11SamplerState>&;

};
