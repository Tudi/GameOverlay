#ifndef _DRAW_OUTPUT_H_
#define _DRAW_OUTPUT_H_

void InitDrawThread();
void QueueOutputToDraw(CScreenImage			*Img);
bool IsDrawQueueEmpty();

#endif