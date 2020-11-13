#include "TexturesBuffer.h"

void TexturesBuffer::init(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring path)
{
	// create default sampler
	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;      // Тип фильтрации
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;         // Задаем координаты
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	// Создаем интерфейс сэмпла текстурирования
	ComPtr<ID3D11SamplerState> samplerDefault;
	auto hr(d3dDevice->CreateSamplerState(&sampDesc, samplerDefault.GetAddressOf()));
	if (FAILED(hr))
		throw ExceptionTextures(L"Error create default sampler in Direct3D11 render\n");
	bufferSamples.emplace(L"default", samplerDefault);

	fs::path shaderCatalog(path);
	for (fs::recursive_directory_iterator iter(shaderCatalog); iter != fs::recursive_directory_iterator(); ++iter)
	{
		std::wstring name(iter->path().wstring());
		std::replace(name.begin(), name.end(), L'\\', L'/');
		std::wstring ext(iter->path().extension().wstring());
		if (isImg(ext))
			add(d3dDevice, name);
	}
}

bool TexturesBuffer::isImg(std::wstring& ex)
{
	std::array<std::wstring, 7> _ex =
	{
		L".dds", L".png", L".jpg", L".jpeg", L".tga", L".bmp", L".gif"
	};
	std::transform(ex.begin(), ex.end(), ex.begin(), towlower);
	return !(std::find(_ex.begin(), _ex.end(), ex) == _ex.end());
}

TexturesBuffer::TexturesBuffer()
{
}

void TexturesBuffer::add(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring path)
{
	ComPtr<ID3D11ShaderResourceView> textureTarget(nullptr);
	auto hr(DirectX::CreateDDSTextureFromFile(d3dDevice.Get(), path.c_str(), nullptr, &textureTarget));
	if (FAILED(hr)) 
		throw ExceptionTextures(L"Error load texture " + path + L"\n");
	bufferTextures.emplace(path, textureTarget);

}

auto TexturesBuffer::getTexture(const std::wstring& key) -> ComPtr<ID3D11ShaderResourceView>&
{
	if (bufferTextures.find(key) == bufferTextures.end())
		throw ExceptionTextures(L"error find texture - " + key + L"\n");
	return bufferTextures[key];
}

auto TexturesBuffer::getSampler(const std::wstring& key) -> ComPtr<ID3D11SamplerState>&
{
	if (bufferSamples.find(key) == bufferSamples.end())
		throw ExceptionTextures(L"error find sampler - " + key + L"\n");
	return bufferSamples[key];
}
