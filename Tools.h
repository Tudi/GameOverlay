#ifndef _IMG_TOOLS_H_
#define _IMG_TOOLS_H_

void SearchImgOnScreen( CImage *Screen, CImage *srch, int *retx, int *rety, int *Sad );
void StupidImgCopy( CImage *Dest, CImage *Src, int DestX, int DestY );
int GetLocalSad( int *a, int *b, int stride1, int stride2, int w, int h );
int PiramidSearch( PiramidImage *Big, PiramidImage *Small, int *RetX, int *RetY, int *RSad, int *GSad, int *BSad, int SadLimit, int pLayer = -1, int AtX = -1, int AtY = -1, int RecCount = 0 );
void MakeConsoleTransparentAndActive( HWND hWnd );
void PainBltImgToWnd( HWND hWnd, CImage *img );
HWND FindMyTopMostWindow();
void CreateNewWindow( HWND owner, HWND &child );
void CreateNewWindow2( HWND owner, HWND &child );
HWND FindWindowWithNameNonThreaded( char *Name );
void GetDesktopResolution(int& horizontal, int& vertical);

#define COUNT_NUMBER_OF_SADS

#ifdef COUNT_NUMBER_OF_SADS
extern int SadCounter;
#endif

#endif