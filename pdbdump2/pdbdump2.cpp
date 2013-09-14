// pdbdump2.cpp : コンソール アプリケーションのエントリ ポイントを定義します。
//

#include "stdafx.h"

#include "DbgEngine.h"
#include "DbgModule.h"
#include "DbgSymbol.h"
#include "DbgType.h"

#include <comdef.h>	//use _com_ptr_t
#include "XmlLiteHelper.h"

#include <stdarg.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <set>
#include <locale>

using namespace std;

#include <time.h>

template<class Cont, class Pred>
static void divide(Cont &container1, Cont &container2, Pred pred) {
	Cont::iterator it = container1.begin();

	while (it != container1.end()) {
		if (pred(*it)) {
			container2.push_back(*it);
			container1.erase(it++);
		}
		else {
			++it;
		}
	}
}

struct SymFlagsComparator {
	explicit SymFlagsComparator(DWORD flag)
		: flag_(flag) {
	}
	bool operator()(std::shared_ptr<CDbgSymbol> symbol) {
		return ((symbol->GetSymFlags() & this->flag_) != 0);
	}
private:
	DWORD flag_;
};

class CDumpXml {
public:
	CXmlWriter xml;
	CDumpXml(IXmlWriter* pWriter) : xml(pWriter)
	{ }

	HRESULT MakeChildSymbolNode(LPCWSTR nodename, CDbgSymbol::List &children)
	{
		xml.WriteStartElement(NULL, nodename, NULL);
		if (FAILED(xml.GetError()))
			return xml.GetError();

		for (auto& i : children) {
			HRESULT hr = MakeChildSymbolNode(i);
			if (FAILED(hr))
				return hr;
		}

		xml.WriteEndElement();
		return xml.GetError();
	}

	HRESULT MakeChildSymbolNode(std::shared_ptr<CDbgSymbol> symbol)
	{
	//	TiXmlElement *elem = new TiXmlElement("symbol");
		xml.WriteStartElement(NULL, L"symbol", NULL);
		if (FAILED(xml.GetError()))
			return xml.GetError();

		//elem->SetAttribute("index", symbol->GetIndex());
		//elem->SetAttribute("name", symbol->GetName());
		//elem->SetAttribute("tag", Dbg_GetTagName(symbol->GetTag()));
		xml.WriteAttributeString(NULL, L"index", NULL, _T("%d"), symbol->GetIndex());
		xml.WriteAttributeString(NULL, L"name", NULL, _T("%s"), symbol->GetName());
		xml.WriteAttributeString(NULL, L"tag", NULL, _T("%s"), CDbgEngine::GetTagName(symbol->GetTag()));
		if (symbol->GetType() != NULL) {
			//elem->SetAttribute("typeindex", symbol->GetType()->GetTypeIndex());
			xml.WriteAttributeString(NULL, L"typeindex", NULL, _T("%d"), symbol->GetType()->GetTypeIndex());
		}
		//elem->SetAttribute("size", symbol->GetSize());
		xml.WriteAttributeString(NULL, L"size", NULL, _T("%d"), symbol->GetSize());
		//hr = pWriter->WriteAttributeString(NULL, L"size", NULL, std::to_wstring(symbol->GetSize()).c_str());
		//if (FAILED(hr))
		//	return hr;

		// ローカル変数などはレジスタからのオフセットで表現されます。
		if (symbol->GetRegister() == CV_REG_NONE) {
			//TCHAR buffer[128];
			//snprintf(buffer, sizeof(buffer), L"0x%08x", (DWORD)symbol->GetAdress());
			//elem->SetAttribute("address", buffer);
			xml.WriteAttributeString(NULL, L"address", NULL, _T("0x%08x"), (DWORD)symbol->GetAdress());

			// アドレスが存在するなら、アドレスから型が定義された位置を特定します。
			IMAGEHLP_LINE64 line = {sizeof(IMAGEHLP_LINE64)};
			DWORD dwLineDisp;
			if (symbol->GetEngine()->SymGetLineFromAddr64(symbol->GetAdress(), &dwLineDisp, &line)) {
				//elem->SetAttribute("filename", line.FileName);
				//elem->SetAttribute("lineno", line.LineNumber);
				xml.WriteAttributeString(NULL, L"filename", NULL, to_wstring(line.FileName).c_str());
				xml.WriteAttributeString(NULL, L"lineno", NULL, _T("%d"), line.LineNumber);
			}
		}
		else {
			//char buffer[32];
			//snprintf(buffer, sizeof(buffer), "%s(%d)", Dbg_GetRegName(symbol->GetRegister()), (int)symbol->GetAdress());
			//elem->SetAttribute("register", buffer);
			xml.WriteAttributeString(NULL, L"register", NULL, _T("%s(%d)"), CDbgEngine::GetRegName(symbol->GetRegister()), (int)symbol->GetAdress());

		}

		HRESULT hr = S_OK;
		CDbgSymbol::List children;
		children = symbol->GetChildSymbols();
		if (!children.empty()) {
			CDbgSymbol::List params;
			divide(children, params, SymFlagsComparator(SYMFLAG_PARAMETER));
			if (!params.empty()) {
				//elem->LinkEndChild(MakeChildSymbolNode("parameters", params));
				hr = MakeChildSymbolNode(_T("parameters"), params);
				if (FAILED(hr))
					return hr;
			}

			CDbgSymbol::List locals;
			divide(children, locals, SymFlagsComparator(SYMFLAG_LOCAL));
			if (!locals.empty()) {
				//elem->LinkEndChild(MakeChildSymbolNode("locals", locals));
				hr = MakeChildSymbolNode(_T("locals"), locals);
				if (FAILED(hr))
					return hr;
			}

			if (!children.empty()) {
				//elem->LinkEndChild(MakeChildSymbolNode("children", children));
				hr = MakeChildSymbolNode(_T("children"), children);
				if (FAILED(hr))
					return hr;
			}
		}

		xml.WriteEndElement();
		return xml.GetError();
	}

	HRESULT MakeModuleNode(std::shared_ptr<CDbgModule> module)
	{
		//TiXmlElement *elem = new TiXmlElement("module");
		xml.WriteStartElement(NULL, L"module", NULL);

		IMAGEHLP_MODULE64 *module_info = module->GetModuleInfo();
		//elem->SetAttribute("image-name", module_info->LoadedImageName);
		xml.WriteAttributeString(NULL, L"image-name", NULL, _T("%s"), to_wstring(module_info->LoadedImageName).c_str());

		TCHAR buffer[128];
		time_t t = module_info->TimeDateStamp;
		//char *timestr = ctime(&t);
		_tctime_s(buffer, 128, &t);
		TCHAR* timestr = buffer;
		if (timestr != NULL) {
			// なぜか最後の文字が改行コードになっているので、それを潰します。
			std::wstring timestring(timestr, _tcslen(timestr) - 1);
			//elem->SetAttribute("time-stamp", timestring);
			xml.WriteAttributeString(NULL, L"time-stamp", NULL, _T("%s"), timestring.c_str());
		}

		if (module_info->LoadedPdbName[0] != '\0') {
			//elem->SetAttribute("pdb-name", module_info->LoadedPdbName);
			xml.WriteAttributeString(NULL, L"pdb-name", NULL, _T("%s"), to_wstring(module_info->LoadedPdbName).c_str());
		}

		//snprintf(buffer, sizeof(buffer), L"0x%08x", (DWORD)module_info->BaseOfImage);
		//elem->SetAttribute("base-address", buffer);
		xml.WriteAttributeString(NULL, L"base-address", NULL, L"0x%08x", (DWORD)module_info->BaseOfImage);

		//snprintf(buffer, sizeof(buffer), L"%d", module_info->ImageSize);
		//elem->SetAttribute("image-size", buffer);
		xml.WriteAttributeString(NULL, L"image-size", NULL, L"%d", module_info->ImageSize);

		//snprintf(buffer, sizeof(buffer), L"%d", module_info->CheckSum);
		//elem->SetAttribute("checksum", buffer);
		xml.WriteAttributeString(NULL, L"checksum", NULL, L"%d", module_info->CheckSum);
		//assert( SUCCEEDED(hr) );

		//elem->SetAttribute("has-publics", (module_info->Publics ? "true" : "false"));
		//elem->SetAttribute("has-global-symbols", (module_info->GlobalSymbols ? "true" : "false"));
		//elem->SetAttribute("has-line-numbers", (module_info->LineNumbers ? "true" : "false"));
		xml.WriteAttributeString(NULL, L"has-publics", NULL, module_info->Publics ? L"true" : L"false");
		xml.WriteAttributeString(NULL, L"has-global-symbols", NULL, module_info->GlobalSymbols ? L"true" : L"false");
		xml.WriteAttributeString(NULL, L"has-line-numbers", NULL, module_info->LineNumbers ? L"true" : L"false");

//		xml.WriteFullEndElement();
		return xml.GetError();
	}


	struct CompareSymbolWithAddress {
		bool operator()(shared_ptr<CDbgSymbol> lhs, shared_ptr<CDbgSymbol> rhs) {
			return (lhs->GetAdress() < rhs->GetAdress());
		}
	};

	struct SectionSelector {
		typedef shared_ptr<CDbgSymbol> argument_type;

		explicit SectionSelector(DWORD64 imagebase,
								 IMAGE_SECTION_HEADER *begin_section,
								 IMAGE_SECTION_HEADER *end_section)
			: imagebase_(imagebase), begin_section_(begin_section), end_section_(end_section) {
		}
		bool operator()(const shared_ptr<CDbgSymbol> &symbol) const {
			if (this->begin_section_ != NULL) {
				DWORD64 begin_addr = this->imagebase_ + this->begin_section_->VirtualAddress;
				if (symbol->GetAdress() < begin_addr) {
					return false;
				}
			}
			if (this->end_section_ != NULL) {
				DWORD64 end_addr = this->imagebase_ + this->end_section_->VirtualAddress;
				if (end_addr <= symbol->GetAdress()) {
					return false;
				}
			}
			return true;
		}
	private:
		DWORD64 imagebase_;
		IMAGE_SECTION_HEADER *begin_section_;
		IMAGE_SECTION_HEADER *end_section_;
	};

	HRESULT MakeSymbolNode(std::shared_ptr<CDbgModule> module, const CDbgSymbol::List& symbollist) {
		//TiXmlElement *elem = MakeModuleNode(module);
		HRESULT hr = MakeModuleNode(module);
		if (FAILED(hr))
			return hr;
		CDbgSymbol::List newlist(symbollist);

		// モジュールのトップレベルに定義されたオブジェクト群をアドレス値でソートします。
		newlist.sort(CompareSymbolWithAddress());

		// セクションごとにシンボルを出力していきます。
		IMAGE_SECTION_HEADER *sections = module->GetSections();
		DWORD section_num = module->GetSectionNum();
	
		for (DWORD i = 0; i < section_num; ++i) {
			CDbgSymbol::List tmplist = newlist;
			//char buffer[32];

			// 指定されたセクションに含まれるシンボルのみを抽出します。
			SectionSelector pred(
				module->GetImageBase(),
				&sections[i],
				(i + 1 >= section_num ? NULL : &sections[i + 1]));

			tmplist.erase(
				std::remove_if(tmplist.begin(), tmplist.end(), std::not1(pred)),
				tmplist.end());

			// セクション要素を作成します。
			//TiXmlElement *section_elem = new TiXmlElement("section");
			xml.WriteStartElement(NULL, L"section", NULL);
			//memset(buffer, 0, sizeof(sections[i].Name) + 1);
			//memcpy(buffer, sections[i].Name, sizeof(sections[i].Name));
			//section_elem->SetAttribute("name", buffer);
			xml.WriteAttributeString(NULL, L"name", NULL, to_wstring(std::string((char*)sections[i].Name, sizeof(sections[i].Name))).c_str());

			//snprintf(buffer, sizeof(buffer), "0x%08x", (DWORD)(module.GetImageBase() + sections[i].VirtualAddress));
			//section_elem->SetAttribute("address", buffer);
			xml.WriteAttributeString(NULL, L"address", NULL, L"0x%08x", (DWORD)(module->GetImageBase() + sections[i].VirtualAddress));
			//elem->LinkEndChild(section_elem);

			CDbgSymbol::List::iterator it;
			for (it = tmplist.begin(); it != tmplist.end(); ++it) {
				//TiXmlElement *node = MakeChildSymbolNode(*it);
				//section_elem->LinkEndChild(node);
				hr = MakeChildSymbolNode(*it);
				if (FAILED(hr))
					return hr;
			}
			xml.WriteEndElement();	//section
		}

		return hr;
	}

	/////////////////////////////////////////////////////////////////////
	/// 以前に表示された型を再度表示しないようにします。
	struct DbgTypeComparator {
		bool operator()(const shared_ptr<CDbgType> &x, const shared_ptr<CDbgType> &y) const {
			return (x->GetTypeIndex() < y->GetTypeIndex());
		}
	};
	typedef std::set<shared_ptr<CDbgType>, DbgTypeComparator> DbgTypeSet;

	std::wstring GetTypeTagName(shared_ptr<CDbgType> type) {
		UdtKind udtkind;

		if (type->GetTypeTag() == SymTagUDT && type->GetUdtKind(&udtkind) == S_OK) {
			//char buffer[128];
			//_snprintf(buffer, sizeof(buffer), "UDT(%s)", Dbg_GetUdtKindName(udtkind));
			TCHAR buffer[128];
			_sntprintf_s(buffer, sizeof(buffer), _T("UDT(%s)"), CDbgEngine::GetUdtKindName(udtkind));
			return std::wstring(&buffer[0]);
		}
		else {
			return CDbgEngine::GetTagName(type->GetTypeTag());
		}
	}

	HRESULT MakeXmlChildNode(shared_ptr<CDbgType> type, DbgTypeSet &typecache) {
		// すでに定義が出力されている場合は、表示をインデックスによる参照で済ませます。
		if (typecache.find(type) != typecache.end()) {
			//TiXmlElement *elem = new TiXmlElement("type-ref");
			//elem->SetAttribute("index", type->GetTypeIndex());
			//elem->SetAttribute("tag", GetTypeTagName(type));
			//return elem;
			xml.WriteStartElement(NULL, L"type-ref", NULL);
			xml.WriteAttributeString(NULL, L"index", NULL, _T("%d"), type->GetTypeIndex());
			xml.WriteAttributeString(NULL, L"tag", NULL, GetTypeTagName(type).c_str());
			xml.WriteEndElement();
			return xml.GetError();
		}
		typecache.insert(type);

		// 新たな型ノードを作成します。
		//TiXmlElement *elem = new TiXmlElement("type");
		//elem->SetAttribute("index", type->GetTypeIndex());
		//elem->SetAttribute("tag", GetTypeTagName(type));
		xml.WriteStartElement(NULL, L"type", NULL);
//		xml.WriteAttributeString(NULL, L"index", NULL, _T("%d"), type->GetTypeIndex());
		xml.WriteAttributeString(NULL, L"index", NULL, to_wstring(type->GetTypeIndex()).c_str());
		xml.WriteAttributeString(NULL, L"tag", NULL, GetTypeTagName(type).c_str());

		ULONG64 length = 0;
		ULONG64 *plength = NULL;
		if (type->GetLength(&length) == S_OK) {
			plength = &length;
		}

		std::wstring symname;
		if (type->GetSymName(&symname) == S_OK) {
			//elem->SetAttribute("name", symname);
			xml.WriteAttributeString(NULL, L"name", NULL, symname.c_str());
		}

		BasicType basic_type;
		if (type->GetBaseType(&basic_type) == S_OK) {
			//elem->SetAttribute("type", Dbg_GetBasicTypeName(basic_type, (ULONG)length));
			xml.WriteAttributeString(NULL, L"type", NULL, CDbgEngine::GetBasicTypeName(basic_type, (ULONG)length));
		}

		if (plength != NULL) {
			//elem->SetAttribute("size", (int)*plength);
			xml.WriteAttributeString(NULL, L"size", NULL, _T("%d"), (int)*plength);
		}

		DataKind datakind;
		if (type->GetDataKind(&datakind) == S_OK) {
			//elem->SetAttribute("datakind", Dbg_GetDataKindName(datakind));
			xml.WriteAttributeString(NULL, L"datakind", NULL, CDbgEngine::GetDataKindName(datakind));
		}

		ULONG uvalue;
		if (type->GetOffset(&uvalue) == S_OK) {
			//elem->SetAttribute("offset", uvalue);
			xml.WriteAttributeString(NULL, L"offset", NULL, _T("%d"), uvalue);
		}

		ULONG64 address;
		if (type->GetAddress(&address) == S_OK) {
			//char buffer[32];
			//_snprintf(buffer, sizeof(buffer), "%08x", address);
			//elem->SetAttribute("address", buffer);
			xml.WriteAttributeString(NULL, L"address", NULL, L"0x%08x", (DWORD)(address));

			// アドレスが存在するなら、アドレスから型が定義された位置を特定します。
			IMAGEHLP_LINE64 line = {sizeof(IMAGEHLP_LINE64)};
			DWORD dwLineDisp;
			if (type->GetEngine()->SymGetLineFromAddr64(address, &dwLineDisp, &line)) {
				//elem->SetAttribute("filename", line.FileName);
				//elem->SetAttribute("lineno", line.LineNumber);
				xml.WriteAttributeString(NULL, L"filename", NULL, to_wstring(line.FileName).c_str());
				xml.WriteAttributeString(NULL, L"lineno", NULL, _T("%d"), line.LineNumber);
			}
		}

		ULONG address_offset;
		if (type->GetAddressOffset(&address_offset) == S_OK) {
			//elem->SetAttribute("address-offset", address_offset);
			xml.WriteAttributeString(NULL, L"address-offset", NULL, L"0x%x", address_offset);
		}

		ULONG this_adjust;
		if (type->GetThisAdjust(&this_adjust) == S_OK) {
			//elem->SetAttribute("this-adjust", this_adjust);
			xml.WriteAttributeString(NULL, L"this-adjust", NULL, L"0x%x", this_adjust);
		}

		ULONG count;
		if (type->GetArrayCount(&count) == S_OK) {
			if (type->GetTypeTag() == SymTagArrayType) {
				//elem->SetAttribute("array-count", count);
				xml.WriteAttributeString(NULL, L"array-count", NULL, to_wstring(count).c_str());
			}
			else {
				//elem->SetAttribute("argument-count", count);
				xml.WriteAttributeString(NULL, L"argument-count", NULL, to_wstring(count).c_str());
			}
		}

		CV_call_e calling;
		if (type->GetCallingConversion(&calling) == S_OK) {
			//elem->SetAttribute("cv", Dbg_GetCallingConversionName(calling));
			xml.WriteAttributeString(NULL, L"cv", NULL, CDbgEngine::GetCallingConversionName(calling));
		}

		HRESULT hr = S_OK;
		shared_ptr<CDbgType> next_type;
		if (type->GetNextType(&next_type) == S_OK) {
			//elem->LinkEndChild(MakeXmlChildNode(next_type, typecache));
			hr = MakeXmlChildNode(next_type, typecache);
			if (FAILED(hr))
				return hr;
		}

		shared_ptr<CDbgType> array_index_type;
		if (type->GetArrayIndexType(&array_index_type) == S_OK) {
			//TiXmlElement *next_elem = new TiXmlElement("array-index");
			//elem->LinkEndChild(next_elem);
			//next_elem->LinkEndChild(MakeXmlChildNode(array_index_type, typecache));
			xml.WriteStartElement(NULL, L"array-index", NULL);
			hr = MakeXmlChildNode(array_index_type, typecache);
			if (FAILED(hr))
				return hr;
			xml.WriteEndElement();
		}

		shared_ptr<CDbgType> vtable_type;
		if (type->GetVirtualTableShapeType(&vtable_type) == S_OK) {
			//TiXmlElement *next_elem = new TiXmlElement("vtable");
			//elem->LinkEndChild(next_elem);
			//next_elem->LinkEndChild(MakeXmlChildNode(vtable_type, typecache));
			xml.WriteStartElement(NULL, L"vtable", NULL);
			hr = MakeXmlChildNode(vtable_type, typecache);
			if (FAILED(hr))
				return hr;
			xml.WriteEndElement();
		}

		shared_ptr<CDbgType> parent_type;
		if (type->GetClassParentType(&parent_type) == S_OK) {
			//TiXmlElement *next_elem = new TiXmlElement("parent");
			//elem->LinkEndChild(next_elem);
			//next_elem->LinkEndChild(MakeXmlChildNode(parent_type, typecache));
			xml.WriteStartElement(NULL, L"parent", NULL);
			hr = MakeXmlChildNode(parent_type, typecache);
			if (FAILED(hr))
				return hr;
			xml.WriteEndElement();
		}

		CDbgType::List children;
		if (type->GetChildTypes(&children) == S_OK) {
			//TiXmlElement *children_elem = new TiXmlElement("children");
			//elem->LinkEndChild(children_elem);
			xml.WriteStartElement(NULL, L"children", NULL);

			CDbgType::List::iterator it;
			for (it = children.begin(); it != children.end(); ++it) {
				//shared_ptr<CDbgType> child = *it;
				//children_elem->LinkEndChild(MakeXmlChildNode(*it, typecache));
				shared_ptr<CDbgType> child = *it;
				hr = MakeXmlChildNode(child, typecache);
				if (FAILED(hr))
					return hr;
			}
			xml.WriteEndElement();
		}

		xml.WriteEndElement();	//type
		return xml.GetError();
	}

	HRESULT MakeXmlNode(CDbgType::List &typelist) {
		//TiXmlElement *elem = new TiXmlElement("type-root");
		xml.WriteStartElement(NULL, L"type-root", NULL);
		DbgTypeSet typecache;
		int total = typelist.size();
		int i = 0;
		int prev = -1;
		CDbgType::List::iterator it;
		for (it = typelist.begin(); it != typelist.end(); ++it) {
			//shared_ptr<DbgType> type = *it;

			//TiXmlElement *node = MakeXmlChildNode(type, typecache);
			//if (node != NULL) {
			//	elem->LinkEndChild(node);
			//}
			shared_ptr<CDbgType> type = *it;
			HRESULT hr = MakeXmlChildNode(type, typecache);
			if (FAILED(hr))
				return hr;
			auto progress = (int)(i*1000.0/total);
			if (prev != progress) {
				prev = progress;
				wcout << progress << "/1000\r" << flush;
			}
			i++;
		}

		return xml.GetError();
	}


};

void TraceLog(const char *fmt, ...) {
	va_list vlist;

	va_start(vlist, fmt);
	vfprintf(stdout, fmt, vlist);
	va_end(vlist);
}

int DbgDump(std::shared_ptr<CDbgEngine> engine, LPCTSTR filename) {
	TraceLog("Begin to initialize module information.\n");
	std::shared_ptr<CDbgModule> module;
	HRESULT hr = CreateInsatnce<CDbgModule>(engine, filename, &module);
	if (hr != S_OK) {
		fprintf(stdout, "Couldn't load the '%S' module.\n", filename);
		return -1;
	}

	if (module->GetModuleInfo()->SymType != SymCv &&
		module->GetModuleInfo()->SymType != SymPdb &&
		module->GetModuleInfo()->SymType != SymDia) {
		fprintf(stderr, "There is no debug info in '%S'.\n", filename);
		return -1;
	}
	TraceLog("Succeeded in the initialization of module information.\n");

	TraceLog("\nSymbols\n");
	TraceLog("Begin to collect symbol information.\n");
	CDbgSymbol::List symbollist = CDbgSymbol::GetModuleSymbols(engine, module->GetImageBase());
	TraceLog("Succeeded in the collection of symbol information.\n");
	{
		TraceLog("Begin to create the xml nodes.\n");
		//TiXmlDocument doc;
		//TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
		//doc.LinkEndChild(decl);
		//doc.LinkEndChild(MakeSymbolNode(module, symbollist));

		_com_ptr_t<_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)> > pWriter;
		HRESULT hr = ::CreateWriter(&pWriter);
		if (FAILED(hr))
			return hr;
		CDumpXml dump(pWriter);
		std::wstring out_filenamefile = std::wstring(filename) + _T(".symbols.xml");
		_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)> > pFile;
		hr = ::CreateStreamFile(out_filenamefile.c_str(), STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_NONE, &pFile);
		if (FAILED(hr))
			return hr;
		dump.xml.SetOutput(pFile);
		dump.xml.SetProperty(XmlWriterProperty_Indent, TRUE);
		dump.xml.WriteStartDocument(XmlStandalone_Omit);
		hr = dump.MakeSymbolNode(module, symbollist);
		if (FAILED(hr))
			return hr;
		dump.xml.WriteFullEndElement();
		dump.xml.WriteEndDocument();
		dump.xml.Flush();
		TraceLog("Succeeded in the creation of the xml nodes.\n");

		//std::string out_filenamefile = filename + ".symbols.xml";
		//TraceLog("Begin to output symbols information to '%s'.\n", out_filenamefile.c_str());
		//doc.SaveFile(out_filenamefile);
		//TraceLog("Succeeded in the output of symbol information to '%s'.\n", out_filenamefile.c_str());

		//doc.Print(stdout);
	}

	TraceLog("\nTypes\n");
	TraceLog("Begin to collect type infomation.\n");
	CDbgType::List typelist = CDbgType::GetModuleTypes(engine, module->GetImageBase());
	TraceLog("Succeeded in the collection of type infomation.\n");
	{
		TraceLog("Begin to create the xml nodes.\n");
		//TiXmlDocument doc;
		//TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "", "");
		//doc.LinkEndChild(decl);
		//doc.LinkEndChild(MakeXmlNode(typelist));

		_com_ptr_t<_com_IIID<IXmlWriter, &__uuidof(IXmlWriter)> > pWriter;
		HRESULT hr = ::CreateWriter(&pWriter);
		if (FAILED(hr))
			return hr;
		CDumpXml dump(pWriter);
		std::wstring out_filenamefile = std::wstring(filename) + _T(".types.xml");
		_com_ptr_t<_com_IIID<IStream, &__uuidof(IStream)> > pFile;
		hr = ::CreateStreamFile(out_filenamefile.c_str(), STGM_CREATE | STGM_WRITE | STGM_SHARE_DENY_NONE, &pFile);
		if (FAILED(hr))
			return hr;
		dump.xml.SetOutput(pFile);
		dump.xml.SetProperty(XmlWriterProperty_Indent, TRUE);
		dump.xml.WriteStartDocument(XmlStandalone_Omit);
		hr = dump.MakeXmlNode(typelist);
		if (FAILED(hr))
			return hr;
		dump.xml.WriteFullEndElement();
		dump.xml.WriteEndDocument();
		dump.xml.Flush();

		TraceLog("Succeeded in the creation of the xml nodes.\n");

		//std::string out_filename = filename + ".types.xml";
		//TraceLog("Begin to output type infomation to '%s'.\n", out_filename.c_str());
		//doc.SaveFile(out_filename);
		//TraceLog("Succeeded in the output of type information to '%s'.\n", out_filename.c_str());

		//doc.Print(stdout);
	}
	
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	std::locale::global(std::locale(""));
	std::shared_ptr<CDbgEngine> engine;
	HRESULT hr = CreateInsatnce((HMODULE)GetCurrentProcess(), &engine);
	DbgDump(engine, argv[1]);
	return 0;
}
