#pragma once

#include "cvconst.h"
#include "dbghelp.h"

template <class F>
HRESULT try_catch(F fn)
{
	try {
		return fn();
	} catch (HRESULT hr) {
		//ATLTRACE(_T("try_catch HRESULT(%x)\n"), hr);
		return hr;
	} catch (...) {
		HRESULT hr = HRESULT_FROM_WIN32(::GetLastError());
		//ATLTRACE(_T("try_catch ...(hr=%x)\n"), hr);
		return hr;
	}
}

template <class T>
HRESULT CreateInsatnce(std::shared_ptr<T>* ppNew)
{
	if (!ppNew)
		return E_POINTER;
	auto fn = [&]() {
		*ppNew  = std::make_shared<T>();
		return S_OK; 
	};
	return try_catch(fn);

}

template <class T, class A>
HRESULT CreateInsatnce(A arg1, std::shared_ptr<T>* ppNew)
{
	if (!ppNew)
		return E_POINTER;
	auto fn = [&]() {
		*ppNew  = std::make_shared<T>(arg1);
		return S_OK; 
	};
	return try_catch(fn);

}

template <class T, class A1, class A2>
HRESULT CreateInsatnce(A1 arg1, A2 arg2, std::shared_ptr<T>* ppNew)
{
	if (!ppNew)
		return E_POINTER;
	auto fn = [&]() {
		*ppNew  = std::make_shared<T>(arg1, arg2);
		return S_OK; 
	};
	return try_catch(fn);

}

template <class T, class A1, class A2, class A3>
HRESULT CreateInsatnce(A1 arg1, A2 arg2, A3 arg3, std::shared_ptr<T>* ppNew)
{
	if (!ppNew)
		return E_POINTER;
	auto fn = [&]() {
		*ppNew  = std::make_shared<T>(arg1, arg2, arg3);
		return S_OK; 
	};
	return try_catch(fn);

}
