#include "stdafx.h"
#include "DbgEngine.h"

//-----------------------------------------------------------------
typedef PIMAGE_NT_HEADERS STDAPICALLTYPE FImageNtHeader(
    __in PVOID Base
    );

typedef BOOL STDAPICALLTYPE FSymInitialize(
    __in HANDLE hProcess,
    __in_opt PCSTR UserSearchPath,
    __in BOOL fInvadeProcess
    );

typedef BOOL STDAPICALLTYPE FSymCleanup(
    __in HANDLE hProcess
    );

typedef DWORD STDAPICALLTYPE FSymSetOptions(
    __in DWORD   SymOptions
    );

typedef DWORD STDAPICALLTYPE FSymGetOptions(
    VOID
    );

typedef DWORD64 STDAPICALLTYPE FSymLoadModule64(
    __in HANDLE hProcess,
    __in_opt HANDLE hFile,
    __in_opt PCSTR ImageName,
    __in_opt PCSTR ModuleName,
    __in DWORD64 BaseOfDll,
    __in DWORD SizeOfDll
    );

typedef BOOL STDAPICALLTYPE FSymUnloadModule64(
    __in HANDLE hProcess,
    __in DWORD64 BaseOfDll
    );

typedef BOOL STDAPICALLTYPE FSymGetModuleInfo64(
    __in HANDLE hProcess,
    __in DWORD64 qwAddr,
    __out PIMAGEHLP_MODULE64 ModuleInfo
    );

typedef BOOL STDAPICALLTYPE FSymGetTypeInfo(
    __in HANDLE hProcess,
    __in DWORD64 ModBase,
    __in ULONG TypeId,
    __in IMAGEHLP_SYMBOL_TYPE_INFO GetType,
    __out PVOID pInfo
    );

typedef BOOL STDAPICALLTYPE FSymSetContext(
    __in HANDLE hProcess,
    __in PIMAGEHLP_STACK_FRAME StackFrame,
    __in_opt PIMAGEHLP_CONTEXT Context
    );

typedef BOOL STDAPICALLTYPE FSymEnumSymbols(
    __in HANDLE hProcess,
    __in ULONG64 BaseOfDll,
    __in_opt PCSTR Mask,
    __in PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    __in_opt PVOID UserContext
    );

typedef BOOL STDAPICALLTYPE FSymEnumTypes(
    __in HANDLE hProcess,
    __in ULONG64 BaseOfDll,
    __in PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback,
    __in_opt PVOID UserContext
    );

typedef BOOL STDAPICALLTYPE FSymGetLineFromAddr64(
    __in HANDLE hProcess,
    __in DWORD64 qwAddr,
    __out PDWORD pdwDisplacement,
    __out PIMAGEHLP_LINE64 Line64
    );

#define PPCAT(X, Y) PPCAT_I(X, Y)
#define PPCAT_I(X, Y) X ## Y

#define GET_PROC(NAME) \
	{ \
		PPCAT(p,NAME) = (PPCAT(F,NAME) *)::GetProcAddress(hDbgHelp, #NAME); \
		if (PPCAT(p,NAME) == NULL) { \
			return HRESULT_FROM_WIN32(::GetLastError()); \
		} \
	}

//-----------------------------------------------------------------
struct CDbgEngine::impl {
	HANDLE process;
	HMODULE dbghelp;

	FImageNtHeader *pImageNtHeader;
	FSymInitialize *pSymInitialize;
	FSymCleanup *pSymCleanup;
	FSymSetOptions *pSymSetOptions;
	FSymGetOptions *pSymGetOptions;
	FSymLoadModule64 *pSymLoadModule64;
	FSymUnloadModule64 *pSymUnloadModule64;
	FSymGetModuleInfo64 *pSymGetModuleInfo64;
	FSymGetTypeInfo *pSymGetTypeInfo;
	FSymSetContext *pSymSetContext;
	FSymEnumSymbols *pSymEnumSymbols;
	FSymEnumTypes *pSymEnumTypes;
	FSymGetLineFromAddr64 *pSymGetLineFromAddr64;

	HRESULT Init(HMODULE hProcess)
	{
		HMODULE hDbgHelp = LoadLibraryA("dbghelp.dll");
		if (hDbgHelp  == NULL) {
			//fprintf(stderr, "Not found 'dbghelp.dll'.\n");
			return HRESULT_FROM_WIN32(::GetLastError());
		}

		GET_PROC(ImageNtHeader)
		GET_PROC(SymInitialize)
		GET_PROC(SymCleanup)
		GET_PROC(SymSetOptions)
		GET_PROC(SymGetOptions)
		GET_PROC(SymLoadModule64)
		GET_PROC(SymUnloadModule64)
		GET_PROC(SymGetModuleInfo64)
		GET_PROC(SymGetTypeInfo)
		GET_PROC(SymSetContext)
		GET_PROC(SymEnumSymbols)
		GET_PROC(SymEnumTypes)
		GET_PROC(SymGetLineFromAddr64)

		if (!pSymInitialize(hProcess, NULL, FALSE)) {
			//fprintf(stderr, "Couldn't initialize the dbghelp.dll.\n");
			return HRESULT_FROM_WIN32(::GetLastError());
		}

		process = hProcess;
		dbghelp = hDbgHelp;
		return S_OK;
	}

	void Uninit()
	{
		if (process == NULL) {
			return;
		}

		if (dbghelp != NULL) {
			if (pSymCleanup != NULL) {
				pSymCleanup(process);
			}
			FreeLibrary(dbghelp);
		}
	}

	~impl()
	{
		Uninit();
	}
};

//-----------------------------------------------------------------
CDbgEngine::CDbgEngine(HMODULE hProcess) : m_pimpl(new impl)
{
	HRESULT hr = m_pimpl->Init(hProcess);
	if (FAILED(hr))
		throw hr;
}

CDbgEngine::~CDbgEngine(void)
{
}

//-----------------------------------------------------------------
// dbghelp.h API
HANDLE CDbgEngine::GetProcess()
{
	return m_pimpl->process;
}

PIMAGE_NT_HEADERS CDbgEngine::ImageNtHeader(PVOID Base)
{
	return m_pimpl->pImageNtHeader(Base);
}

DWORD64 CDbgEngine::SymLoadModule64(HANDLE hFile, PCSTR ImageName, PCSTR ModuleName, DWORD64 BaseOfDll, DWORD SizeOfDll)
{
	return m_pimpl->pSymLoadModule64(GetProcess(), hFile, ImageName, ModuleName, BaseOfDll, SizeOfDll);
}

BOOL CDbgEngine::SymUnloadModule64(DWORD64 BaseOfDll)
{
	return m_pimpl->pSymUnloadModule64(GetProcess(), BaseOfDll);
}

BOOL CDbgEngine::SymGetModuleInfo64(DWORD64 qwAddr, PIMAGEHLP_MODULE64 ModuleInfo)
{
	return m_pimpl->pSymGetModuleInfo64(GetProcess(), qwAddr, ModuleInfo);
}

BOOL CDbgEngine::SymGetTypeInfo(DWORD64 ModBase, ULONG TypeId, IMAGEHLP_SYMBOL_TYPE_INFO GetType, PVOID pInfo)
{
	return m_pimpl->pSymGetTypeInfo(GetProcess(), ModBase, TypeId, GetType, pInfo);
}

BOOL CDbgEngine::SymSetContext(PIMAGEHLP_STACK_FRAME StackFrame, PIMAGEHLP_CONTEXT Context)
{
	return m_pimpl->pSymSetContext(GetProcess(), StackFrame, Context);
}

BOOL CDbgEngine::SymEnumSymbols(ULONG64 BaseOfDll, PCSTR Mask, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext)
{
	return m_pimpl->pSymEnumSymbols(GetProcess(), BaseOfDll, Mask, EnumSymbolsCallback, UserContext);
}

BOOL CDbgEngine::SymEnumTypes(ULONG64 BaseOfDll, PSYM_ENUMERATESYMBOLS_CALLBACK EnumSymbolsCallback, PVOID UserContext)
{
	return m_pimpl->pSymEnumTypes(GetProcess(), BaseOfDll, EnumSymbolsCallback, UserContext);
}

BOOL CDbgEngine::SymGetLineFromAddr64(DWORD64 qwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINE64 Line64)
{
	return m_pimpl->pSymGetLineFromAddr64(GetProcess(), qwAddr, pdwDisplacement, Line64);
}


// helper
LPCTSTR CDbgEngine::GetTagName(enum SymTagEnum tag)
{
/*
	SymTagNull,
    SymTagExe,
    SymTagCompiland,
    SymTagCompilandDetails,
    SymTagCompilandEnv,
    SymTagFunction,
    SymTagBlock,
    SymTagData,
    SymTagAnnotation,
    SymTagLabel,
    SymTagPublicSymbol,
    SymTagUDT,
    SymTagEnum,
    SymTagFunctionType,
    SymTagPointerType,
    SymTagArrayType,
    SymTagBaseType,
    SymTagTypedef,
    SymTagBaseClass,
    SymTagFriend,
    SymTagFunctionArgType,
    SymTagFuncDebugStart,
    SymTagFuncDebugEnd,
    SymTagUsingNamespace,
    SymTagVTableShape,
    SymTagVTable,
    SymTagCustom,
    SymTagThunk,
    SymTagCustomType,
    SymTagManagedType,
    SymTagDimension,
    SymTagMax
*/
	static LPCTSTR s_tagNames[] = {
		_T(""),
		_T("Executable (Global)"),
		_T("Compiland"), 
		_T("CompilandDetails"), 
		_T("CompilandEnv"),
		_T("Function"), 
		_T("Block"),
		_T("Data"),
		_T("Unused"),
		_T("Label"),
		_T("PublicSymbol"),
		_T("UDT"),
		_T("Enum"),
		_T("FunctionType"),
		_T("PointerType"),
		_T("ArrayType"),
		_T("BaseType"),
		_T("Typedef"),
		_T("BaseClass"),
		_T("Friend"),
		_T("FunctionArgType"),
		_T("FuncDebugStart"),
		_T("FuncDebugEnd"),
		_T("UsingNamespace"),
		_T("VTableShape"),
		_T("VTable"),
		_T("Custom"),
		_T("Thunk"),
		_T("CustomType"),
		_T("ManagedType"),
		_T("Dimension"),
		_T("")
	};

	if (tag < SymTagNull || SymTagMax < tag) {
		return NULL;
	}

	return s_tagNames[tag];
}

bool CDbgEngine::IsScope(enum SymTagEnum tag)
{
	static bool s_isScopes[] = {
		false,
		true,
		true,
		false,
		false,
		true,
		true,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false
	};

	if (tag < SymTagNull || SymTagMax <= tag) {
		return NULL;
	}

	return s_isScopes[tag];
}

LPCTSTR CDbgEngine::GetBasicTypeName(BasicType type, ULONG length)
{
/*
	btNoType = 0,
    btVoid = 1,
    btChar = 2,
    btWChar = 3,
    btInt = 6,
    btUInt = 7,
    btFloat = 8,
    btBCD = 9,
    btBool = 10,
    btLong = 13,
    btULong = 14,
    btCurrency = 25,
    btDate = 26,
    btVariant = 27,
    btComplex = 28,
    btBit = 29,
    btBSTR = 30,
    btHresult = 31
*/
	switch (type) {
	case btNoType:
		return _T("notype");
	case btVoid:
		return _T("void");
	case btChar:
		return _T("char");
	case btWChar :
		return _T("wchar_t");
	case btInt:
		switch (length) {
		case 1:
			return _T("signed char");
		case 2:
			return _T("short");
		case 4:
			return _T("int");
		case 8:
			return _T("__int64");
		default:
			return _T("signed type");
		}
		break;
	case btUInt:
		switch (length) {
		case 1:
			return _T("unsigned char");
		case 2:
			return _T("unsigned short");
		case 4:
			return _T("unsigned int");
		case 8:
			return _T("unsigned __int64");
		default:
			return _T("unsigned type");
		}
		break;
	case btFloat:
		return _T("float");
	case btBCD:
		return _T("BCD");
	case btBool:
		return _T("bool");
	case btLong:
		return _T("long");
	case btULong:
		return _T("unsigned long");
	case btCurrency:
		return _T("CURRENCY");
	case btDate:
		return _T("DATE");
	case btVariant:
		return _T("VARIANT");
	case btComplex:
		return _T("COMPLEX");
	case btBit:
		return _T("bit");
	case btBSTR:
		return _T("BSTR");
	case btHresult:
		return _T("HRESULT");
	}

	assert(false);
	return NULL;
}

LPCTSTR CDbgEngine::GetUdtKindName(UdtKind kind)
{
/*
    UdtStruct,
    UdtClass,
    UdtUnion
*/
	switch (kind) {
	case UdtStruct:
		return _T("Struct");
	case UdtClass:
		return _T("Class");
	case UdtUnion:
		return _T("Union");
	}

	assert(false);
	return NULL;
}

LPCTSTR CDbgEngine::GetDataKindName(DataKind kind)
{
/*
    DataIsUnknown,
    DataIsLocal,
    DataIsStaticLocal,
    DataIsParam,
    DataIsObjectPtr,
    DataIsFileStatic,
    DataIsGlobal,
    DataIsMember,
    DataIsStaticMember,
	DataIsConstant
*/
	switch (kind) {
	case DataIsUnknown:
		return _T("Unknown");
	case DataIsLocal:
		return _T("Local");
	case DataIsStaticLocal:
		return _T("StaticLocal");
	case DataIsParam:
		return _T("Param");
	case DataIsObjectPtr:
		return _T("ObjectPtr");
	case DataIsFileStatic:
		return _T("FileStatic");
	case DataIsGlobal:
		return _T("Global");
	case DataIsMember:
		return _T("Member");
	case DataIsStaticMember:
		return _T("StaticMember");
	case DataIsConstant:
		return _T("Constant");
	}

	assert(false);
	return NULL;
}

LPCTSTR CDbgEngine::GetCallingConversionName(CV_call_e call)
{
/*
	CV_CALL_NEAR_C      = 0x00, // near right to left push, caller pops stack
    CV_CALL_FAR_C       = 0x01, // far right to left push, caller pops stack
    CV_CALL_NEAR_PASCAL = 0x02, // near left to right push, callee pops stack
    CV_CALL_FAR_PASCAL  = 0x03, // far left to right push, callee pops stack
    CV_CALL_NEAR_FAST   = 0x04, // near left to right push with regs, callee pops stack
    CV_CALL_FAR_FAST    = 0x05, // far left to right push with regs, callee pops stack
    CV_CALL_SKIPPED     = 0x06, // skipped (unused) call index
    CV_CALL_NEAR_STD    = 0x07, // near standard call
    CV_CALL_FAR_STD     = 0x08, // far standard call
    CV_CALL_NEAR_SYS    = 0x09, // near sys call
    CV_CALL_FAR_SYS     = 0x0a, // far sys call
    CV_CALL_THISCALL    = 0x0b, // this call (this passed in register)
    CV_CALL_MIPSCALL    = 0x0c, // Mips call
    CV_CALL_GENERIC     = 0x0d, // Generic call sequence
    CV_CALL_ALPHACALL   = 0x0e, // Alpha call
    CV_CALL_PPCCALL     = 0x0f, // PPC call
    CV_CALL_SHCALL      = 0x10, // Hitachi SuperH call
    CV_CALL_ARMCALL     = 0x11, // ARM call
    CV_CALL_AM33CALL    = 0x12, // AM33 call
    CV_CALL_TRICALL     = 0x13, // TriCore Call
    CV_CALL_SH5CALL     = 0x14, // Hitachi SuperH-5 call
    CV_CALL_M32RCALL    = 0x15, // M32R Call
    CV_CALL_RESERVED    = 0x16  // first unused call enumeration
*/
	switch (call) {
	case CV_CALL_NEAR_C:
	case CV_CALL_FAR_C:
		return _T("__cdecl");
	case CV_CALL_NEAR_PASCAL:
	case CV_CALL_FAR_PASCAL:
		return _T("__pascal");
	case CV_CALL_NEAR_FAST:
	case CV_CALL_FAR_FAST:
		return _T("__fastcall");
	case CV_CALL_SKIPPED:
		return _T("__skipped");
	case CV_CALL_NEAR_STD:
	case CV_CALL_FAR_STD:
		return _T("__stdcall");
	case CV_CALL_NEAR_SYS:
	case CV_CALL_FAR_SYS:
		return _T("__syscall");
	case CV_CALL_THISCALL:
		return _T("__thiscall");
	case CV_CALL_MIPSCALL:
		return _T("__call");
	case CV_CALL_GENERIC:
		return _T("__generic");
	case CV_CALL_ALPHACALL:
		return _T("__alphacall");
	case CV_CALL_PPCCALL:
		return _T("__prccall");
	case CV_CALL_SHCALL:
		return _T("__shcall");
	case CV_CALL_ARMCALL:
		return _T("__armcall");
	case CV_CALL_AM33CALL:
		return _T("__am33call");
	case CV_CALL_TRICALL:
		return _T("__tricall");
	case CV_CALL_SH5CALL:
		return _T("__sh5call");
	case CV_CALL_M32RCALL:
		return _T("__m32call");
	case CV_CALL_CLRCALL:
		return _T("__clrcall");
	case CV_CALL_RESERVED:
		return _T("***RESERVED***");
	}

	assert(false);
	return NULL;
}

LPCTSTR CDbgEngine::GetRegName(CV_HREG_e reg)
{
	switch (reg) {
	case CV_REG_AL: return _T("al");
	case CV_REG_CL: return _T("cl");
	case CV_REG_DL: return _T("dl");
	case CV_REG_BL: return _T("bl");
	case CV_REG_AH: return _T("ah");
	case CV_REG_CH: return _T("ch");
	case CV_REG_DH: return _T("dh");
	case CV_REG_BH: return _T("bh");
	case CV_REG_AX: return _T("ax");
	case CV_REG_CX: return _T("cx");
	case CV_REG_DX: return _T("dx");
	case CV_REG_BX: return _T("bx");
	case CV_REG_SP: return _T("sp");
	case CV_REG_BP: return _T("bp");
	case CV_REG_SI: return _T("si");
	case CV_REG_DI: return _T("di");
	case CV_REG_EAX: return _T("eax");
	case CV_REG_ECX: return _T("ecx");
	case CV_REG_EDX: return _T("edx");
	case CV_REG_EBX: return _T("ebx");
	case CV_REG_ESP: return _T("esp");
	case CV_REG_EBP: return _T("ebp");
	case CV_REG_ESI: return _T("esi");
	case CV_REG_EDI: return _T("edi");
	case CV_REG_ES: return _T("es");
	case CV_REG_CS: return _T("cs");
	case CV_REG_SS: return _T("ss");
	case CV_REG_DS: return _T("ds");
	case CV_REG_FS: return _T("fs");
	case CV_REG_GS: return _T("gs");
	case CV_REG_IP: return _T("ip");
	case CV_REG_FLAGS: return _T("flags");
	case CV_REG_EIP: return _T("eip");
	case CV_REG_EFLAGS: return _T("eflags");
	}

	assert(false);
	return _T("");
}
