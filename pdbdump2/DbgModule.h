#pragma once

class CDbgEngine;
class CDbgModule
{
	struct impl;
	std::unique_ptr<impl> m_pimpl;

public:
	explicit CDbgModule(std::shared_ptr<CDbgEngine> engine, LPCTSTR modname);
	virtual ~CDbgModule(void);

	/// モジュールのヘッダを取得します。
	IMAGE_NT_HEADERS *GetHeader();

	/// このモジュールの情報を取得します。
	IMAGEHLP_MODULE64 *GetModuleInfo();

	/// モジュールのベースアドレスを取得します。
	DWORD64 GetImageBase();

	/// セクションを取得します。
	IMAGE_SECTION_HEADER *GetSections();

	/// セクションの数を取得します。
	DWORD GetSectionNum();
};

