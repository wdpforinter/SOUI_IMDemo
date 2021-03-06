// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#pragma warning(disable:4996)
#pragma warning(disable:4018)
#pragma warning(disable:4200)
#pragma warning(disable:4244)
#pragma warning(disable:4305)
#pragma warning(disable:4503)
#pragma warning(disable:4482)
#pragma warning(disable:4005)

#define  _CRT_SECURE_NO_WARNINGS
#define	 DLL_SOUI
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <com-cfg.h>
#include "trayicon/SShellNotifyIcon.h"
#include "resource.h"
#define R_IN_CPP	//定义这个开关来
#include "res\resource.h"
#include "layout/SouiLayout.h"
using namespace SOUI;

#include <Imm.h>
#pragma comment(lib, "imm32.lib")

#include "extend.ctrls/imre/SImRichEdit.h"
#include "extend.ctrls/SButtonEx.h"
#include "extend.ctrls/ExtendCtrls.h"
#include "extend.ctrls/SSplitBar.h"
#include "extend.ctrls/SImageView.h"

#include "extend.skins/ExtendSkins.h"
#include "extend.skins/SAntialiasSkin.h"
#include "extend.skins/SVscrollbar.h"

#include "extend.ctrls/gif/SAniImgFrame.h"
#include "extend.ctrls/gif/SGifPlayer.h"
#include "extend.ctrls/gif/SSkinAPNG.h"
#include "extend.ctrls/gif/SSkinGif.h"

#include "CommonFunction.h"