#ifndef _UI_HELPERS_H_
#define _UI_HELPERS_H_

#define OEMRESOURCE

#include <algorithm>
#include <unordered_map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include <windows.h>
#include <WindowsX.h>
#include <SHLWAPI.H>
#include <vssym32.h>
#include <uxtheme.h>
#include <shldisp.h>
#include <ShlObj.h>
#include <gdiplus.h>
#include <Usp10.h>

#include "../pfc/pfc.h"
#include "../foobar2000/shared/shared.h"
#include <CommonControls.h>

#include "../mmh/stdafx.h"

#include "../columns_ui-sdk/ui_extension.h"

#include "handle.h"
#include "win32_helpers.h"
#include "ole.h"

#include "container_window.h"
#include "message_hook.h"
#include "low_level_hook.h"
#include "trackbar.h"
#include "solid_fill.h"

#include "gdi.h"
#include "text_drawing.h"
#include "List View/list_view.h"
#include "drag_image.h"
#include "message_window.h"
#include "menu.h"

#include "config_var.h"
#include "fcl.h"

#include "OLE/DataObj.h"
#include "OLE/EnumFE.h"

#endif