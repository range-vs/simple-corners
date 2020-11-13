#pragma once

#include <exception>
#include <string>

class IException
{
protected:
	std::wstring error;
public:
	IException(const wchar_t* e)noexcept { error = e; };
	IException(const std::wstring& e)noexcept { error = e; };
	const wchar_t* what() const noexcept {return error.c_str();};
};

class ExceptionMemory : public IException
{
public:
	ExceptionMemory(const wchar_t* e)noexcept :IException(e) {};
	ExceptionMemory(const std::wstring& e)noexcept :IException(e) {};
	const wchar_t* what() const noexcept { return IException::what();  };
};

class ExceptionWindow : public IException
{
public:
	ExceptionWindow(const wchar_t* e)noexcept :IException(e) {};
	ExceptionWindow(const std::wstring& e)noexcept :IException(e) {};
	const wchar_t* what() const noexcept { return IException::what(); };
};

class ExceptionDirect3D11 : public IException
{
public:
	ExceptionDirect3D11(const wchar_t* e)noexcept :IException(e) {};
	ExceptionDirect3D11(const std::wstring& e)noexcept :IException(e) {};
	const wchar_t* what() const noexcept { return IException::what(); };
};

class ExceptionShaders : public IException
{
public:
	ExceptionShaders(const wchar_t* e)noexcept :IException(e) {};
	ExceptionShaders(const std::wstring& e)noexcept :IException(e) {};
	const wchar_t* what() const noexcept { return IException::what(); };
};

class ExceptionTextures : public IException
{
public:
	ExceptionTextures(const wchar_t* e)noexcept :IException(e) {};
	ExceptionTextures(const std::wstring& e)noexcept :IException(e) {};
	const wchar_t* what() const noexcept { return IException::what(); };
};