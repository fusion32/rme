//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_MAIN_H_
#define RME_MAIN_H_

#if defined(_DEBUG) && defined(NDEBUG)
#	undef NDEBUG
#endif

#if defined(_MSC_VER) && !defined(_DEBUG)
#	define _ITERATOR_DEBUG_LEVEL 0
#endif

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	ifdef _WIN32_WINNT
#		undef _WIN32_WINNT
#	endif
#	define _WIN32_WINNT 0x0501
#	include <crtdbg.h>
#endif

#ifdef DEBUG_MEM
#define _CRTDBG_MAP_ALLOC
#pragma warning(disable: 4291)
_Ret_bytecap_(_Size) inline void * __CRTDECL operator new(size_t _Size, const char* file, int line)
        { return ::operator new(_Size, _NORMAL_BLOCK, file, line); }
_Ret_bytecap_(_Size) inline void* __CRTDECL operator new[](size_t _Size, const char* file, int line)
        { return ::operator new[](_Size, _NORMAL_BLOCK, file, line); }
#define newd new(__FILE__, __LINE__)
#else
#define newd new
#endif //DEBUG_MEM

// stdlib
#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

// stdlib C++
#include <list>
#include <vector>
#include <map>
#include <string>
#include <istream>
#include <ostream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <set>
#include <queue>
#include <stdexcept>
#include <fstream>
#include <memory>
#include <exception>
#include <cmath>
#include <ranges>
#include <regex>

// wxWidgets
#include <wx/defs.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#   include <wx/wx.h>
#endif
#include <wx/thread.h>
#include <wx/utils.h>
#include <wx/progdlg.h>
#include <wx/glcanvas.h>
#include <wx/debugrpt.h>
#include <wx/minifram.h>
#include <wx/gbsizer.h>
#include <wx/choicebk.h>
#include <wx/tglbtn.h>
#include <wx/dcbuffer.h>
#include <wx/aui/aui.h>
#include <wx/cmdline.h>
#include <wx/filename.h>
#include <wx/filepicker.h>
#include <wx/arrstr.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/vlbox.h>
#include <wx/stdpaths.h>
#include <wx/grid.h>
#include <wx/clipbrd.h>

// pugixml
#include "ext/pugixml.hpp"

#define ASSERT assert

#if wxCHECK_VERSION(3, 1, 0)
#   define FROM_DIP(widget, size) widget->FromDIP(size)
#else
#   define FROM_DIP(widget, size) size
#endif

// TODO(fusion): Probably get rid of these?
// wxString conversions
#define nstr(str) std::string((const char*)(str.mb_str(wxConvUTF8)))
#define wxstr(str) wxString((str).c_str(), wxConvUTF8)

#include "definitions.h"
#include "common.h"
#include "const.h"
#include "forward.h"
#include "threads.h"

#endif
