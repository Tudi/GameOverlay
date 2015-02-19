#ifndef _MOTION_ESTIMATION_H_
#define _MOTION_ESTIMATION_H_

//does not support Alpha channel( 4th byte )
struct MEImageDescRGB32
{
	unsigned int StartX,StartY;
	unsigned int EndX,EndY;
	unsigned int PixelByteCount;	//kinda hardcoded to 4
	unsigned int Stride;		// should be 3 * width or more. Given in byte count 
	unsigned char *Data;
	int		ErodeRadius,ErodeLimit;
};

unsigned int CompareAndMark( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg, MEImageDescRGB32 &outImg );
unsigned int GetImgSAD( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg );
unsigned int GetImgSADIntrinsic( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg );
unsigned int CompareAndMarkIntrinsic4x( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg, MEImageDescRGB32 &outImg );

#endif