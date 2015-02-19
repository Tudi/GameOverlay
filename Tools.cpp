#include "StdAfx.h"

void SearchImgOnScreen( CImage *Screen, CImage *srch, int *retx, int *rety, int *Sad )
{
	DWORD BestSad = 0x0FFFFFFF;
	int PixelJump = 3;
	for( int y = 0; y < Screen->GetHeight() - srch->GetHeight(); y += PixelJump )
		for( int x = 0; x < Screen->GetWidth() - srch->GetWidth(); x += PixelJump )
		{
			DWORD SAD = 0;
			for( int y1 = 0; y1 < srch->GetHeight(); y1 += PixelJump )
				for( int x1 = 0; x1 < srch->GetWidth(); x1 += PixelJump )
				{
					COLORREF ScreenCol = Screen->GetPixel( x + x1, y + y1 );
					COLORREF SrchCol = srch->GetPixel( x1, y1 );
					DWORD R1 = ( ScreenCol & 0x000000FF ) >> 0;
					DWORD R2 = ( SrchCol & 0x000000FF ) >> 0;
					DWORD G1 = ( ScreenCol & 0x0000FF00 ) >> 8;
					DWORD G2 = ( SrchCol & 0x0000FF00 ) >> 8;
					DWORD B1 = ( ScreenCol & 0x00FF0000 ) >> 16;
					DWORD B2 = ( SrchCol & 0x00FF0000 ) >> 16;
					SAD += abs( R1 - R2 ) + abs( G1 - G2 ) + abs( B1 - B2 );
				}
			if( BestSad > SAD )
			{
				BestSad = SAD;
				*retx = x;
				*rety = y;
			}
		}
	*Sad = BestSad;
	printf("Best location found at %d %d with SAD %d\n", *retx, *rety, BestSad );
}

void StupidImgCopy( CImage *Dest, CImage *Src, int DestX, int DestY )
{
	for( int y = 0; y < Src->GetHeight() && DestY + y < Dest->GetHeight(); y++ )
		for( int x = 0; x < Src->GetWidth() && DestX + x < Dest->GetWidth(); x++ )
		{
			COLORREF SrcC = Src->GetPixel( x, y );
			if( SrcC == 0x00FFFFFF )
				continue;
			Dest->SetPixel( x + DestX, y + DestY, SrcC );
		}
}

#ifdef COUNT_NUMBER_OF_SADS
int SadCounter = 0;
#endif

int GetLocalSad( int *a, int *b, int stride1, int stride2, int w, int h )
{
	int RSadNow = 0;
	for( int y1 = 0; y1 < h; y1++ )
		for( int x1 = 0; x1 < w; x1++ )
		{
			int BigC = a[ y1 * stride1 + x1 ];
			int SmallC = b[ y1 * stride2 + x1 ];
			RSadNow += abs( BigC - SmallC );
#ifdef COUNT_NUMBER_OF_SADS
			SadCounter++;
#endif
		}
	return RSadNow;
}

//only search one layer and see if it can improve the search on next layer
int PiramidSearchCanRefine( int Layer, PiramidImage *Big, PiramidImage *Small, int atX, int atY, int RSadPrev, int GSadPrev, int BSadPrev, int &RetX, int &RetY, int &RetRSad, int &RetGSad, int &RetBSad  )
{
	if( Layer < 0 )
	{
		RetRSad = RSadPrev;
#ifndef GENERATE_ONLY_R
		RetGPrev = GSadPrev;
		RetBPrev = BSadPrev;
#endif
		RetX = atX;
		RetY = atY;
		return 0;
	}

	//too small to sample this layer ?
	if( Small->ImageLayersY[Layer] <= MIN_SIZE_FOR_SEARCH || Small->ImageLayersX[Layer] <= MIN_SIZE_FOR_SEARCH )
		return 0;

	int BestRSad = MAX_INT;
	int BestGSad = MAX_INT;
	int BestBSad = MAX_INT;

	int PixelsInPixel = 1;
	for( int i = 0; i < Layer; i++ )
		PixelsInPixel *= PIXEL_STEPDOWN_LAYER;

	int StartX = atX / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
	int EndX = atX / PixelsInPixel + PIXEL_STEPDOWN_LAYER;
	int StartY = atY / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
	int EndY = atY / PixelsInPixel + PIXEL_STEPDOWN_LAYER;

	if( StartX < 0 )
		StartX = 0;
	if( EndX > Big->ImageLayersX[Layer] - Small->ImageLayersX[Layer] - 1 )
		EndX = Big->ImageLayersX[Layer] - Small->ImageLayersX[Layer] - 1;

	if( StartY < 0 )
		StartY = 0;
	if( EndY > Big->ImageLayersY[Layer] - Small->ImageLayersY[Layer] - 1 )
		EndY = Big->ImageLayersY[Layer] - Small->ImageLayersY[Layer] - 1;

	for( int y = StartY; y <= EndY; y++ )
		for( int x = StartX; x < EndX; x++ )
		{
			int RSadNow = GetLocalSad( &Big->ImageLayers[0][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[0][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
			if( RSadNow < BestRSad )
			{
#ifndef GENERATE_ONLY_R
				int GSadNow = GetLocalSad( &Big->ImageLayers[1][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[1][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
				int BSadNow = GetLocalSad( &Big->ImageLayers[2][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[2][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
				if( RSadNow + GSadNow + BSadNow < BestRSad + BestGSad + BestBSad )
#endif
				{
					BestRSad = RSadNow;
#ifndef GENERATE_ONLY_R
					BestGSad = GSadNow;
					BestBSad = BSadNow;
#endif
					RetX = x * PixelsInPixel;
					RetY = y * PixelsInPixel;
				}
			}
		}
#ifndef GENERATE_ONLY_R
	RetRSad = BestRSad;
	RetGSad = BestGSad;
	RetBSad = BestBSad;
	return ( ( BestRSad - RSadPrev ) + ( BestGSad - GSadPrev ) + ( BestBSad - BSadPrev ) );
#else
	RetRSad = BestRSad;
	return ( BestRSad - RSadPrev );
#endif
}

//search all layers
int PiramidSearch( PiramidImage *Big, PiramidImage *Small, int *RetX, int *RetY, int *RSad, int *GSad, int *BSad, int SadLimit, int pLayer, int AtX, int AtY, int RecCount )
{
	//in case we fail at the search, somehow signal back
	*RetX = 0;
	*RetY = 0;
	*RSad = *GSad = *BSad = MAX_INT;

	int BestRSad = MAX_INT;
	int BestGSad = MAX_INT;
	int BestBSad = MAX_INT;
	int BestRefine = MAX_INT;

	//search in worst version only in R
	int LayerStart = MAX_IMAGE_LAYERS-1;
	int LayerEnd = -1;
	if( AtX != -1 )
	{
		LayerStart = pLayer;
		LayerEnd = pLayer-1;
		if( LayerEnd < -1 )
			LayerEnd = -1;
	}

	int FirstSearchedLayer = 1;
	int CanSkipNextLayer = 0;

	for( int Layer=LayerStart; Layer>LayerEnd; Layer = Layer - 1 - CanSkipNextLayer )
	{
		//too small to sample this layer ?
		if( Small->ImageLayersY[Layer] <= MIN_SIZE_FOR_SEARCH || Small->ImageLayersX[Layer] <= MIN_SIZE_FOR_SEARCH )
			continue;

		CanSkipNextLayer = 0;

		int PixelsInPixel = 1;
		for( int i = 0; i < Layer; i++ )
			PixelsInPixel *= PIXEL_STEPDOWN_LAYER;

		int StartX, EndX, StartY, EndY;
		if( AtX != -1 )
		{
			StartX = AtX / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
			EndX = AtX / PixelsInPixel + PIXEL_STEPDOWN_LAYER;
			StartY = AtY / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
			EndY = AtY / PixelsInPixel + PIXEL_STEPDOWN_LAYER;
		}
		else if( FirstSearchedLayer == 0 && BestRSad != MAX_INT )
		{
			StartX = *RetX / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
			EndX = *RetX / PixelsInPixel + PIXEL_STEPDOWN_LAYER;
			StartY = *RetY / PixelsInPixel - PIXEL_STEPDOWN_LAYER;
			EndY = *RetY / PixelsInPixel + PIXEL_STEPDOWN_LAYER;
		}
		else
		{
			StartX = 0;
			EndX = Big->ImageLayersX[Layer] - Small->ImageLayersX[Layer] - 1;
			StartY = 0;
			EndY = Big->ImageLayersY[Layer] - Small->ImageLayersY[Layer] - 1;
		}

		if( StartX < 0 )
			StartX = 0;
		if( EndX > Big->ImageLayersX[Layer] - Small->ImageLayersX[Layer] - 1 )
			EndX = Big->ImageLayersX[Layer] - Small->ImageLayersX[Layer] - 1;

		if( StartY < 0 )
			StartY = 0;
		if( EndY > Big->ImageLayersY[Layer] - Small->ImageLayersY[Layer] - 1 )
			EndY = Big->ImageLayersY[Layer] - Small->ImageLayersY[Layer] - 1;

		printf("Starting search layer %d from %d %d to %d %d or %d %d to %d %d. Recursion %d. Small img %d %d\n",Layer, StartX, StartY, EndX, EndY, StartX * PixelsInPixel, StartY * PixelsInPixel, EndX * PixelsInPixel, EndY * PixelsInPixel, RecCount, Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
		for( int y = StartY; y <= EndY; y++ )
			for( int x = StartX; x <= EndX; x++ )
			{
//if( y * PixelsInPixel <= 69 && ( y + 1 ) * PixelsInPixel >= 69 && x * PixelsInPixel <= 9 && ( x + 1 ) * PixelsInPixel >= 9 )
//if( y * PixelsInPixel <= 69 && ( y + 1 ) * PixelsInPixel >= 69 && x * PixelsInPixel <= 9 && ( x + 1 ) * PixelsInPixel >= 9 && Layer == 2 )
//if( y == 69 && x == 9 && Layer == 0 )
//	printf( "1903471923");
				int RSadNow = GetLocalSad( &Big->ImageLayers[0][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[0][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
				int GSadNow = 0;
				int BSadNow = 0;
#ifdef MERGE_RGB_INTO_R
				if( RSadNow <= BestRSad )
#endif
				{
#ifndef GENERATE_ONLY_R
					GSadNow = GetLocalSad( &Big->ImageLayers[1][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[1][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
					BSadNow = GetLocalSad( &Big->ImageLayers[2][Layer][ ( y + 0 ) * Big->ImageLayersX[Layer] + ( x + 0 ) ], &Small->ImageLayers[2][Layer][ ( 0 + 0 ) * Small->ImageLayersX[Layer] + ( 0 + 0 ) ], Big->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersX[Layer], Small->ImageLayersY[Layer] );
					if( RSadNow + GSadNow + BSadNow <= BestRSad + BestGSad + BestBSad )
#endif
					{
						//check if 
						int BetterX, BetterY, BetterRSad, BetterGSad, BetterBSad;
						int RefineAmt = 0;
						if( Layer > 0 )
							RefineAmt = PiramidSearchCanRefine( Layer - 1, Big, Small, x * PixelsInPixel, y * PixelsInPixel, RSadNow, GSadNow, BSadNow, BetterX, BetterY, BetterRSad, BetterGSad, BetterBSad );
						else
						{
							BetterX = x * PixelsInPixel;
							BetterY = y * PixelsInPixel;
							BetterRSad = RSadNow;
							BetterGSad = GSadNow;
							BetterBSad = BSadNow;
						}
						if( RefineAmt <= 0 )
						{
							if( Layer > 1 )
								CanSkipNextLayer = 1;

							BestRSad = BetterRSad;
#ifndef GENERATE_ONLY_R
							BestGSad = BetterGSad;
							BestBSad = BetterBSad;
#endif

							*RetX = BetterX;
							*RetY = BetterY;

#ifndef GENERATE_ONLY_R
							if( BestRSad + BestGSad + BestBSad <= SadLimit )
#else
							if( BestRSad <= SadLimit )
#endif
							{
								printf("For Layer %d best sad %d at %d %d \n", Layer, BestRSad, *RetX, *RetY );
								printf("Perfect match found. Aborting higher layer search\n" );
								return 0;
							}
						}
					}
				}
			}
		for( int t=0;t<RecCount;t++)
			printf("\t");
//		printf("For Layer %d best sad %d at %d %d \n", Layer, BestRSad, *RetX, *RetY );
#ifdef GENERATE_ONLY_R
		BestGSad = BestBSad = 0;
#endif
		printf("For Layer %d best sad %d at %d %d L = %d x = %d y = %d\n", Layer, BestRSad + BestGSad + BestBSad, *RetX, *RetY, pLayer, AtX, AtY );
		printf("\n");

		FirstSearchedLayer = 0;
	}
	return 0;
}

void PainBltImgToWnd( HWND hWnd, CImage *img )
{
	RECT rec;
	GetWindowRect( hWnd, &rec );
	//valid for window with scrollbar
//	int w = rec.right - rec.left - 25;
//	int h = rec.bottom - rec.top - 5;
	//valid for window with scrollbar
	int w = rec.right - rec.left;
	int h = rec.bottom - rec.top;

	HDC hdc  = GetDC( hWnd );
	img->StretchBlt( hdc, 0, 0, w, h );
	ReleaseDC( hWnd, hdc );
}

typedef DWORD (WINAPI *PSLWA)(HWND, DWORD, BYTE, DWORD);

static PSLWA pSetLayeredWindowAttributes = NULL;
static BOOL initialized = FALSE;

DWORD MakeWindowTransparent(HWND hWnd, unsigned char factor, unsigned char r, unsigned char g, unsigned char b)
{
	assert( hWnd );
	/* First, see if we can get the API call we need. If we've tried
	 * once, we don't need to try again. */
	if (!initialized)
	{
		HMODULE hDLL = LoadLibrary ("user32");

		pSetLayeredWindowAttributes = 
			(PSLWA) GetProcAddress(hDLL, "SetLayeredWindowAttributes");

		initialized = TRUE;
	}

	if (pSetLayeredWindowAttributes == NULL) 
		return FALSE;

	/* Windows need to be layered to be made transparent. This is done
		* by modifying the extended style bits to contain WS_EX_LAYARED. */
	SetLastError(0);

	SetWindowLong( hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

//	DWORD OLDStyle = GetWindowLong(hWnd, GWL_STYLE);
//	DWORD NewStyle = OLDStyle & ~(WS_BORDER | WS_DLGFRAME | WS_THICKFRAME | WS_CAPTION | WS_VSCROLL | WS_HSCROLL | WS_SYSMENU );
//	NewStyle = NewStyle & ~WS_CLIPSIBLINGS;
//    ::SetWindowLong( hWnd, GWL_STYLE, WS_VISIBLE );
//    ::SetWindowLong( hWnd, GWL_EXSTYLE, ::GetWindowLong( hWnd, GWL_EXSTYLE) & ~( WS_EX_DLGMODALFRAME ) );
	if (GetLastError())
		return FALSE;

	/* Now, we need to set the 'layered window attributes'. This
	* is where the alpha values get set. */
//	return pSetLayeredWindowAttributes (hWnd, RGB(255,255,255), factor, LWA_COLORKEY|LWA_ALPHA );
	return pSetLayeredWindowAttributes( hWnd, RGB(r,g,b), factor, LWA_COLORKEY|LWA_ALPHA );
}

// Get the horizontal and vertical screen sizes in pixel
void GetDesktopResolution(int& horizontal, int& vertical)
{
   RECT desktop;
   // Get a handle to the desktop window
   const HWND hDesktop = GetDesktopWindow();
   // Get the size of screen to the variable desktop
   GetWindowRect(hDesktop, &desktop);
   // The top left corner will have coordinates (0,0)
   // and the bottom right corner will have coordinates
   // (horizontal, vertical)
   horizontal = desktop.right;
   vertical = desktop.bottom;
}

void MakeConsoleTransparentAndActive( HWND hWnd )
{
	MakeWindowTransparent( hWnd, 255, GlobalData.TransparentR, GlobalData.TransparentG, GlobalData.TransparentB );
	int maxx,maxy;
	GetDesktopResolution( maxx, maxy );
//	SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, maxx, maxy, SWP_NOMOVE | SWP_NOSIZE );
	SetWindowPos( hWnd, HWND_TOPMOST, 0, 0, maxx, maxy, 0 );
//	SetWindowPos( hWnd, HWND_TOPMOST, 300, 300, 600, 600, 0 );
}

HWND FindMyTopMostWindow()
{
    DWORD dwProcID = GetCurrentProcessId();
    HWND hWnd = GetTopWindow( GetDesktopWindow() ); //i can click through my window
    while(hWnd)
    {
        DWORD dwWndProcID = 0;
        GetWindowThreadProcessId( hWnd, &dwWndProcID );
        if( dwWndProcID == dwProcID )
            return hWnd;            
        hWnd = GetNextWindow( hWnd, GW_HWNDNEXT );
    }
	return GetForegroundWindow();
}

#define MAX_LOADSTRING 100

// Child window
TCHAR _szTitleChild[MAX_LOADSTRING]; // Caption
TCHAR _szWindowClassChild[MAX_LOADSTRING]; // Class

LRESULT CALLBACK ChildWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
/**/
/*    switch ( msg )
    {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            break;
        }
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);/**/
}

HWND Child;

DWORD WINAPI MessageListenerThread( LPVOID lpParam )
{
    MSG msg ;
    while( GetMessage( &msg, Child, 0, 0 ) ) 
		DispatchMessage(&msg) ;
	return 0;
}

void CreateNewWindow( HWND owner, HWND &child )
{
	HINSTANCE _hInst = GetModuleHandle(NULL);
	// Child window
	strcpy_s( _szTitleChild, MAX_LOADSTRING, "Resampled image" );
	strcpy_s( _szWindowClassChild, MAX_LOADSTRING, "RESAMPLECHILDWND" );

	WNDCLASSEX wcex;

	memset( &wcex, 0, sizeof( wcex ) );

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= ChildWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= _hInst;
	wcex.hIcon			= LoadIcon(NULL, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH));
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= _szWindowClassChild;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	INT w, h;

	HDC hdc = GetDC(NULL);
	w = GetDeviceCaps(hdc, HORZRES)/2;
	h = GetDeviceCaps(hdc, VERTRES)/2;
	ReleaseDC(NULL, hdc);
		
//	child = CreateWindow( _szWindowClassChild, _szTitleChild, WS_OVERLAPPED | WS_SIZEBOX, w, 0, w, h, owner, NULL, _hInst, NULL);
//	child = CreateWindow( _szWindowClassChild, _szTitleChild, WS_OVERLAPPED | WS_SIZEBOX, w, 0, w, h, NULL, NULL, _hInst, NULL);
	child = CreateWindow( _szWindowClassChild, _szTitleChild, WS_OVERLAPPED | WS_SIZEBOX, w, 0, w, h, HWND_DESKTOP, NULL, _hInst, NULL);

	if( !child )
	{
		return;
	}

//	ShowWindow( _hWndChild, nCmdShow );
	ShowWindow( child, SW_SHOWDEFAULT );
	UpdateWindow( child );

	DWORD   dwThreadId;
	HANDLE  hThread; 
		hThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MessageListenerThread,			// thread function name
			0,						// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 
	if (hThread == NULL) 
	   ExitProcess(3);
}


void CreateNewWindow2( HWND owner, HWND &child )
{
    const char* const myclass = "myclass" ;
    WNDCLASSEX wndclass = { sizeof(WNDCLASSEX), CS_DBLCLKS, ChildWndProc,
                            0, 0, GetModuleHandle(0), LoadIcon(0,IDI_APPLICATION),
                            LoadCursor(0,IDC_ARROW), HBRUSH(COLOR_WINDOW+1),
                            0, myclass, LoadIcon(0,IDI_APPLICATION) } ;
    if( RegisterClassEx(&wndclass) )
    {
        HWND window = CreateWindowEx( 0, myclass, "title",
                   WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                   CW_USEDEFAULT, CW_USEDEFAULT, owner, 0, GetModuleHandle(0), 0 ) ;
        if(window)
        {
            ShowWindow( window, SW_SHOWDEFAULT ) ;
			child = Child = window;
//            MSG msg ;
//            while( GetMessage( &msg, 0, 0, 0 ) ) DispatchMessage(&msg) ;
        }
    }

	DWORD   dwThreadId;
	HANDLE  hThread; 
		hThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			MessageListenerThread,			// thread function name
			0,						// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 
	if (hThread == NULL) 
	   ExitProcess(3);
}

struct SearchCallbackIOStruct
{
	char *SearchName;
	HWND ret;
	bool SearchDone;
};

BOOL CALLBACK EnumWindowsProcFindWndByName( HWND hwnd, LPARAM lParam )
{
	char WindowTitle[ DEFAULT_BUFLEN ];
	SearchCallbackIOStruct *pIO = (SearchCallbackIOStruct*)lParam;

	if( pIO->SearchDone == true )
		return FALSE;

	GetWindowText( hwnd, WindowTitle, sizeof( WindowTitle ) );

	if( strstr( WindowTitle, pIO->SearchName ) != NULL )
	{
		pIO->ret = hwnd;
		pIO->SearchDone = true;
	}
	return TRUE;
}

SearchCallbackIOStruct MySearch;
HWND FindWindowWithNameNonThreaded( char *Name )
{
	if( Name == NULL )
		return 0;

	MySearch.ret = 0; // mark that we did not find it yet
	MySearch.SearchName = Name;
	MySearch.SearchDone = false;

	EnumWindows( EnumWindowsProcFindWndByName, (LPARAM)&MySearch );
	int WaitTimeout = 1000;
	while( MySearch.SearchDone == false && WaitTimeout > 0 )
	{
		WaitTimeout -= 100;
		Sleep( 100 );
	}

	if( WaitTimeout <= 0 )
		printf( "searching for window with title %s timed out !\n", Name );

	return MySearch.ret;
}
