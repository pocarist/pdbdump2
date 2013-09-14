#include "stdafx.h"
#include "DbgSymbol.h"
#include "DbgEngine.h"
#include "DbgType.h"


//-----------------------------------------------------------------
struct CDbgSymbol::impl {
	std::shared_ptr<CDbgEngine> engine;
	SYMBOL_INFO info;
	std::wstring name;
	std::shared_ptr<CDbgType> type;

	HRESULT Init(std::shared_ptr<CDbgEngine> _engine, const SYMBOL_INFO* _info)
	{
		engine = _engine;
		info = *_info;
		name = to_wstring(std::string(_info->Name, _info->NameLen));
		HRESULT hr = CreateInsatnce<CDbgType>(engine, info.ModBase, info.TypeIndex, &type);
		return hr;
	}

	//void Uninit()
	//{
	//}

	//~impl()
	//{
	//	Uninit();
	//}
};

//-----------------------------------------------------------------
CDbgSymbol::CDbgSymbol(std::shared_ptr<CDbgEngine> engine, const SYMBOL_INFO* info) : m_pimpl(new impl)
{
	HRESULT hr = m_pimpl->Init(engine, info);
	if (FAILED(hr))
		throw hr;
}

CDbgSymbol::~CDbgSymbol(void)
{
}

//-----------------------------------------------------------------
struct DbgUserContext {
	std::shared_ptr<CDbgEngine> engine;
	CDbgSymbol::List children;
	DbgUserContext(std::shared_ptr<CDbgEngine> _engine) : engine(_engine)
	{ }
};

static BOOL CALLBACK SymEnumSymbolCallback(
	PSYMBOL_INFO symbol_info, ULONG /*symbol_size*/, PVOID user_context) {
	DbgUserContext *data = static_cast<DbgUserContext *>(user_context);
	std::shared_ptr<CDbgSymbol> child;
	HRESULT hr = CreateInsatnce(data->engine, symbol_info, &child);
	assert( SUCCEEDED(hr) );
	if (FAILED(hr))
		return FALSE;
	data->children.push_back(child);
	return TRUE;
}

CDbgSymbol::List CDbgSymbol::GetModuleSymbols(std::shared_ptr<CDbgEngine> engine, DWORD64 modbase)
{
	DbgUserContext tmpdata(engine);
	if (!engine->SymEnumSymbols(modbase, "", &SymEnumSymbolCallback, &tmpdata)) {
		assert(!"Failed to call SymEnumSymbols.");
	}
	return tmpdata.children;
}

CDbgSymbol::List CDbgSymbol::GetChildSymbols()
{
	if (!CDbgEngine::IsScope(GetTag())) {
		return List();
	}

	// ローカルシンボルの基点となるアドレスなどを設定します。
	IMAGEHLP_STACK_FRAME stackframe;
	memset(&stackframe, 0, sizeof(stackframe));
	stackframe.InstructionOffset = GetAdress();
	if (!m_pimpl->engine->SymSetContext(&stackframe, NULL)) {
		//assert(!"Failed to call SymSetContext.");
		return List();
	}

	// ローカルシンボルを収集します。
	DbgUserContext tmpdata(m_pimpl->engine);
	if (!m_pimpl->engine->SymEnumSymbols(0, "", &SymEnumSymbolCallback, &tmpdata)) {
		//assert(!"Failed to call SymEnumSymbols.");
	}

	return tmpdata.children;
}

//-----------------------------------------------------------------
/// デバッグエンジンを取得します。
std::shared_ptr<CDbgEngine> CDbgSymbol::GetEngine() const
{
	return m_pimpl->engine;
}

ULONG CDbgSymbol::GetIndex() const
{
	return m_pimpl->info.Index;
}

enum SymTagEnum CDbgSymbol::GetTag() const
{
	return static_cast<enum SymTagEnum>(m_pimpl->info.Tag);
}

LPCTSTR CDbgSymbol::GetName() const
{
	return m_pimpl->name.c_str();
}

ULONG CDbgSymbol::GetSize() const
{
	return m_pimpl->info.Size;
}

ULONG CDbgSymbol::GetSymFlags() const
{
	return m_pimpl->info.Flags;
}

ULONG64 CDbgSymbol::GetAdress() const
{
	return m_pimpl->info.Address;
}

ULONG64 CDbgSymbol::GetModBase() const
{
	return m_pimpl->info.ModBase;
}

CV_HREG_e CDbgSymbol::GetRegister() const
{
	return static_cast<CV_HREG_e>(m_pimpl->info.Register);
}

std::shared_ptr<CDbgType> CDbgSymbol::GetType() const
{
	return m_pimpl->type;
}

