#include "StdAfx.h"

void DetectBoxOfMotion( MEImageDescRGB32 &In, MEImageDescRGB32 &Out )
{
	//errode small moving objects
	ErrodeSumSADMap( In, Out, In.ErodeRadius );

	unsigned int MinLeft = In.EndX;
	unsigned int MaxRight = In.StartX;
	unsigned int MinTop = In.EndY;
	unsigned int MaxBottom = In.StartY;
	unsigned int PixelCount = 0;

	// raster scan the input image and remember 
	for( unsigned int y = In.StartY; y < In.EndY; y +=1 )
		for( unsigned int x = In.StartX; x < In.EndX; x += 1 )
		{
			int PixelValue = Out.Data[ y * In.Stride + x ];
			if( PixelValue != 0 )
			{
				if( y > MaxBottom )
					MaxBottom = y;
				if( y < MinTop )
					MinTop = y;
				if( x > MaxRight )
					MaxRight = x;
				if( x < MinLeft )
					MinLeft = x;
				PixelCount++;
			}
		}
	Out.StartX = MinLeft;
	Out.EndX = MaxRight;
	Out.StartY = MinTop;
	Out.EndY = MaxBottom;
	Out.ErodeLimit = PixelCount;
}