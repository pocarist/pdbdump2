#pragma once

class CDbgEngine
{
	struct impl;
	std::unique_ptr<impl> m_pimpl;

public:
	//CDbgEngine();
	explicit CDbgEngine(HMODULE hProcess);
	virtual ~CDbgEngine(void);

public:
	// dbghelp.h API
	HANDLE GetProcess();
	PIMAGE_NT_HEADERS ImageNtHeader(PVOID Base);
	DWORD64 SymLoadModule64(HANDLE hFile, PCSTR ImageName, PCSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll);
	BOOL SymUnloadModule64(DWORD64 BaseOfDll);
	BOOL SymGetModuleInfo64(DWORD64 qwAddr, PIMAGEHLP_MODULE64 ModuleInfo);
	BOOL SymGetTypeInfo(DWORD64 ModBase, ULONG TypeId, IMAGEHLP_SYMBOL_TYPE_INFO GetType, PVOID pInfo);
	BOOL SymSetContext(PIMAGEHLP_STACK_FRAME StackFrame, PIMAGEHLP_CONTEXT Context);
	BOOL SymEnumSymbols(ULONG64 BaseOfDll, PCSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext);
	BOOL SymEnumTypes(ULONG64 BaseOfDll, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext);
	BOOL SymGetLineFromAddr64(DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64);

	// helper
	static LPCTSTR GetTagName(enum SymTagEnum tag);
	static bool IsScope(enum SymTagEnum tag);
	static LPCTSTR GetBasicTypeName(BasicType type, ULONG length = 0);
	static LPCTSTR GetUdtKindName(UdtKind kind);
	static LPCTSTR GetDataKindName(DataKind kind);
	static LPCTSTR GetCallingConversionName(CV_call_e call);
	static LPCTSTR GetRegName(CV_HREG_e reg);

};

	