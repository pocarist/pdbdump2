#include "stdafx.h"

#include <initguid.h>
#include "XmlLiteHelper.h"

#pragma comment(lib, "xmllite.lib")	//for CreateXmlWriter
#pragma comment(lib, "shlwapi.lib")	//for SHCreateStreamOnFile

#include <vector>

//-----------------------------------------------------------------
HRESULT CreateWriter(IXmlWriter** ppWriter)
{
	return ::CreateXmlWriter(IID_IXmlWriter, (void**)ppWriter, NULL);
}

//-----------------------------------------------------------------
HRESULT CreateStreamFile(
	_In_   LPCTSTR pszFile,
	_In_   DWORD grfMode,
	_Out_  IStream **ppstm)
{
	return ::SHCreateStreamOnFile(pszFile, grfMode, ppstm);
}

//-----------------------------------------------------------------
//printf形式で文字列データを作る
std::vector<wchar_t> CXmlWriter::FormatV(LPCWSTR lpszFormat, va_list args)
{
	int len = _vscwprintf(lpszFormat, args);
	std::vector<wchar_t> buf(len+1);
	len = vswprintf_s( &buf[0], len+1, lpszFormat, args );
	return buf;
}

std::vector<wchar_t> CXmlWriter::Format(LPCWSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	std::vector<wchar_t> buf = FormatV(lpszFormat, args);
	va_end(args);
	return buf;
}

IXmlWriter* CXmlWriter::GetWriter() { return m_pWriter; }
HRESULT CXmlWriter::GetError() { return m_hr; }

//-----------------------------------------------------------------
HRESULT CXmlWriter::SetOutput( _In_opt_  IUnknown *pOutput)
{
	SetError( m_pWriter->SetOutput(pOutput) );
	return GetError();
}
	
HRESULT CXmlWriter::GetProperty( 
	_In_  UINT nProperty,
	_Out_  LONG_PTR *ppValue)
{
	SetError( m_pWriter->GetProperty(nProperty, ppValue) );
	return GetError();
}
	
HRESULT CXmlWriter::SetProperty( 
	_In_  UINT nProperty,
	_In_opt_  LONG_PTR pValue)
{
	SetError( m_pWriter->SetProperty(nProperty, pValue) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteAttributes( 
	_In_  IXmlReader *pReader,
	_In_  BOOL fWriteDefaultAttributes)
{
	SetError( m_pWriter->WriteAttributes(pReader, fWriteDefaultAttributes) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteAttributeString( 
	_In_opt_  LPCWSTR pwszPrefix,
	_In_opt_  LPCWSTR pwszLocalName,
	_In_opt_  LPCWSTR pwszNamespaceUri,
	_In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteAttributeString(pwszPrefix, pwszLocalName, pwszNamespaceUri, &buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteCData( _In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteCData(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteCharEntity( _In_  WCHAR wch)
{
	SetError( m_pWriter->WriteCharEntity(wch) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteChars( _In_reads_opt_(cwch)  const WCHAR *pwch, _In_  UINT cwch)
{
	SetError( m_pWriter->WriteChars(pwch, cwch) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteComment( _In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteComment(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteDocType( 
	_In_opt_  LPCWSTR pwszName,
	_In_opt_  LPCWSTR pwszPublicId,
	_In_opt_  LPCWSTR pwszSystemId,
	_In_opt_  LPCWSTR pwszSubset)
{
	SetError( m_pWriter->WriteDocType(pwszName, pwszPublicId, pwszSystemId, pwszSubset) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteElementString( 
	_In_opt_  LPCWSTR pwszPrefix,
	_In_  LPCWSTR pwszLocalName,
	_In_opt_  LPCWSTR pwszNamespaceUri,
	_In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteElementString(pwszPrefix, pwszLocalName, pwszNamespaceUri, &buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteEndDocument( void)
{
	SetError( m_pWriter->WriteEndDocument() );
	return GetError();
}
	
HRESULT CXmlWriter::WriteEndElement( void)
{
	SetError( m_pWriter->WriteEndElement() );
	return GetError();
}
	
HRESULT CXmlWriter::WriteEntityRef( _In_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteEntityRef(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteFullEndElement( void)
{
	SetError( m_pWriter->WriteFullEndElement() );
	return GetError();
}
	
HRESULT CXmlWriter::WriteName( _In_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteName(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteNmToken( _In_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteNmToken(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteNode( _In_  IXmlReader *pReader, _In_  BOOL fWriteDefaultAttributes)
{
	SetError( m_pWriter->WriteNode(pReader, fWriteDefaultAttributes) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteNodeShallow( _In_  IXmlReader *pReader, _In_  BOOL fWriteDefaultAttributes)
{
	SetError( m_pWriter->WriteNodeShallow(pReader, fWriteDefaultAttributes) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteProcessingInstruction( _In_  LPCWSTR pwszName, _In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteProcessingInstruction(pwszName, &buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteQualifiedName( _In_  LPCWSTR pwszLocalName, _In_opt_  LPCWSTR pwszNamespaceUri)
{
	SetError( m_pWriter->WriteQualifiedName(pwszLocalName, pwszNamespaceUri) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteRaw( _In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteRaw(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteRawChars( _In_reads_opt_(cwch)  const WCHAR *pwch, _In_  UINT cwch)
{
	SetError( m_pWriter->WriteRawChars(pwch, cwch) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteStartDocument( _In_  XmlStandalone standalone)
{
	SetError( m_pWriter->WriteStartDocument(standalone) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteStartElement(
	_In_opt_  LPCWSTR pwszPrefix,
	_In_  LPCWSTR pwszLocalName,
	_In_opt_  LPCWSTR pwszNamespaceUri)
{
	SetError( m_pWriter->WriteStartElement(pwszPrefix, pwszLocalName, pwszNamespaceUri) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteString( _In_opt_  LPCWSTR pwszFormat, ...)
{
	va_list args;
	va_start(args, pwszFormat);
	std::vector<wchar_t> buf = FormatV(pwszFormat, args);
	va_end(args);
	SetError( m_pWriter->WriteString(&buf[0]) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteSurrogateCharEntity( _In_  WCHAR wchLow, _In_  WCHAR wchHigh)
{
	SetError( m_pWriter->WriteSurrogateCharEntity(wchLow, wchHigh) );
	return GetError();
}
	
HRESULT CXmlWriter::WriteWhitespace( _In_opt_  LPCWSTR pwszWhitespace)
{
	SetError( m_pWriter->WriteWhitespace(pwszWhitespace) );
	return GetError();
}
	
HRESULT CXmlWriter::Flush( void)
{
	SetError( m_pWriter->Flush() );
	return GetError();
}
