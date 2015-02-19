#ifndef _RECOGNIZABLE_OBJECT_H_
#define _RECOGNIZABLE_OBJECT_H_

class RecognizableObject
{
public:
	RecognizableObject()
	{
		RecognizeMask = NULL;
	}

	//update our StampWhenLastChangeDetected
	void	CheckIfChangeDetected( MEImageDescRGB32 &In );

	unsigned char *RecognizeMask;	//an image marking what we should or should not inspect
	int Height,Width;
	int	MinSADR,MinSADG,MinSADB;	//this object has sad values in range of ....
	int	MaxSADR,MaxSADG,MaxSADB;	//this object has sad values in range of ....
	int StampWhenLastChangeDetected;
	int InspectStartX,InspectEndX,InspectStartY,InspectEndY;
};

#endif