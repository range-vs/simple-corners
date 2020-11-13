#include "ShadersBuffer.h"

#include "Exceptions.h"

ShadersBuffer::ShadersBuffer(const std::wstring& ver):ver(ver.begin(), ver.end())
{
}

ComPtr<ID3DBlob> ShadersBuffer::compileShaderFromFile(const std::wstring& fileName, LPCSTR entryPoint, LPCSTR shaderModel)
{
	ID3DBlob* error(nullptr);
	ID3DBlob* blob; // выходной шейдер
	UINT flags(0);
#ifdef DEBUG
	flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	auto hr(D3DCompileFromFile(fileName.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, shaderModel, flags,
		0, &blob, &error));
	std::string err;
	if (error)
	{
		for (auto i(0); i < error->GetBufferSize(); i++)
			err += ((char*)(error->GetBufferPointer()))[i];
		std::wstring mes(err.begin(), err.end());
		throw ExceptionShaders(L"path shader - " + fileName + L"\nMessage error: " + mes + L"\n\n");
	}
	if (hr != S_OK)
		return nullptr;
	return blob;
}


void ShadersBuffer::init(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring& p)
{
	fs::path shaderCatalog(p);
	for (fs::recursive_directory_iterator iter(shaderCatalog); iter != fs::recursive_directory_iterator(); ++iter)
	{
		// потом прикрутить к созданию шейдера фабрику, по расширению
		std::wstring name = iter->path().wstring();
		if (iter->path().extension().wstring() == L".vs")
			load(d3dDevice, name, TypeShader::VertexShaderType);
		else if (iter->path().extension().wstring() == L".ps")
			load(d3dDevice, name, TypeShader::PixelShaderType);
		// etc
	}
}

void ShadersBuffer::load(const ComPtr<ID3D11Device>& d3dDevice, const std::wstring& p, const TypeShader& ts)
{
	ComPtr<ID3D11VertexShader> d3dVertexShader;
	ComPtr<ID3D11PixelShader> d3dPixelShader;
	ComPtr<ID3DBlob> blob;

	HRESULT hr;
	std::string _ver;
	switch (ts)
	{
	case TypeShader::VertexShaderType:
		_ver = "vs_" + ver;
		blob = compileShaderFromFile(p, "main", _ver.c_str());
		hr = d3dDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &d3dVertexShader);
		if (FAILED(hr))
			throw ExceptionShaders(L"error create vertex shader - " + p);
		vertexShaders.emplace(p, d3dVertexShader);
		vertexShadersBlobs.emplace(p, blob);
		break;

	case TypeShader::PixelShaderType:
		_ver = "ps_" + ver;
		blob = compileShaderFromFile(p, "main", _ver.c_str());
		hr = d3dDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), NULL, &d3dPixelShader);
		if (FAILED(hr))
			throw ExceptionShaders(L"error create pixel shader - " + p);
		pixelShaders.emplace( p, d3dPixelShader );
		pixelShadersBlobs.emplace( p, blob );
		break;

	default:
		debug::printError(L"This shader functional is not released\n");
		break;
	}
}

void ShadersBuffer::clear()
{

}

ShadersBuffer::~ShadersBuffer()
{
	clear();
}

ComPtr<ID3D11VertexShader> ShadersBuffer::getVertexShader(const std::wstring& key)
{
	if(vertexShaders.find(key) == vertexShaders.end())
		throw ExceptionShaders(L"error find vertex shader - " + key);
	return vertexShaders[key];
}

ComPtr<ID3DBlob> ShadersBuffer::getVertexShaderBlob(const std::wstring& key)
{
	if (vertexShadersBlobs.find(key) == vertexShadersBlobs.end())
		throw ExceptionShaders(L"error find vertex shader blob - " + key);
	return vertexShadersBlobs[key];
}

ComPtr<ID3D11PixelShader> ShadersBuffer::getPixelShader(const std::wstring& key)
{
	if (pixelShaders.find(key) == pixelShaders.end())
		throw ExceptionShaders(L"error find pixel shader - " + key);
	return pixelShaders[key];
}

ComPtr<ID3DBlob> ShadersBuffer::getPixelShaderBlob(const std::wstring& key)
{
	if (pixelShadersBlobs.find(key) == pixelShadersBlobs.end())
		throw ExceptionShaders(L"error find vertex shader blob - " + key);
	return pixelShadersBlobs[key];
}