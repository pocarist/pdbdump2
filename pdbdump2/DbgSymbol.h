#pragma once

class CDbgEngine;
class CDbgType;
class CDbgSymbol
{
	struct impl;
	std::unique_ptr<impl> m_pimpl;

public:
	explicit CDbgSymbol(std::shared_ptr<CDbgEngine> engine, const SYMBOL_INFO* info);
	virtual ~CDbgSymbol(void);

public:
	typedef std::list<std::shared_ptr<CDbgSymbol> > List;

	static List GetModuleSymbols(std::shared_ptr<CDbgEngine> engine, DWORD64 modbase);
	List GetChildSymbols();

	/// デバッグエンジンを取得します。
	std::shared_ptr<CDbgEngine> GetEngine() const;
	ULONG GetIndex() const;
	enum SymTagEnum GetTag() const;
	LPCTSTR GetName() const;
	ULONG GetSize() const;
	ULONG GetSymFlags() const;
	ULONG64 GetAdress() const;
	ULONG64 GetModBase() const;
	CV_HREG_e GetRegister() const;
	std::shared_ptr<CDbgType> GetType() const;
};
