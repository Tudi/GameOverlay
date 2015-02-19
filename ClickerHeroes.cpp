#include "StdAfx.h"

bool KeyIsPressed ( unsigned char k ) 
{
	USHORT status = GetAsyncKeyState ( k );
	return (( ( status & 0x8000 ) >> 15 ) == 1) || (( status & 1 ) == 1);
}

void LeftClick()
{  
	INPUT    Input={0};													// Create our input.

	Input.type        = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;							// We are setting left mouse button down.
	SendInput( 1, &Input, sizeof(INPUT) );								// Send the input.

	ZeroMemory(&Input,sizeof(INPUT));									// Fills a block of memory with zeros.
	Input.type        = INPUT_MOUSE;									// Let input know we are using the mouse.
	Input.mi.dwFlags  = MOUSEEVENTF_LEFTUP;								// We are setting left mouse button up.
	SendInput( 1, &Input, sizeof(INPUT) );								// Send the input.
}


void LeftClickCount( int N )
{  
	assert( N > 0 );
	INPUT    *InputList = (INPUT*)malloc( sizeof( INPUT ) * N * 2 );
	memset( InputList, 0, sizeof( INPUT ) * N * 2 );
	for( int i=0;i<N*2;i+=2)
	{
		InputList[i].type        = INPUT_MOUSE;									// Let input know we are using the mouse.
		InputList[i].mi.dwFlags  = MOUSEEVENTF_LEFTDOWN;							// We are setting left mouse button down.
		InputList[i+1].type        = INPUT_MOUSE;									// Let input know we are using the mouse.
		InputList[i+1].mi.dwFlags  = MOUSEEVENTF_LEFTUP;								// We are setting left mouse button up.
	}
	int Count = SendInput( N * 2, InputList, sizeof( INPUT ) );
	printf("Tried to send %d count of mouse events\n",Count);

}

void WindowSpecificLeftClick( HWND target, int x, int y )
{
//	PostMessage ( target, WM_LBUTTONDOWN, 0, ( x ) & ( ( y ) << 16 ) );
//	PostMessage ( target, WM_LBUTTONUP, 0, ( x ) & ( ( y ) << 16 ) );
	PostMessage ( target, WM_LBUTTONDOWN | MOUSE_MOVE_ABSOLUTE, 0, ( x ) & ( ( y ) << 16 ) );
	PostMessage ( target, WM_LBUTTONUP | MOUSE_MOVE_ABSOLUTE, 0, ( x ) & ( ( y ) << 16 ) );
}

void ClickerHeoresLoop()
{
	printf("waiting to click 100\n");
	_getch();
	int Start = GetTickCount();
	LeftClickCount( 10000 );
	int End = GetTickCount();
	printf( "10k click took %d. CPS %d \n", End - Start, 10000 / ( End - Start ) );
	_getch();

	//init
	ClickerHeroesStore CHS;
	//load images we will try to find 
	CHS.FrostLeaf.Load("ClickerHeroesFrostLeaf.bmp");
	PiramidImage FrostLeaf;
	FrostLeaf.BuildFromImg( &CHS.FrostLeaf );

	HWND WndPaintDst = GlobalData.WndPaintDst;
	HWND hWndSrc = FindWindowWithNameNonThreaded( "Mozilla" );
	while( 1 )
	{
		CScreenImage Screen;
		PiramidImage PScreen;

		Screen.CaptureWindow( hWndSrc );
		Screen.CaptureWindowConvert( hWndSrc );
		PScreen.BuildFromImg( &Screen );

		int x,y,rs,gs,bs;
		PiramidSearch( &PScreen, &FrostLeaf, &x, &y, &rs, &gs, &bs, 0 );

		SetCursorPos( x, y );
		Sleep( 10000 );
	};
}