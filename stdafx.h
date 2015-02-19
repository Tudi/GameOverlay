// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


// TODO: reference additional headers your program requires here
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <assert.h>
#include <tmmintrin.h>
#include <conio.h>

#include <atlimage.h>
#include <assert.h>
#include "ScreenCap.h"
#include "PiramidImg.h"
#include "Tools.h"
#include "MotionEstimation.h"
#include "ImageTools.h"
#include "Interception.h"
#include "KeyboardHandlerInit.h"
#include "DrawOutput.h"
#include "Resampling.h"
#include "DetectCenterOfMotion.h"
#include "RecognizableObject.h"
#include "3dShooterHandler.h"
#include "ClickerHeroes.h"

DWORD WINAPI dummymain( LPVOID lpParam );

struct GlobalStore
{
	HWND					WndPaintDst;
	unsigned char			TransparentR,TransparentG,TransparentB;
	InterceptionContext		InterceptoinContext;
	InterceptionDevice		KeyboardDevice;
	InterceptionDevice		MouseDevice;
	InterceptionKeyStroke	KeyboardStroke;
	InterceptionMouseStroke	MouseStroke;
	int						MouseXLimitMin,MouseYLimitMin,MouseXLimitMax,MouseYLimitMax;	//this is only checked periodically !
	int						TrackedMouseX,TrackedMouseY,DosBoxWidth,DosBoxHeight,TrackedMouseScriptX,TrackedMouseScriptY,TrackedMouseScriptStamp;
	int						DrawThreadAlive;
	CScreenImage			*InProgressDrawImage;
};

extern GlobalStore GlobalData;

#define DEFAULT_BUFLEN 200
#define RGB_BYTE_COUNT 4	//not safe to macro this as default RGB byte count is actually 3 bytes