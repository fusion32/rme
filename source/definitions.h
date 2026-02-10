//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_DEFINITIONS_H_
#define RME_DEFINITIONS_H_

#define __RME_APPLICATION_NAME__ "Remere's Map Editor"
#define __RME_WEBSITE_URL__      "https://otland.net"

// Version info
// xxyyzzt (major, minor, patch)
#define __RME_VERSION_MAJOR__      7
#define __RME_VERSION_MINOR__      7
#define __RME_VERSION_PATCH__      0

#define MAKE_VERSION_ID(major, minor, patch) ((major) * 10000000 + (minor) * 100000 +  (patch) * 1000)
#define __RME_VERSION_ID__ MAKE_VERSION_ID(__RME_VERSION_MAJOR__, __RME_VERSION_MINOR__, __RME_VERSION_PATCH__)

#define STRINGIFY_(x) #x
#define STRINGIFY(x) STRINGIFY_(x)
#define __RME_VERSION__ ("v" STRINGIFY(__RME_VERSION_MAJOR__) "." STRINGIFY(__RME_VERSION_MINOR__) "." STRINGIFY(__RME_VERSION_PATCH__) "T")

#if defined(__DEBUG__) && defined(NDEBUG)
#	undef NDEBUG
#endif

#if defined(_MSC_VER) && !defined(__DEBUG__)
#	define _ITERATOR_DEBUG_LEVEL 0
#endif

// wxString conversions
#define nstr(str) std::string((const char*)(str.mb_str(wxConvUTF8)))
#define wxstr(str) wxString((str).c_str(), wxConvUTF8)

#endif
