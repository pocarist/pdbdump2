#pragma once

#include <xmllite.h>
#include <shlwapi.h> 
#include <comdef.h>	//use _com_ptr_t

//-----------------------------------------------------------------
//テスト用。単にCreateXmlWriter()を呼んでるだけ。
HRESULT CreateWriter(IXmlWriter** ppWriter);

//-----------------------------------------------------------------
//テスト用。単にSHCreateStreamOnFile()を呼んでるだけ。
HRESULT CreateStreamFile(
	_In_   LPCTSTR pszFile,
	_In_   DWORD grfMode,
	_Out_  IStream **ppstm);

//-----------------------------------------------------------------
// IXmlWriterのAPIのうちPrintf形式のFormatで文字列を生成できるようにしたもの。
class CXmlWriter
{
	_com_ptr_t<_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)> > m_pWriter;
	HRESULT m_hr;

	inline HRESULT _SetError(HRESULT hr) { return m_hr = hr; }
#ifdef _DEBUG
#define SetError(expr) assert(SUCCEEDED(_SetError(expr)))
#else
#define SetError(expr) _SetError(expr)
#endif // DEBUG
public:
	CXmlWriter(IXmlWriter* ptr) : m_pWriter(ptr), m_hr(S_OK)
	{ }

	//printf形式で文字列データを作る
	static std::vector<wchar_t> FormatV(LPCWSTR lpszFormat, va_list args);
	static std::vector<wchar_t> Format(LPCWSTR lpszFormat, ...);

	IXmlWriter* GetWriter();
	HRESULT GetError();

	//-----------------------------------------------------------------
	HRESULT SetOutput( _In_opt_  IUnknown *pOutput);
	HRESULT GetProperty( _In_  UINT nProperty, _Out_  LONG_PTR *ppValue);
	HRESULT SetProperty( _In_  UINT nProperty, _In_opt_  LONG_PTR pValue);
	HRESULT WriteAttributes( _In_  IXmlReader *pReader, _In_  BOOL fWriteDefaultAttributes);
	HRESULT WriteAttributeString( _In_opt_  LPCWSTR pwszPrefix, _In_opt_  LPCWSTR pwszLocalName, _In_opt_  LPCWSTR pwszNamespaceUri, _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteCData( _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteCharEntity( _In_  WCHAR wch);
	HRESULT WriteChars( _In_reads_opt_(cwch)  const WCHAR *pwch, _In_  UINT cwch);
	HRESULT WriteComment( _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteDocType( _In_opt_  LPCWSTR pwszName, _In_opt_  LPCWSTR pwszPublicId, _In_opt_  LPCWSTR pwszSystemId, _In_opt_  LPCWSTR pwszSubset);
	HRESULT WriteElementString( _In_opt_  LPCWSTR pwszPrefix, _In_  LPCWSTR pwszLocalName, _In_opt_  LPCWSTR pwszNamespaceUri, _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteEndDocument( void);
	HRESULT WriteEndElement( void);
	HRESULT WriteEntityRef( _In_  LPCWSTR pwszFormat, ...);
	HRESULT WriteFullEndElement( void);
	HRESULT WriteName( _In_  LPCWSTR pwszFormat, ...);
	HRESULT WriteNmToken( _In_  LPCWSTR pwszFormat, ...);
	HRESULT WriteNode( _In_  IXmlReader *pReader, _In_  BOOL fWriteDefaultAttributes);
	HRESULT WriteNodeShallow( _In_  IXmlReader *pReader, _In_  BOOL fWriteDefaultAttributes);
	HRESULT WriteProcessingInstruction( _In_  LPCWSTR pwszName, _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteQualifiedName( _In_  LPCWSTR pwszLocalName, _In_opt_  LPCWSTR pwszNamespaceUri);
	HRESULT WriteRaw( _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteRawChars( _In_reads_opt_(cwch)  const WCHAR *pwch, _In_  UINT cwch);
	HRESULT WriteStartDocument( _In_  XmlStandalone standalone);
	HRESULT WriteStartElement(	_In_opt_  LPCWSTR pwszPrefix, _In_  LPCWSTR pwszLocalName, _In_opt_  LPCWSTR pwszNamespaceUri);
	HRESULT WriteString( _In_opt_  LPCWSTR pwszFormat, ...);
	HRESULT WriteSurrogateCharEntity( _In_  WCHAR wchLow, _In_  WCHAR wchHigh);
	HRESULT WriteWhitespace( _In_opt_  LPCWSTR pwszWhitespace);
	HRESULT Flush( void);

};
