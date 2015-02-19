#include "StdAfx.h"

void InitOutputToInput( CScreenImage **out, HWND hWndSrc )
{
	if( *out )
		delete *out;
	*out = new CScreenImage;
	(*out)->CaptureWindow( hWndSrc );
	(*out)->CaptureWindowConvert( hWndSrc );
}

void TrackAndMarkObjectMotion()
{
	HWND WndPaintDst = GlobalData.WndPaintDst;

	HWND hWndSrc = FindWindowWithNameNonThreaded( "Mozilla" );

#define TRACK_IMAGE_COUNT	3
	CScreenImage	*Screens[TRACK_IMAGE_COUNT];
	CScreenImage	*OutputImage = NULL;

	int CurScreenIndex = TRACK_IMAGE_COUNT - 1, NextScreenIndex = 0;
	int CapturesTaken = 0;
	unsigned int BiggestSAD = 0;

	unsigned char	*ActiveBuffer = NULL;
	unsigned char	*NeglectBuffer = NULL;	//to try to neglect simple texture changes like wall illumination
	unsigned char	*SADBuffer = NULL;
	unsigned char	*ErodeBuffer = NULL;
	unsigned char	*EdgeBuffer = NULL;
	int				ErodeWidth,ErodeHeight;

	memset( Screens, NULL, sizeof( Screens ) );

	while( 1 )
	{
		int RefScreenIndex = CurScreenIndex;
		CurScreenIndex = NextScreenIndex;
		NextScreenIndex = ( NextScreenIndex + 1 ) % TRACK_IMAGE_COUNT;

		unsigned int StartStamp = GetTickCount();

		InitOutputToInput( &Screens[ CurScreenIndex ], hWndSrc );

		CScreenImage *PrevScreen = Screens[ RefScreenIndex ];
		CScreenImage *CurScreen = Screens[ CurScreenIndex ];

		//resizing test
		{
			MEImageDescRGB32 ResizeDesc;
			ResizeDesc.PixelByteCount = CurScreen->ActiveImagePixelByteCount;
			ResizeDesc.StartX = 0;
			ResizeDesc.EndX = 320;
			ResizeDesc.StartY = 0;
			ResizeDesc.EndY = 240;
			ResizeDesc.Stride = ResizeDesc.EndX * ResizeDesc.PixelByteCount;
			ResizeDesc.Data = (unsigned char*)CurScreen->ActiveRGB4ByteImageBuff;
			CurScreen->Resample( ResizeDesc );
		}/**/


		CapturesTaken++;
		if( PrevScreen == NULL || CurScreen == NULL )
		{
			printf("Painting initial screencap as we do not have enough buffers to process\n");
			PainBltImgToWnd( WndPaintDst, CurScreen );
			continue;
		}

		if( OutputImage == NULL || CurScreen->bi.biWidth != PrevScreen->bi.biWidth || CurScreen->bi.biHeight != PrevScreen->bi.biHeight )
		{
			printf( "image sizes changed, skipping compare %d %d %d,%d -> %d,%d \n", CurScreenIndex, RefScreenIndex, CurScreen->bi.biWidth, CurScreen->bi.biHeight, PrevScreen->bi.biWidth, PrevScreen->bi.biHeight );
			InitOutputToInput( &OutputImage, hWndSrc );

			OutputImage->ActiveImageWidth = CurScreen->ActiveImageWidth;
			OutputImage->ActiveImageHeight = CurScreen->ActiveImageHeight;
			OutputImage->ActiveImageStride = CurScreen->ActiveImageStride;
			OutputImage->ActiveImagePixelByteCount = CurScreen->ActiveImagePixelByteCount;

			if( OutputImage->ActiveImagePixelByteCount != 1 )
			{
				OutputImage->ActiveImageWidth = OutputImage->ActiveImageWidth / 4;
				OutputImage->ActiveImageStride = OutputImage->ActiveImageStride / 4;
				OutputImage->ActiveImageHeight = OutputImage->ActiveImageHeight / 4;
				OutputImage->ActiveImagePixelByteCount = 1;
			}

			if( ErodeBuffer != NULL && ( ErodeWidth != OutputImage->ActiveImageWidth || ErodeHeight != OutputImage->ActiveImageHeight ) )
			{
				free( ErodeBuffer );
				free( SADBuffer );
				free( NeglectBuffer );
				free( EdgeBuffer );
				ErodeBuffer = NULL;
			}

			if( ErodeBuffer == NULL )
			{
				ErodeWidth = OutputImage->ActiveImageWidth;
				ErodeHeight = OutputImage->ActiveImageHeight;
				ErodeBuffer = (unsigned char*)malloc( ( CurScreen->ActiveImageStride + 1 ) * ( CurScreen->ActiveImageHeight + 1 ) );
				SADBuffer = (unsigned char*)malloc( ( CurScreen->ActiveImageStride + 1 ) * ( CurScreen->ActiveImageHeight + 1 ) );
				NeglectBuffer = (unsigned char*)malloc( ( CurScreen->ActiveImageStride + 1 ) * ( CurScreen->ActiveImageHeight + 1 ) );
				EdgeBuffer = (unsigned char*)malloc( ( CurScreen->ActiveImageStride + 1 ) * ( CurScreen->ActiveImageHeight + 1 ) );

				memset( ErodeBuffer, 0, ( OutputImage->ActiveImageStride + 1 ) * ( ErodeHeight + 1 ) );
				memset( SADBuffer, 0, ( OutputImage->ActiveImageStride + 1 ) * ( ErodeHeight + 1 ) );
				memset( NeglectBuffer, 0, ( OutputImage->ActiveImageStride + 1 ) * ( ErodeHeight + 1 ) );
				memset( EdgeBuffer, 0, ( OutputImage->ActiveImageStride + 1 ) * ( ErodeHeight + 1 ) );

				OutputImage->ActiveRGB4ByteImageBuff = SADBuffer;
			}

			continue;
		}

		//restore it in case we messed up
//		OutputImage->ActiveRGB4ByteImageBuff = OutputImage->lpbitmap;

		MEImageDescRGB32 CurDesc,PrevDesk,OutDesk;
		CurDesc.StartX = 0;
		CurDesc.EndX = CurScreen->ActiveImageWidth;
		CurDesc.StartY = 0;
		CurDesc.EndY = CurScreen->ActiveImageHeight;
		CurDesc.Stride = CurScreen->ActiveImageStride;
		CurDesc.Data = (unsigned char*)CurScreen->ActiveRGB4ByteImageBuff;
		CurDesc.PixelByteCount = CurScreen->ActiveImagePixelByteCount;

		PrevDesk.StartX = 0;
		PrevDesk.EndX = PrevScreen->ActiveImageWidth;
		PrevDesk.StartY = 0;
		PrevDesk.EndY = PrevScreen->ActiveImageHeight;
		PrevDesk.Stride = PrevScreen->ActiveImageStride;
		PrevDesk.Data = (unsigned char*)PrevScreen->ActiveRGB4ByteImageBuff;
		PrevDesk.PixelByteCount = CurScreen->ActiveImagePixelByteCount;

		OutDesk.StartX = 0;
		OutDesk.EndX = OutputImage->ActiveImageWidth;
		OutDesk.StartY = 0;
		OutDesk.EndY = OutputImage->ActiveImageHeight;
		OutDesk.Stride = OutputImage->ActiveImageStride;
		OutDesk.Data = (unsigned char*)SADBuffer;

		unsigned int EndStampPreSAD = GetTickCount();
		float FPSAquire = 1000.0f / ( EndStampPreSAD - StartStamp );
		unsigned int CurSAD = CompareAndMarkIntrinsic4x( CurDesc, PrevDesk, OutDesk );
		ActiveBuffer = OutDesk.Data;

		//detect moving object
		if( CurSAD > 0 )
		{
			MEImageDescRGB32 ErodeIn,ErodeOut;
			int ErodeRadius = 3;
			
			ErodeIn.ErodeRadius = ErodeRadius;
			ErodeIn.ErodeLimit = ( 2 * ErodeRadius + 1 ) * ( 2 * ErodeRadius + 1 ) / 2;

			ErodeIn.StartX = ErodeRadius;
			ErodeIn.EndX = OutputImage->ActiveImageWidth - ErodeRadius;
			ErodeIn.StartY = ErodeRadius;
			ErodeIn.EndY = OutputImage->ActiveImageHeight - ErodeRadius;
			ErodeIn.Stride = OutputImage->ActiveImageStride;
			ErodeIn.Data = (unsigned char*)ActiveBuffer;

			ErodeOut.StartX = 0;
			ErodeOut.EndX = OutputImage->ActiveImageWidth;
			ErodeOut.StartY = 0;
			ErodeOut.EndY = OutputImage->ActiveImageHeight;
			ErodeOut.Stride = OutputImage->ActiveImageStride;
			ErodeOut.Data = (unsigned char*)ErodeBuffer;

			DetectBoxOfMotion( ErodeIn, ErodeOut );

			ActiveBuffer = ErodeOut.Data;

/*			printf(" %d %d %d %d\n",ErodeOut.StartX,ErodeOut.EndX,ErodeOut.StartY,ErodeOut.EndY );
			printf(" %d %d \n",( ErodeOut.EndX - ErodeOut.StartX ),( ErodeOut.EndY - ErodeOut.StartY ) );
			printf(" %d %d | %d %d\n", ( ErodeOut.EndX - ErodeOut.StartX ) * 120 / 100, ( ErodeIn.EndX - ErodeIn.StartX ), ( ErodeOut.EndY - ErodeOut.StartY ) * 120 / 100, ( ErodeIn.EndY - ErodeIn.StartY ) );

			printf(" %d %d \n",ErodeOut.StartX > ErodeIn.StartX, ErodeOut.EndX < ErodeIn.EndX );
			printf(" %d %d \n",ErodeOut.StartY > ErodeIn.StartY, ErodeOut.EndY < ErodeIn.EndY );
			printf(" %d %d \n",ErodeOut.StartY, ErodeIn.StartY );
			printf(" %d %d \n",( ErodeOut.EndX - ErodeOut.StartX ) * 120 / 100 < ( ErodeIn.EndX - ErodeIn.StartX ), ( ErodeOut.EndY - ErodeOut.StartY ) * 120 / 100 < ( ErodeIn.EndY - ErodeIn.StartY ) );
*/
			int ChangeCountPCT = 0;
			if( ErodeOut.ErodeLimit > 0 ) 
				ChangeCountPCT = ErodeOut.ErodeLimit * 100 / ( ( ErodeIn.EndX - ErodeIn.StartX ) * ( ErodeIn.EndY - ErodeIn.StartY ) );
			printf( "PCT pixels changed %d\n", ChangeCountPCT);
			if( ErodeOut.StartX > ErodeIn.StartX && ErodeOut.EndX < ErodeIn.EndX 
				&& ErodeOut.StartY > ErodeIn.StartY && ErodeOut.EndY < ErodeIn.EndY 
				&& ( ErodeOut.EndX - ErodeOut.StartX ) * 120 / 100 < ( ErodeIn.EndX - ErodeIn.StartX )
				&& ( ErodeOut.EndY - ErodeOut.StartY ) * 120 / 100 < ( ErodeIn.EndY - ErodeIn.StartY )
				&& ChangeCountPCT < 5 && ChangeCountPCT > 0
				)
			{
				//generate edge map
/*				MEImageDescRGB32 EdgeDesk;
				EdgeDesk.StartX = 0;
				EdgeDesk.EndX = CurScreen->ActiveImageWidth;
				EdgeDesk.StartY = 0;
				EdgeDesk.EndY = CurScreen->ActiveImageHeight;
				EdgeDesk.Stride = CurScreen->ActiveImageWidth;
				EdgeDesk.Data = (unsigned char*)EdgeBuffer;
				EdgeDesk.PixelByteCount = 1;
				GenerateEdgeMapRobertCross( CurDesc, EdgeDesk ); */

				MEImageDescRGB32 ErodeMask;
				ErodeMask.StartX = 0;
				ErodeMask.EndX = OutputImage->ActiveImageWidth;
				ErodeMask.StartY = 0;
				ErodeMask.EndY = OutputImage->ActiveImageHeight;
				ErodeMask.Stride = ErodeOut.Stride;
				ErodeMask.Data = (unsigned char*)ErodeOut.Data;

//				SaveImageToFile( &ErodeOut, NULL, "DetectedSaves\\MoveDetected" );
//				SaveImageToFile( &ErodeIn, &ErodeMask, "DetectedSaves\\MoveDetected" );
//				SaveImageToFile( &CurDesc, NULL, "DetectedSaves\\MoveDetected", true, RGB_CHAR_DATA_TYPE );
				SaveImageToFile( &CurDesc, &ErodeMask, "DetectedSaves\\MoveDetected", true, RGB_CHAR_DATA_TYPE );
//				SaveImageToFile( CurScreen->MEDesc, NULL, "DetectedSaves\\MoveDetected", true, RGB_CHAR_DATA_TYPE );
//				SaveImageToFile( CurScreen->MEDesc, &ErodeMask, "DetectedSaves\\MoveDetected", true, RGB_CHAR_DATA_TYPE );
//				SaveImageToFile( &EdgeDesk, NULL, "DetectedSaves\\MoveDetected", true, CHAR_CHAR_DATA_TYPE );
				printf("Saving screenshots of moving objects\n");
				while( GlobalData.InProgressDrawImage != 0 )
					Sleep( 1 );
				CurScreen->WriteOurBitmapToDC( 1, 320, 0, OutputImage );
//				PainBltImgToWnd( WndPaintDst, OutputImage );
			}

			OutputImage->ActiveRGB4ByteImageBuff = ErodeBuffer;
		}/**/

		if( CurSAD > 0 )
		{
			QueueOutputToDraw( OutputImage );
			unsigned int EndStamp = GetTickCount();
			printf("Ghosting image changes %d. Processing FPS %f %f\n", CurSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
		}
		else
		{
			unsigned int EndStamp = GetTickCount();
			printf("Nochange detected. Skipping paint. Processing FPS %f %f\n", CurSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
		}
		printf("\n");
	}
}

#if 0
void TrackAndMarkObjectMotion()
{
//	HWND WndPaintDst = FindMyTopMostWindow();
	HWND WndPaintDst = GlobalData.WndPaintDst;

//	HWND hWndSrc = FindWindowWithNameNonThreaded( "Studio" );
//	HWND hWndSrc = FindWindowWithNameNonThreaded( "Clicker" ); //LOL this will find that small label that only has the text "clicker"...
	HWND hWndSrc = FindWindowWithNameNonThreaded( "Mozilla" );
//	char ActiveWindowTitle[ DEFAULT_BUFLEN ];
//	GetWindowText( hWndSrc, ActiveWindowTitle, DEFAULT_BUFLEN );

#define TRACK_IMAGE_COUNT	20
	CScreenImage	*Screens[TRACK_IMAGE_COUNT];
	CScreenImage	*OutputImage = NULL;
//	InitOutputToInput( &OutputImage, hWndSrc );

	int CurScreenIndex = TRACK_IMAGE_COUNT - 1, NextScreenIndex = 0;
	int CapturesTaken = 0;
	unsigned int BiggestSAD = 0;

	memset( Screens, NULL, sizeof( Screens ) );

	while( 1 )
	{
		int RefScreenIndex = CurScreenIndex;
		CurScreenIndex = NextScreenIndex;
		NextScreenIndex = ( NextScreenIndex + 1 ) % TRACK_IMAGE_COUNT;

		unsigned int StartStamp = GetTickCount();

		InitOutputToInput( &Screens[ CurScreenIndex ], hWndSrc );

		CScreenImage *PrevScreen = Screens[ RefScreenIndex ];
		CScreenImage *CurScreen = Screens[ CurScreenIndex ];

		//resizing test
		{
			MEImageDescRGB32 ResizeDesc;
			ResizeDesc.PixelByteCount = CurScreen->ActiveImagePixelByteCount;
			ResizeDesc.StartX = 0;
			ResizeDesc.EndX = 320;
			ResizeDesc.StartY = 0;
			ResizeDesc.EndY = 240;
			ResizeDesc.Stride = ResizeDesc.EndX * ResizeDesc.PixelByteCount;
			ResizeDesc.Data = (unsigned char*)CurScreen->ActiveRGB4ByteImageBuff;
			CurScreen->Resample( ResizeDesc );
		}/**/


		CapturesTaken++;
		if( PrevScreen == NULL || CurScreen == NULL )
			continue;

		if( OutputImage == NULL || CurScreen->bi.biWidth != PrevScreen->bi.biWidth || CurScreen->bi.biHeight != PrevScreen->bi.biHeight )
		{
			printf( "image sizes changed, skipping compare %d %d %d,%d -> %d,%d \n", CurScreenIndex, RefScreenIndex, CurScreen->bi.biWidth, CurScreen->bi.biHeight, PrevScreen->bi.biWidth, PrevScreen->bi.biHeight );
			InitOutputToInput( &OutputImage, hWndSrc );

			OutputImage->ActiveImageWidth = CurScreen->ActiveImageWidth;
			OutputImage->ActiveImageHeight = CurScreen->ActiveImageHeight;
			OutputImage->ActiveImageStride = CurScreen->ActiveImageStride;
			OutputImage->ActiveImagePixelByteCount = CurScreen->ActiveImagePixelByteCount;

			if( OutputImage->ActiveImagePixelByteCount != 1 )
			{
				OutputImage->ActiveImageWidth = OutputImage->ActiveImageWidth / 4;
				OutputImage->ActiveImageStride = OutputImage->ActiveImageStride / 4;
				OutputImage->ActiveImageHeight = OutputImage->ActiveImageHeight / 4;
				OutputImage->ActiveImagePixelByteCount = 1;
			}

			continue;
		}

		MEImageDescRGB32 CurDesc,PrevDesk,OutDesk;
		CurDesc.StartX = 0;
		CurDesc.EndX = CurScreen->ActiveImageWidth;
		CurDesc.StartY = 0;
		CurDesc.EndY = CurScreen->ActiveImageHeight;
		CurDesc.Stride = CurScreen->ActiveImageStride;
		CurDesc.Data = (unsigned char*)CurScreen->ActiveRGB4ByteImageBuff;

		PrevDesk.StartX = 0;
		PrevDesk.EndX = PrevScreen->ActiveImageWidth;
		PrevDesk.StartY = 0;
		PrevDesk.EndY = PrevScreen->ActiveImageHeight;
		PrevDesk.Stride = PrevScreen->ActiveImageStride;
		PrevDesk.Data = (unsigned char*)PrevScreen->ActiveRGB4ByteImageBuff;

		OutDesk.StartX = 0;
		OutDesk.EndX = OutputImage->ActiveImageWidth;
		OutDesk.StartY = 0;
		OutDesk.EndY = OutputImage->ActiveImageHeight;
		OutDesk.Stride = OutputImage->ActiveImageStride;

		//test resample. Not like we need it for tests, just test if it works and what are the effects of it

		if( IsDrawQueueEmpty() )
			OutDesk.Data = (unsigned char*)OutputImage->ActiveRGB4ByteImageBuff;
		else
			OutDesk.Data = NULL;

		unsigned int EndStampPreSAD = GetTickCount();
		float FPSAquire = 1000.0f / ( EndStampPreSAD - StartStamp );
		unsigned int CurSAD;
//		unsigned int CurSAD = GetImgSAD( CurDesc, PrevDesk );		//almost 0 overhead
//		unsigned int CurSAD = GetImgSADIntrinsic( CurDesc, PrevDesk );		//almost 50% timeshare in debug mode
//		unsigned int CurSAD = CompareAndMark( CurDesc, PrevDesk, OutDesk );	//fluctuates between 0-50% timeshare
		{
			CurSAD = CompareAndMarkIntrinsic4x( CurDesc, PrevDesk, OutDesk );
		}

		if( CurSAD > BiggestSAD )
		{
//			CompareAndMark( CurDesc, PrevDesk, OutDesk );
			BiggestSAD = CurSAD;
//			OutputImage->WriteOurBitmapToDC();
//			OutputImage->Write4xSADmapToDC();
//			PainBltImgToWnd( WndPaintDst, OutputImage );
			QueueOutputToDraw( OutputImage );
			unsigned int EndStamp = GetTickCount();
			printf("Found a new biggest SAD %d instead %d. Painting it. Processing FPS %f %f\n", CurSAD, BiggestSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
	//		_getch();
		}
		else if( BiggestSAD == 0 )
		{
			PainBltImgToWnd( WndPaintDst, CurScreen );
			unsigned int EndStamp = GetTickCount();
			printf("Cur Sad now %d. Processing FPS %f %f\n", CurSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
		}
		else if( CurSAD > 0 )
		{
//			OutputImage->Write4xSADmapToDC( 4 );
//			PainBltImgToWnd( WndPaintDst, OutputImage );
			QueueOutputToDraw( OutputImage );
//			OutputImage = NULL;
			unsigned int EndStamp = GetTickCount();
			printf("Ghosting image changes %d. Processing FPS %f %f\n", CurSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
		}
		else
		{
			unsigned int EndStamp = GetTickCount();
			printf("Cur Sad now %d. Skipping paint. Processing FPS %f %f\n", CurSAD, 1000.0f / ( EndStamp - StartStamp ), FPSAquire );
		}
		//avoid 100% CPU usage
//		Sleep( 1000 );
	}
}
#endif