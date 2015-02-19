#include "StdAfx.h"

static const WORD MAX_CONSOLE_LINES = 500;


void RedirectIOToConsole()
{
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

// allocate a console for this app
    AllocConsole();

// set the screen buffer to be big enough to let us scroll text
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

// redirect unbuffered STDOUT to the console
    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stdout = *fp;

    setvbuf( stdout, NULL, _IONBF, 0 );

// redirect unbuffered STDIN to the console

    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "r" );
    *stdin = *fp;
    setvbuf( stdin, NULL, _IONBF, 0 );

// redirect unbuffered STDERR to the console
    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

    fp = _fdopen( hConHandle, "w" );

    *stderr = *fp;

    setvbuf( stderr, NULL, _IONBF, 0 );

// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog
// point to console as well
//    ios::sync_with_stdio();
}

void LoopAndSearchFor1Image()
{

//	HWND WndPaintDst = FindMyTopMostWindow();
	HWND WndPaintDst = GlobalData.WndPaintDst;

	HWND hWndSrc = FindWindowWithNameNonThreaded( "Studio" );

	CImage mouse;
	mouse.Load( "search0.bmp" );
//	mouse.Load( "search1.bmp" );
//	mouse.Load( "search2.bmp" );

	PiramidImage Pmouse;
	Pmouse.BuildFromImg( &mouse );

//	Pmouse.SaveLayersToFile( "MouseLayers" );
//	mouse.Save( "LoadTestMouseSave.bmp" );

	CImage overlay;
	overlay.Load( "overlay2.bmp" );

	while( 1 )
	{
		CScreenImage Screen;
		if( hWndSrc )
		{
			Screen.CaptureWindow( hWndSrc );
			Screen.CaptureWindowConvert( hWndSrc );
		}
		else
		{
			Screen.CaptureScreen();			// this version will be used for merge and save
			Screen.CaptureScreenConvert();	// this version will be used for the search
		}
		Screen.Save( "Desktop.bmp" );

		int x,y,rs,gs,bs;
	//	SearchImgOnScreen( &Screen, &mouse, &x, &y, &s );

		int Start = GetTickCount();

		PiramidImage PScreen;
		PScreen.BuildFromImg( &Screen );

	//	PScreen.SaveLayersToFile( "DesktopLayers" );

	//	PiramidSearch( &PScreen, &Pmouse, &x, &y, &rs,&gs, &bs, 0, 0, 9, 69 );
		PiramidSearch( &PScreen, &Pmouse, &x, &y, &rs,&gs, &bs, 0 );
		int End = GetTickCount();
		printf("Build + Search took %d. FPS %f\n", End - Start, 1000.0f / ( End - Start ) );

	#ifdef COUNT_NUMBER_OF_SADS
		printf("Number of SAD calculations done : %d\n", SadCounter );
		printf("Number of SAD calculations flat search small image: %d\n", Pmouse.ImageLayersX[0] * Pmouse.ImageLayersY[0] * 3 );
		double MaxSADS = (double)Pmouse.ImageLayersX[0] * (double)Pmouse.ImageLayersY[0] * (double)PScreen.ImageLayersX[0] * (double)PScreen.ImageLayersY[0] * 3;
		printf("Number of SAD calculations flat search full image: %f\n", (float)MaxSADS );
		double SadCPCT = (double)SadCounter * 100.0;
		printf("Number of SAD calculations done / max : %f%%\n", (float)( SadCPCT / MaxSADS ) );
	#endif

	//	PScreen.SaveLayersToFile( "DesktopLayersFound", x, y, mouse.GetWidth(), mouse.GetHeight() );

		//set screen to transparent
//		Screen.SetToColor( GlobalData.TransparentR, GlobalData.TransparentG, GlobalData.TransparentB );

		//draw overlays
		StupidImgCopy( &Screen, &overlay, x, y );

		//save it for debugging sake
		Screen.Save( "DesktopWithOverlay.bmp" );

		//code to copy our result bmp to the console window
		{
			PainBltImgToWnd( WndPaintDst, &Screen );
			//wait until keypressed
	//		printf("Running until Keypress");
	//		_getch();
		}/**/
		//avoid 100% CPU usage
		Sleep( 1000 );
	}
}

DWORD WINAPI dummymain( LPVOID lpParam )
{
	//wait until main window initializes 
	while( GlobalData.WndPaintDst == 0 )
		Sleep( 100 );

	RedirectIOToConsole();

	//position console window to the lower left part of the screen
	{
		HWND WindowToRepos = GetConsoleWindow();
		RECT WndSize;
		int DesktopWidth,DesktopHeight;
		GetClientRect( WindowToRepos, &WndSize );
		GetDesktopResolution( DesktopWidth, DesktopHeight );
		int NewHeight = 200;
		int NewWidth = WndSize.right;
		MoveWindow( WindowToRepos, DesktopWidth - NewWidth, DesktopHeight - NewHeight, NewWidth, NewHeight, TRUE);
	}

	//position paint window to the upper rigth corner
	{
		HWND WindowToRepos = GlobalData.WndPaintDst;
		RECT WndSize;
		int DesktopWidth,DesktopHeight;
		GetClientRect( WindowToRepos, &WndSize );
		GetDesktopResolution( DesktopWidth, DesktopHeight );
//		int NewHeight = DesktopHeight / 2;
//		int NewWidth = DesktopWidth / 2;
		int NewWidth = 320 * 2;
		int NewHeight = 240 * 2;
		MoveWindow( WindowToRepos, DesktopWidth - NewWidth, 0, NewWidth, NewHeight, TRUE);
	}

	InitDrawThread();

	printf("Waiting for window reposition\n");
	_getch();

	//load previous settings for our paint window and set it up
//	MakeConsoleTransparentAndActive( WndPaintDst );

//	LoopAndSearchFor1Image();
	TrackAndMarkObjectMotion();
//	ClickerHeoresLoop();

	//wait until keypressed
//	printf("Running until Keypress");
//	_getch();

//	printf("Running until Keypress");
//	_getch();
	return 0;
}