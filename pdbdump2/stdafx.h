// stdafx.h : 標準のシステム インクルード ファイルのインクルード ファイル、または
// 参照回数が多く、かつあまり変更されない、プロジェクト専用のインクルード ファイル
// を記述します。
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <comdef.h>	//use _com_ptr_t

#include <cassert>

// TODO: プログラムに必要な追加ヘッダーをここで参照してください。
#include <memory>
#include <string>
#include <vector>
#include <list>

#include "DbgHelper.h"
#include "tstring.h"
