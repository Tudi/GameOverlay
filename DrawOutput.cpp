#include "StdAfx.h"

bool IsDrawQueueEmpty()
{
	return ( GlobalData.InProgressDrawImage == NULL );
}

void QueueOutputToDraw(CScreenImage			*Img)
{
	if( GlobalData.InProgressDrawImage == NULL )
		GlobalData.InProgressDrawImage = Img;
	else
	{
//		delete Img;
	}
}

DWORD WINAPI DrawChangesOnOutputWindow( LPVOID lpParam )
{
    while( GlobalData.DrawThreadAlive == 1 ) 
	{
		if( GlobalData.InProgressDrawImage != NULL )
		{
			GlobalData.InProgressDrawImage->WriteOurBitmapToDC( 4 );
			PainBltImgToWnd( GlobalData.WndPaintDst, GlobalData.InProgressDrawImage );
//			delete GlobalData.InProgressDrawImage;
			GlobalData.InProgressDrawImage = NULL;
		}
		else
			Sleep( 10 );
	}
	return 0;
}

void InitDrawThread()
{
	DWORD   dwThreadId;
	HANDLE  hThread; 
		hThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			DrawChangesOnOutputWindow,			// thread function name
			0,						// argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);   // returns the thread identifier 
}
