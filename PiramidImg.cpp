#include "StdAfx.h"

PiramidImage::PiramidImage()
{
	memset( ImageLayers, NULL, sizeof( ImageLayers ) );
}

PiramidImage::~PiramidImage()
{
	for( int i=0;i<MAX_IMAGE_LAYERS;i++ )
		if( ImageLayers[0][i] )
		{
			free( ImageLayers[0][i] );
			free( ImageLayers[1][i] );
			free( ImageLayers[2][i] );
		}
	memset( ImageLayers, NULL, sizeof( ImageLayers ) );
}

void PiramidImage::BuildFromImg( CScreenImage *Src )
{
//	BuildFromImg( Src, Src->lpbitmap );
	if( Src->lpbitmap == NULL )
	{
		assert( false );
		return;
	}

	//alloc layers
	ImageLayersX[0] = Src->bi.biWidth;
	ImageLayersY[0] = Src->bi.biHeight;
	for( int i=0;i<MAX_IMAGE_LAYERS;i++ )
		if( ImageLayers[0][i] == NULL )
		{
			if( i > 0 )
			{
				ImageLayersX[ i ] = ( ImageLayersX[ i - 1 ] ) / PIXEL_STEPDOWN_LAYER;
				ImageLayersY[ i ] = ( ImageLayersY[ i - 1 ] ) / PIXEL_STEPDOWN_LAYER;
			}
			if( ImageLayersX[ i ] <= MIN_SIZE_FOR_SEARCH || ImageLayersY[ i ] <= MIN_SIZE_FOR_SEARCH )
				continue;

			ImageLayers[0][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
#if !defined( GENERATE_ONLY_R ) && !defined( MERGE_RGB_INTO_R )
			ImageLayers[1][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
			ImageLayers[2][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
#endif
		}

	//copy the bitmap to our buffer
	BYTE* byteptr = (unsigned char*)Src->lpbitmap;
	int stride = ( ImageLayersX[ 0 ] * ( 32 / 8) + 3) & ~3;
	for( int y = 0; y < ImageLayersY[ 0 ]; y += 1 )
	{
		int MirrorY = ImageLayersY[ 0 ] - 1 - y;
	    int rowBase = y * stride;
		for( int x = 0; x < ImageLayersX[ 0 ]; x += 1 )
		{
			//pointer arithmetics to find (i,j) pixel colors:
			int R = *( byteptr + rowBase + x * 4 + 2 );
			ImageLayers[0][0][ MirrorY * ImageLayersX[ 0 ] + x ] = R;
#ifdef MERGE_RGB_INTO_R
			int G = *( byteptr + rowBase + x * 4 + 1 );
			int B = *( byteptr + rowBase + x * 4 + 0 ); 
			ImageLayers[0][0][ MirrorY * ImageLayersX[ 0 ] + x ] += G;
			ImageLayers[0][0][ MirrorY * ImageLayersX[ 0 ] + x ] += B;
#endif
#ifndef GENERATE_ONLY_R
			int G = *( byteptr + rowBase + x * 4 + 1 );
			int B = *( byteptr + rowBase + x * 4 + 0 ); 
			ImageLayers[1][0][ MirrorY * ImageLayersX[ 0 ] + x ] = G;
			ImageLayers[2][0][ MirrorY * ImageLayersX[ 0 ] + x ] = B;
#endif
#ifdef DEBUG_WRITEBACK_WHAT_PIXELS_WE_READ
			Src->SetPixel( x, MirrorY, RGB( R, G, B ) );
#endif
		}
	}

	BuildFromImgOtherLevels();
}

void PiramidImage::BuildFromImg( CImage *Src )
{
	//aloc layers
	ImageLayersX[0] = Src->GetWidth();
	ImageLayersY[0] = Src->GetHeight();
	for( int i=0;i<MAX_IMAGE_LAYERS;i++ )
		if( ImageLayers[0][i] == NULL )
		{
			if( i > 0 )
			{
				ImageLayersX[ i ] = ( ImageLayersX[ i - 1 ] ) / PIXEL_STEPDOWN_LAYER;
				ImageLayersY[ i ] = ( ImageLayersY[ i - 1 ] ) / PIXEL_STEPDOWN_LAYER;
			}
			if( ImageLayersX[ i ] <= MIN_SIZE_FOR_SEARCH || ImageLayersY[ i ] <= MIN_SIZE_FOR_SEARCH )
				continue;

			ImageLayers[0][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
#if !defined( GENERATE_ONLY_R ) && !defined( MERGE_RGB_INTO_R )
			ImageLayers[1][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
			ImageLayers[2][i] = (int*)malloc( ImageLayersX[i] * ImageLayersY[i] * sizeof( signed int ) );
#endif
		}
/*
	//this almost works, somehow the image is mirrored on one axis... Not goot
	// Considering this is only used for load image it is not urgent to fix
	if( Src->IsDIBSection() )
	{
		BYTE* byteptr = (BYTE*)Src->GetBits();
		int pitch = Src->GetPitch(); //This is a pointer offset to get new line of the bitmap
		for( int y = 0; y < ImageLayersY[ 0 ]; y += 1 )
			for( int x = 0; x < ImageLayersX[ 0 ]; x += 1 )
			{
				//pointer arithmetics to find (i,j) pixel colors:
				int R= *( byteptr + pitch * x + 3 * y + 0 );
				int G= *( byteptr + pitch * x + 3 * y + 1 );
				int B= *( byteptr + pitch * x + 3 * y + 2 ); 
				ImageLayers[0][0][ y * ImageLayersX[ 0 ] + x ] = R;
				ImageLayers[1][0][ y * ImageLayersX[ 0 ] + x ] = G;
				ImageLayers[2][0][ y * ImageLayersX[ 0 ] + x ] = B;
Src->SetPixel( x, y, RGB( R,G,B ) );
			}
	}
	else
/**/

	{
		for( int y = 0; y < ImageLayersY[ 0 ]; y += 1 )
			for( int x = 0; x < ImageLayersX[ 0 ]; x += 1 )
			{
				COLORREF SrchCol = Src->GetPixel( x, y );
				DWORD R = ( SrchCol & 0x000000FF ) >> 0;
				ImageLayers[0][0][ y * ImageLayersX[ 0 ] + x ] = R;
#ifdef MERGE_RGB_INTO_R
				DWORD G = ( SrchCol & 0x0000FF00 ) >> 8;
				DWORD B = ( SrchCol & 0x00FF0000 ) >> 16;
				ImageLayers[0][0][ y * ImageLayersX[ 0 ] + x ] += G;
				ImageLayers[0][0][ y * ImageLayersX[ 0 ] + x ] += B;
#endif
#ifndef GENERATE_ONLY_R
				DWORD G = ( SrchCol & 0x0000FF00 ) >> 8;
				DWORD B = ( SrchCol & 0x00FF0000 ) >> 16;
				ImageLayers[1][0][ y * ImageLayersX[ 0 ] + x ] = G;
				ImageLayers[2][0][ y * ImageLayersX[ 0 ] + x ] = B;
#endif
#ifdef DEBUG_WRITEBACK_WHAT_PIXELS_WE_READ
				Src->SetPixel( x, y, RGB( R, G, B ) );
#endif
			}
	}
/**/

	BuildFromImgOtherLevels();
}

void PiramidImage::BuildFromImgOtherLevels()
{
	for( int Layer=1;Layer<MAX_IMAGE_LAYERS;Layer++)
	{
		int BigerLayer = Layer - 1;

		//image too small to scale it even more down
		if( ImageLayersX[ Layer ] <= MIN_SIZE_FOR_SEARCH || ImageLayersY[ Layer ] <= MIN_SIZE_FOR_SEARCH )
			break;

		int RGB = 0;
#if !defined( GENERATE_ONLY_R ) && !defined( MERGE_RGB_INTO_R )
		for( ; RGB < 3 ; RGB++ )
#endif
		{
			for( int y=0;y<ImageLayersY[ Layer ];y++)
				for( int x=0;x<ImageLayersX[ Layer ];x++)
				{
					int Sum = 0;
					int *BigImg = &ImageLayers[ RGB ][ BigerLayer ][ y * PIXEL_STEPDOWN_LAYER * ImageLayersX[ BigerLayer ] + x * PIXEL_STEPDOWN_LAYER ];
					for( int y1=0;y1<PIXEL_STEPDOWN_LAYER;y1++)
						for( int x1=0;x1<PIXEL_STEPDOWN_LAYER;x1++)
						{
							int SumPrevLayer = BigImg[ y1 * ImageLayersX[ BigerLayer ] + x1 ];
							Sum += SumPrevLayer;
						}
//assert( y / PIXEL_STEPDOWN_LAYER < ImageLayersY[ Layer ] );
//assert( x / PIXEL_STEPDOWN_LAYER < ImageLayersX[ Layer ] );
					ImageLayers[ RGB ][ Layer ][ y * ImageLayersX[ Layer ] + x ] = Sum;
				}
		}
	}
}

void PiramidImage::SaveLayersToFile( char *BaseName, int x, int y, int w, int h )
{
	for( int Layer=0;Layer<MAX_IMAGE_LAYERS;Layer++)
	{
		if( ImageLayersY[Layer] <= MIN_SIZE_FOR_SEARCH || ImageLayersX[Layer] <= MIN_SIZE_FOR_SEARCH )
			continue;

		int PixelsInPixel = 1;
		int PixelsInPixel1D = 1;
		for( int i = 0; i < Layer; i++ )
		{
			PixelsInPixel *= PIXEL_STEPDOWN_LAYER * PIXEL_STEPDOWN_LAYER;
			PixelsInPixel1D *= PIXEL_STEPDOWN_LAYER;
		}

		int width = ImageLayersX[Layer];
		int height = ImageLayersY[Layer];

		int StartX = x / PixelsInPixel1D;
		int StartY = y / PixelsInPixel1D;
		if( w != -1 )
		{
			width = min( w / PixelsInPixel1D, width );
			height = min( h / PixelsInPixel1D, height );
		}

		CImage img;
		img.Create( width + 1, height + 1, 24 /* bpp */, 0 /* No alpha channel */);

		for(int row = StartY; row < StartY + height; row++)
		{
			int nPixel = row * ImageLayersX[Layer] + StartX;
			for(int col = StartX; col < StartX + width; col++)
			{
				BYTE r = ImageLayers[0][Layer][nPixel] / PixelsInPixel;
#ifdef MERGE_RGB_INTO_R
				r = r / 3;
#endif
#ifndef GENERATE_ONLY_R
				BYTE g = ImageLayers[1][Layer][nPixel] / PixelsInPixel;
				BYTE b = ImageLayers[2][Layer][nPixel] / PixelsInPixel;
				img.SetPixel( col - StartX, row - StartY, RGB( r, g, b ) );
#else
				img.SetPixel( col - StartX, row - StartY, RGB( r, r, r ) );
#endif
				nPixel++;
			}
		}
		char FileName[500];
		sprintf_s( FileName, 500, "%s%d.bmp", BaseName, Layer );
		img.Save( FileName );
	}
}