#include "StdAfx.h"
#include <emmintrin.h>

__forceinline unsigned int PixelSadR( unsigned char *data0, unsigned char *data1 )
{
	return abs( (int)(data0[0]) - (int)(data1[0]) );
}

__forceinline unsigned int PixelSadG( unsigned char *data0, unsigned char *data1 )
{
	return abs( (int)(data0[1]) - (int)(data1[1]) );
}

__forceinline unsigned int PixelSadB( unsigned char *data0, unsigned char *data1 )
{
	return abs( (int)(data0[2]) - (int)(data1[2]) );
}


__forceinline unsigned int SAD_8byte( unsigned char *address1, unsigned char *address2 )
{	
	unsigned short sad_array[8];
	__m128i l0, l1, line_sad;

	l0 = _mm_loadl_epi64((__m128i*)address1);
	l1 = _mm_loadl_epi64((__m128i*)address2);
	line_sad = _mm_sad_epu8(l0, l1);
	_mm_storeu_si128((__m128i*)(&sad_array[0]), line_sad);

	return (sad_array[0]);
}

__forceinline unsigned int SAD_16byte( unsigned char *address1, unsigned char *address2 )
{	
	unsigned short sad_array[8];
	__m128i l0, l1, line_sad;

	l0 = _mm_loadu_si128((__m128i*)address1);
	l1 = _mm_loadu_si128((__m128i*)address2);
	line_sad = _mm_sad_epu8(l0, l1);
	_mm_storeu_si128((__m128i*)(&sad_array[0]), line_sad);

	return ( sad_array[0] + sad_array[4] );
}

__forceinline unsigned int SAD_16x4byte( unsigned char *address1, unsigned char *address2, int stride1, int stride2 )
{
	unsigned short sad_array[8];
	__m128i l0, l1, line_sad, acc_sad;

	acc_sad = _mm_setzero_si128();

	l0 = _mm_loadu_si128((__m128i*)address1);
	l1 = _mm_loadu_si128((__m128i*)address2);
	line_sad = _mm_sad_epu8(l0, l1);
	acc_sad = _mm_add_epi16(acc_sad, line_sad);

	l0 = _mm_loadu_si128((__m128i*)(address1+stride1));
	l1 = _mm_loadu_si128((__m128i*)(address2+stride2));
	line_sad = _mm_sad_epu8(l0, l1);
	acc_sad = _mm_add_epi16(acc_sad, line_sad);

	l0 = _mm_loadu_si128((__m128i*)(address1+stride1+stride1));
	l1 = _mm_loadu_si128((__m128i*)(address2+stride2+stride2));
	line_sad = _mm_sad_epu8(l0, l1);
	acc_sad = _mm_add_epi16(acc_sad, line_sad);

	l0 = _mm_loadu_si128((__m128i*)(address1+stride1+stride1+stride1));
	l1 = _mm_loadu_si128((__m128i*)(address2+stride2+stride2+stride2));
	line_sad = _mm_sad_epu8(l0, l1);
	acc_sad = _mm_add_epi16(acc_sad, line_sad);

	_mm_storeu_si128((__m128i*)(&sad_array[0]), acc_sad);

	return (sad_array[0]+sad_array[4]);
}

__forceinline unsigned int PixelSad( unsigned char *data0, unsigned char *data1 )
{
	return ( PixelSadR( data0, data1 ) + PixelSadG( data0, data1 ) + PixelSadB( data0, data1 ) );
}

unsigned int CompareAndMark( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg, MEImageDescRGB32 &OutImg )
{
	assert( prevImg.Data != NULL );
	assert( prevImg.EndX > prevImg.StartX );
	assert( CurImg.Data != NULL );
	assert( CurImg.EndX > CurImg.StartX );
	assert( CurImg.EndX - CurImg.StartX == prevImg.EndX - prevImg.StartX );

	unsigned int RRetSAD = 0, GRetSAD = 0, BRetSAD = 0;
	for( unsigned int Y = prevImg.StartY; Y < prevImg.EndY; Y++ )
	{
		unsigned char *Data1 = prevImg.Data + prevImg.StartX + Y * prevImg.Stride;
		unsigned char *Data1End = Data1 + ( prevImg.EndX - prevImg.StartX ) * 4;
		unsigned char *Data2 = CurImg.Data + CurImg.StartX + Y * CurImg.Stride;
		unsigned char *Data3 = OutImg.Data + OutImg.StartX + Y * OutImg.Stride;
		for( ; Data1 < Data1End; Data1 += 4 )
		{
			unsigned int RRetSADNow = PixelSadR( Data1, Data2 );
			unsigned int GRetSADNow = PixelSadG( Data1, Data2 );
			unsigned int BRetSADNow = PixelSadB( Data1, Data2 );

			RRetSAD += RRetSADNow;
			GRetSAD += GRetSADNow;
			BRetSAD += BRetSADNow;
			Data2 += 4;

			Data3[0] = RRetSADNow;
			Data3[1] = GRetSADNow;
			Data3[2] = BRetSADNow;
			Data3 += 4;
		}
	}
	return RRetSAD + GRetSAD + BRetSAD;
}

unsigned int GetImgSAD( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg )
{
	assert( prevImg.Data != NULL );
	assert( prevImg.EndX > prevImg.StartX );
	assert( CurImg.Data != NULL );
	assert( CurImg.EndX > CurImg.StartX );
	assert( CurImg.EndX - CurImg.StartX == prevImg.EndX - prevImg.StartX );

	unsigned int RRetSAD = 0, GRetSAD = 0, BRetSAD = 0;
	for( unsigned int Y = prevImg.StartY; Y < prevImg.EndY; Y++ )
	{
		unsigned char *Data1 = prevImg.Data + prevImg.StartX + Y * prevImg.Stride;
		unsigned char *Data1End = Data1 + ( prevImg.EndX - prevImg.StartX ) * 4;
		unsigned char *Data2 = CurImg.Data + CurImg.StartX + Y * CurImg.Stride;
		for( ; Data1 < Data1End; Data1 += 4 )
		{
			unsigned int RRetSADNow = PixelSadR( Data1, Data2 );
			unsigned int GRetSADNow = PixelSadG( Data1, Data2 );
			unsigned int BRetSADNow = PixelSadB( Data1, Data2 );

			RRetSAD += RRetSADNow;
			GRetSAD += GRetSADNow;
			BRetSAD += BRetSADNow;
			Data2 += 4;
		}
	}
	return RRetSAD + GRetSAD + BRetSAD;
}

unsigned int GetImgSADIntrinsic( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg )
{
	assert( prevImg.Data != NULL );
	assert( prevImg.EndX > prevImg.StartX );
	assert( CurImg.Data != NULL );
	assert( CurImg.EndX > CurImg.StartX );
	assert( CurImg.EndX - CurImg.StartX == prevImg.EndX - prevImg.StartX );

	unsigned int RetSAD = 0;
	for( unsigned int Y = prevImg.StartY; Y < prevImg.EndY; Y++ )
	{
		unsigned char *Data1 = prevImg.Data + prevImg.StartX + Y * prevImg.Stride;
		unsigned char *Data1End = Data1 + ( prevImg.EndX - prevImg.StartX ) * 4;
//		Data1End = Data1End & (unsigned char *)(~15); //can not do less than 16 bytes. Byte off remaining data
		unsigned char *Data2 = CurImg.Data + CurImg.StartX + Y * CurImg.Stride;
		for( ; Data1 < Data1End; Data1 += 16 )
		{
			int IntrinsicSad = SAD_16byte( Data1, Data2 );
			RetSAD += IntrinsicSad;
/*			unsigned int RRetSADNow = PixelSadR( Data1, Data2 );
			unsigned int GRetSADNow = PixelSadG( Data1, Data2 );
			unsigned int BRetSADNow = PixelSadB( Data1, Data2 );

			if( RRetSADNow + GRetSADNow + BRetSADNow != 0 )
			{
				int IntrinsicSad = SAD_16byte( Data1, Data2 );
				IntrinsicSad = 1;
			} */
			Data2 += 16;
		}
	}
	return RetSAD;
}

//take a 4x4 pixel block, do SAD, write back sad to output image
//pixel sad = R sad + G Sad + B Sad
// 16 pixel sad = ( pixel(0,0) + pixel(0,1) + pixel(4,4) ) / 16
unsigned int CompareAndMarkIntrinsic4x( MEImageDescRGB32 &prevImg, MEImageDescRGB32 &CurImg, MEImageDescRGB32 &OutImg )
{
	assert( prevImg.Data != NULL );
	assert( prevImg.EndX > prevImg.StartX );
	assert( CurImg.Data != NULL );
	assert( CurImg.EndX > CurImg.StartX );
	assert( CurImg.EndX - CurImg.StartX == prevImg.EndX - prevImg.StartX );

	unsigned int RetSAD = 0;
	if( OutImg.Data != NULL )
	{
		for( unsigned int Y = prevImg.StartY; Y < prevImg.EndY - 4; Y += 4 )
		{
			unsigned char *Data1 = prevImg.Data + prevImg.StartX + Y * prevImg.Stride;
			unsigned char *Data1End = Data1 + ( prevImg.EndX - prevImg.StartX ) * 4;
			unsigned char *Data2 = CurImg.Data + CurImg.StartX + Y * CurImg.Stride;
			unsigned char *Data3 = OutImg.Data + OutImg.StartX + Y / 4 * OutImg.Stride;
			for( ; Data1 < Data1End; Data1 += 16 )
			{
				int IntrinsicSad = SAD_16x4byte( Data1, Data2, prevImg.Stride, CurImg.Stride );
				RetSAD += IntrinsicSad;

	//			if( IntrinsicSad != 0 )
				{
	//				IntrinsicSad = SAD_16x4byte( Data1, Data2, prevImg.Stride, CurImg.Stride );
				}

				Data2 += 16;

				Data3[0] = IntrinsicSad / ( 4 * 4 );
				Data3 += 1;
			}
		}
	}
	else
	{
		for( unsigned int Y = prevImg.StartY; Y < prevImg.EndY - 4; Y += 4 )
		{
			unsigned char *Data1 = prevImg.Data + prevImg.StartX + Y * prevImg.Stride;
			unsigned char *Data1End = Data1 + ( prevImg.EndX - prevImg.StartX ) * 4;
			unsigned char *Data2 = CurImg.Data + CurImg.StartX + Y * CurImg.Stride;
			for( ; Data1 < Data1End; Data1 += 16 )
			{
				int IntrinsicSad = SAD_16x4byte( Data1, Data2, prevImg.Stride, CurImg.Stride );
				RetSAD += IntrinsicSad;
				Data2 += 16;
			}
		}
	}
	return RetSAD;
}