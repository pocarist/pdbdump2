#include "stdafx.h"
#include "DbgType.h"
#include "DbgEngine.h"


//-----------------------------------------------------------------
struct CDbgType::impl {
	std::shared_ptr<CDbgEngine> engine;
	ULONG typeindex;
	ULONG64 modbase;
	enum SymTagEnum tag;

	std::shared_ptr<CDbgType> next_type_cache;
	std::shared_ptr<CDbgType::List> children_cache;
	std::shared_ptr<std::wstring> symname_cache;

	HRESULT Init(std::shared_ptr<CDbgEngine> _engine, ULONG64 _modbase, ULONG _typeindex)
	{
		engine = _engine;
		modbase = _modbase;
		typeindex = _typeindex;
		tag = GetTypeTag();
		return S_OK;
	}

	enum SymTagEnum GetTypeTag()
	{
		ULONG tag = 0;

		if (typeindex == 0) {
			return SymTagNull;
		}

		// 型のタグを取得します。
		if (!engine->SymGetTypeInfo(
			modbase, typeindex, TI_GET_SYMTAG, &tag)) {
			//assert(!"Couldn't get the type tag!");
			return SymTagNull;
		}

		return (enum SymTagEnum)tag;
	}

	HRESULT SymGetTypeInfo(IMAGEHLP_SYMBOL_TYPE_INFO GetType, void* pv)
	{
		if (!engine->SymGetTypeInfo(modbase, typeindex, GetType, pv)) {
			return E_NOINTERFACE;
		}

		return S_OK;
	}

	template <class T>
	HRESULT GetTypeInfo(IMAGEHLP_SYMBOL_TYPE_INFO GetType, T& value)
	{
		return SymGetTypeInfo(GetType, &value);
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
CDbgType::CDbgType(std::shared_ptr<CDbgEngine> engine, ULONG64 modbasep, ULONG typeindex) : m_pimpl(new impl)
{
	HRESULT hr = m_pimpl->Init(engine, modbasep, typeindex);
	if (FAILED(hr))
		throw hr;
}

CDbgType::~CDbgType(void)
{
}

//-----------------------------------------------------------------
/// デバッグエンジンを取得します。
std::shared_ptr<CDbgEngine> CDbgType::GetEngine()
{
	return m_pimpl->engine;
}

/// 型のインデックスを取得します。
ULONG CDbgType::GetTypeIndex()
{
	return m_pimpl->typeindex;
}

/// タグを取得します。
enum SymTagEnum CDbgType::GetTypeTag()
{
	return m_pimpl->tag;
}


//-----------------------------------------------------------------
struct SymEnumTypeData {
	std::shared_ptr<CDbgEngine> engine;
	CDbgType::List typelist;
	SymEnumTypeData(std::shared_ptr<CDbgEngine> _engine) : engine(_engine)
	{ }
};

/// 型オブジェクトを収集します。
static BOOL CALLBACK SymEnumTypeCallback(
	PSYMBOL_INFO symbol_info, ULONG /*symbol_size*/, PVOID user_context) {
	// 新しい型オブジェクトを追加します。
	SymEnumTypeData *data = static_cast<SymEnumTypeData *>(user_context);
	std::shared_ptr<CDbgType> child;
	HRESULT hr = CreateInsatnce(data->engine, symbol_info->ModBase, symbol_info->TypeIndex, &child);
	assert( SUCCEEDED(hr) );
	if (FAILED(hr))
		return FALSE;
	data->typelist.push_back(child);
	return TRUE;
}

/// モジュールに定義された型をすべて収集します。
CDbgType::List CDbgType::GetModuleTypes(std::shared_ptr<CDbgEngine> engine, DWORD64 modbase)
{
	SymEnumTypeData data(engine);
	if (!engine->SymEnumTypes(modbase, SymEnumTypeCallback, &data)) {
		//assert(!"Failed to call SymEnumTypes");
		return List();
	}
	return data.typelist;
}

/// 関連付けれた'次'の型を取得します。
HRESULT CDbgType::GetNextType(std::shared_ptr<CDbgType>* retval)
{
	if (m_pimpl->next_type_cache != NULL) {
		*retval = m_pimpl->next_type_cache;
		return S_OK;
	}

	// Get the next type down the chain and decode that one.
	ULONG index = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_TYPEID, index);
	if (FAILED(hr))
		return hr;

	std::shared_ptr<CDbgType> next_type;
	hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, index, &next_type);
	if (FAILED(hr))
		return hr;
	m_pimpl->next_type_cache = next_type;
	*retval = next_type;
	return S_OK;
}

#define CHILDREN_MAX 512

/**
 * @brief 子タイプを列挙するときに使います。
 */
struct FINDCHILDREN : public TI_FINDCHILDREN_PARAMS {
	FINDCHILDREN() {
		memset(this, 0, sizeof(TI_FINDCHILDREN_PARAMS));
		this->Count = CHILDREN_MAX;
		this->Start = 0;
	}

private:
	ULONG children_id[CHILDREN_MAX];
};

/// もしあれば、関連付けられた子供の型を取得します。
HRESULT CDbgType::GetChildTypes(CDbgType::List* retval)
{
	if (m_pimpl->children_cache != nullptr) {
		*retval = *m_pimpl->children_cache;
		return S_OK;
	}

	// 子供の数を取得します。
	ULONG children_count = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_CHILDRENCOUNT, children_count);
	if (FAILED(hr))
		return hr;

	if (children_count == 0) {
		return E_NOT_SET;
	}

	// 子タイプを収集し、コンテナに収めます。
	TI_FINDCHILDREN_PARAMS *find = (TI_FINDCHILDREN_PARAMS *)HeapAlloc(
		GetProcessHeap(), 0, sizeof(TI_FINDCHILDREN_PARAMS) + sizeof(ULONG) * children_count);
	if (find == NULL) {
		return E_OUTOFMEMORY;
	}
	find->Count = children_count;
	find->Start = 0;

	hr = m_pimpl->SymGetTypeInfo(TI_FINDCHILDREN, find);
	if (FAILED(hr)) {
		HeapFree(GetProcessHeap(), 0, find);
		return hr;
	}

	List result;
	for (ULONG i = 0; i < find->Count; ++i) {
		std::shared_ptr<CDbgType> child;
		hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, find->ChildId[i], &child);
		if (FAILED(hr))
			break;
		result.push_back(child);
	}

	HeapFree(GetProcessHeap(), 0, find);
//	m_pimpl->children_cache = std::make_shared<List>(result);
	*retval = result;
	return S_OK;
}

/// もしあればシンボルの名前を取得します。
HRESULT CDbgType::GetSymName(std::wstring* retval)
{
	if (m_pimpl->symname_cache != nullptr) {
		*retval = *m_pimpl->symname_cache;
		return S_OK;
	}

	WCHAR *wname = NULL;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_SYMNAME, wname);
	if (FAILED(hr)) {
		return hr;
	}

	m_pimpl->symname_cache = std::make_shared<std::wstring>(wname);
	LocalFree(wname);
	*retval = *m_pimpl->symname_cache;
	return S_OK;
}

/// もしあれば親クラスを取得します。
HRESULT CDbgType::GetClassParentType(std::shared_ptr<CDbgType>* retval)
{
	ULONG index = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_CLASSPARENTID, index);
	if (FAILED(hr)) {
		return hr;
	}
	std::shared_ptr<CDbgType> type;
	hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, index, &type);
	if (FAILED(hr)) {
		return hr;
	}
	*retval = type;
	return S_OK;
}

/// 型のサイズを取得します。
HRESULT CDbgType::GetLength(ULONG64* retval)
{
	ULONG64 value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_LENGTH, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// Tag==SymTagBase の場合にその基本型を取得します。
HRESULT CDbgType::GetBaseType(BasicType* retval)
{
	ULONG value = btNoType;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_BASETYPE, value);
	if (FAILED(hr))
		return hr;
	*retval = (BasicType)value;
	return S_OK;
}

/// オフセットアドレスを取得します。
HRESULT CDbgType::GetAddressOffset(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_ADDRESSOFFSET, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// アドレスを取得します。
HRESULT CDbgType::GetAddress(ULONG64* retval)
{
	ULONG64 value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_ADDRESS, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// オフセットを取得します。
HRESULT CDbgType::GetOffset(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_OFFSET, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// 配列のインデックス型を取得します。
HRESULT CDbgType::GetArrayIndexType(std::shared_ptr<CDbgType>* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_ARRAYINDEXTYPEID, value);
	if (FAILED(hr))
		return hr;
	std::shared_ptr<CDbgType> type;
	hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, value, &type);
	if (FAILED(hr)) {
		return hr;
	}
	*retval = type;
	return S_OK;
}

/// 配列の要素数を取得します。
HRESULT CDbgType::GetArrayCount(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_COUNT, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// バーチャルベースクラスがあれば、それを取得します。
HRESULT CDbgType::GetVirtualBaseClassType(std::shared_ptr<CDbgType>* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_VIRTUALBASECLASS, value);
	if (FAILED(hr))
		return hr;
	std::shared_ptr<CDbgType> type;
	hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, value, &type);
	if (FAILED(hr)) {
		return hr;
	}
	*retval = type;
	return S_OK;
}

/// 仮想テーブルの型を取得します。
HRESULT CDbgType::GetVirtualTableShapeType(std::shared_ptr<CDbgType>* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_VIRTUALTABLESHAPEID, value);
	if (FAILED(hr))
		return hr;
	std::shared_ptr<CDbgType> type;
	hr = CreateInsatnce(m_pimpl->engine, m_pimpl->modbase, value, &type);
	if (FAILED(hr)) {
		return hr;
	}
	*retval = type;
	return S_OK;
}

HRESULT CDbgType::GetVirtualBasePointerOffset(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_VIRTUALBASEPOINTEROFFSET, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

HRESULT CDbgType::GetVirtualBaseOffset(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_VIRTUALBASEOFFSET, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// thisポインターからのオフセットを取得します。
HRESULT CDbgType::GetThisAdjust(ULONG* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_THISADJUST, value);
	if (FAILED(hr))
		return hr;
	*retval = value;
	return S_OK;
}

/// UDT(UserDataKind)の種類を取得します。
HRESULT CDbgType::GetUdtKind(UdtKind* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_UDTKIND, value);
	if (FAILED(hr))
		return hr;
	*retval = (UdtKind)value;
	return S_OK;
}

/// Dataの種類を取得します。
HRESULT CDbgType::GetDataKind(DataKind* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_DATAKIND, value);
	if (FAILED(hr))
		return hr;
	*retval = (DataKind)value;
	return S_OK;
}

/// 関数の呼び出し規約を取得します。
HRESULT CDbgType::GetCallingConversion(CV_call_e* retval)
{
	ULONG value = 0;
	HRESULT hr = m_pimpl->GetTypeInfo(TI_GET_CALLING_CONVENTION, value);
	if (FAILED(hr))
		return hr;
	*retval = (CV_call_e)value;
	return S_OK;
}

