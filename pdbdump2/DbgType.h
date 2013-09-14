#pragma once

class CDbgEngine;
class CDbgType
{
	struct impl;
	std::unique_ptr<impl> m_pimpl;

public:
	explicit CDbgType(std::shared_ptr<CDbgEngine> engine, ULONG64 modbasep, ULONG typeindex);
	virtual ~CDbgType(void);

	//typedef std::list<std::shared_ptr<CDbgType> > List;
	typedef std::vector<std::shared_ptr<CDbgType> > List;

	/// デバッグエンジンを取得します。
	std::shared_ptr<CDbgEngine> GetEngine();

	/// 型のインデックスを取得します。
	ULONG GetTypeIndex();

	/// タグを取得します。
	enum SymTagEnum GetTypeTag();

	/// モジュールに定義された型をすべて収集します。
	static List GetModuleTypes(std::shared_ptr<CDbgEngine> engine, DWORD64 modbase);

	/// 関連付けれた'次'の型を取得します。
	HRESULT GetNextType(std::shared_ptr<CDbgType>* retval);

	/// もしあれば、関連付けられた子供の型を取得します。
	HRESULT GetChildTypes(CDbgType::List* retval);

	/// もしあればシンボルの名前を取得します。
	HRESULT GetSymName(std::wstring* retval);

	/// もしあれば親クラスを取得します。
	HRESULT GetClassParentType(std::shared_ptr<CDbgType>* retval);

	/// 型のサイズを取得します。
	HRESULT GetLength(ULONG64* retval);

	/// Tag==SymTagBase の場合にその基本型を取得します。
	HRESULT GetBaseType(BasicType* retval);

	/// オフセットアドレスを取得します。
	HRESULT GetAddressOffset(ULONG* retval);

	/// アドレスを取得します。
	HRESULT GetAddress(ULONG64* retval);

	/// オフセットを取得します。
	HRESULT GetOffset(ULONG* retval);

	/// 配列のインデックス型を取得します。
	HRESULT GetArrayIndexType(std::shared_ptr<CDbgType>* retval);

	/// 配列の要素数を取得します。
	HRESULT GetArrayCount(ULONG* retval);

	/// バーチャルベースクラスがあれば、それを取得します。
	HRESULT GetVirtualBaseClassType(std::shared_ptr<CDbgType>* retval);

	/// 仮想テーブルの型を取得します。
	HRESULT GetVirtualTableShapeType(std::shared_ptr<CDbgType>* retval);

	HRESULT GetVirtualBasePointerOffset(ULONG* retval);

	HRESULT GetVirtualBaseOffset(ULONG* retval);

	/// thisポインターからのオフセットを取得します。
	HRESULT GetThisAdjust(ULONG* retval);

	/// UDT(UserDataKind)の種類を取得します。
	HRESULT GetUdtKind(UdtKind* retval);

	/// Dataの種類を取得します。
	HRESULT GetDataKind(DataKind* retval);

	/// 関数の呼び出し規約を取得します。
	HRESULT GetCallingConversion(CV_call_e* retval);
};
