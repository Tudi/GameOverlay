#ifndef _PIRAMID_IMAGE_SEARCH_H_
#define _PIRAMID_IMAGE_SEARCH_H_

//number of layers to be generated. Top Layer is 1 to 1
#define MAX_IMAGE_LAYERS		5
//number of pixels to be merged per layer. Ex : 2x2 pixel block becomes 1x1
#define PIXEL_STEPDOWN_LAYER	2
#define MIN_SIZE_FOR_SEARCH		1
#define MAX_INT					0x01FFFFFF

#define	GENERATE_ONLY_R
//#define DEBUG_WRITEBACK_WHAT_PIXELS_WE_READ
#define MERGE_RGB_INTO_R	//you want to use this in conjunction with GENERATE_ONLY_R

class PiramidImage
{
public:
	PiramidImage();
	~PiramidImage();
	void BuildFromImg( CImage *Src );
	void BuildFromImg( CScreenImage *Src );
	void BuildFromImgOtherLevels();
	void SaveLayersToFile( char *BaseName, int x = 0, int y = 0, int w = -1, int h = -1 );
//private:
	signed int	*ImageLayers[3][MAX_IMAGE_LAYERS];
	int			ImageLayersX[MAX_IMAGE_LAYERS];
	int			ImageLayersY[MAX_IMAGE_LAYERS];
};

#endif