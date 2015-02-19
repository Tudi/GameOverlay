#include "StdAfx.h"

#define SLEEP_BETWEEN_KEYPRESS	20

void SendKeyPress3( int code, int PressDownDelay, int ReleaseDelay )
{
	InterceptionKeyStroke kstroke;
	memset( &kstroke, 0, sizeof( kstroke ) );

	//you can push down a code and leave it like that
	if( PressDownDelay >= 0 )
	{
		kstroke.code = code;
		kstroke.state = 0;
		interception_send( GlobalData.InterceptoinContext, GlobalData.KeyboardDevice, (InterceptionStroke *)&kstroke, 1);
		if( PressDownDelay == 0 && ReleaseDelay >= 0 )
			Sleep( SLEEP_BETWEEN_KEYPRESS );
		else if( PressDownDelay > 0 )
			Sleep( PressDownDelay );
	}

	//you can push down a code and leave it like that
	if( ReleaseDelay >= 0 )
	{
		kstroke.code = code;
		kstroke.state = 1;	
		interception_send( GlobalData.InterceptoinContext, GlobalData.KeyboardDevice, (InterceptionStroke *)&kstroke, 1);
		if( ReleaseDelay > 0 )
			Sleep( ReleaseDelay );
	}
}

void SendMouseChange( int state, int flags, int Xchange, int Ychange, int PressDownDelay, int ReleaseDelay )
{
	InterceptionMouseStroke mstroke;
	memset( &mstroke, 0, sizeof( mstroke ) );

//printf( "sending mouse state change %d x %d y %d down %d up %d \n", state, Xchange, Ychange, PressDownDelay, ReleaseDelay );
	mstroke.state = state;
	mstroke.x = Xchange;
	mstroke.y = Ychange;
	if( flags == 0 )
		mstroke.flags = INTERCEPTION_MOUSE_MOVE_RELATIVE;
	else
		mstroke.flags = flags;

	if( PressDownDelay >= 0 )
	{
		interception_send( GlobalData.InterceptoinContext, GlobalData.MouseDevice, (InterceptionStroke *)&mstroke, 1);
		if( PressDownDelay == 0 && ReleaseDelay > 0 )
			Sleep( SLEEP_BETWEEN_KEYPRESS );
		else
			Sleep( PressDownDelay );
	}

	if( ReleaseDelay >= 0 )
	{
		if( state == INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN || state == INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN )
		{
			mstroke.state = 2 * state;
			mstroke.x = 0;
			mstroke.y = 0;
			interception_send( GlobalData.InterceptoinContext, GlobalData.MouseDevice, (InterceptionStroke *)&mstroke, 1);
			if( ReleaseDelay != 0 )
				Sleep( ReleaseDelay );
		}
	}
}

void InitKeyboadMouseFilterDriver()
{
	InterceptionDevice		LastDevice;
	InterceptionStroke		LastStroke;
	int		PrevKeyEventTick = GetTickCount();
	int		PrevMouseEventTick = GetTickCount();

	interception_set_filter( GlobalData.InterceptoinContext, interception_is_keyboard, INTERCEPTION_FILTER_KEY_DOWN | INTERCEPTION_FILTER_KEY_UP );
	interception_set_filter( GlobalData.InterceptoinContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_MOVE | INTERCEPTION_MOUSE_LEFT_BUTTON_DOWN | INTERCEPTION_MOUSE_LEFT_BUTTON_UP | INTERCEPTION_MOUSE_RIGHT_BUTTON_DOWN | INTERCEPTION_MOUSE_RIGHT_BUTTON_UP );
//		interception_set_filter( GlobalData.InterceptoinContext, interception_is_mouse, INTERCEPTION_FILTER_MOUSE_ALL );
	while( interception_receive( GlobalData.InterceptoinContext, LastDevice = interception_wait( GlobalData.InterceptoinContext ), (InterceptionStroke *)&LastStroke, 1) > 0
//		&& 0
		)
	{
		if( interception_is_mouse( LastDevice ) )
		{
			GlobalData.MouseDevice = LastDevice;
            InterceptionMouseStroke &mstroke = *(InterceptionMouseStroke *) &LastStroke;
			
			memcpy( &GlobalData.MouseStroke, &LastStroke, sizeof( InterceptionMouseStroke ) );

			//try to track mouse coordinate
 /*           if( (mstroke.flags & INTERCEPTION_MOUSE_MOVE_ABSOLUTE) == 0 )
			{
				//this can go off screen
				GlobalData.TrackedMouseX += mstroke.x;
				if( GlobalData.TrackedMouseX > GlobalData.DosBoxWidth )
					GlobalData.TrackedMouseX = GlobalData.DosBoxWidth;
				if( GlobalData.TrackedMouseX < 0 )
					GlobalData.TrackedMouseX = 0;
				GlobalData.TrackedMouseY += mstroke.y;
				if( GlobalData.TrackedMouseY > GlobalData.DosBoxHeight )
					GlobalData.TrackedMouseY = GlobalData.DosBoxHeight;
				if( GlobalData.TrackedMouseY < 0 )
					GlobalData.TrackedMouseY = 0;
			} */

/*			if( GlobalData.StartedRecording == 1 )
			{
				//track mouse positions and on click try to print it out
				GlobalData.TrackedMouseScriptX += mstroke.x;
				if( GlobalData.TrackedMouseScriptX > GlobalData.MouseXLimitMax )
					GlobalData.TrackedMouseScriptX = GlobalData.MouseXLimitMax;
				if( GlobalData.TrackedMouseScriptX < GlobalData.MouseXLimitMin )
					GlobalData.TrackedMouseScriptX = GlobalData.MouseXLimitMin;
				GlobalData.TrackedMouseScriptY += mstroke.y;
				if( GlobalData.TrackedMouseScriptY > GlobalData.MouseYLimitMax )
					GlobalData.TrackedMouseScriptY = GlobalData.MouseYLimitMax;
				if( GlobalData.TrackedMouseScriptY < GlobalData.MouseYLimitMin )
					GlobalData.TrackedMouseScriptY = GlobalData.MouseYLimitMin;

				if( mstroke.state != 0 )
				{
					int TimePassed = GetTickCount() - GlobalData.TrackedMouseScriptStamp;
					GlobalData.TrackedMouseScriptStamp = GetTickCount();
					POINT p;
					GetCursorPos( &p );
//					printf("Registering click at (%d,%d), Time spent %d. Win said %d,%d\n", GlobalData.TrackedMouseScriptX, GlobalData.TrackedMouseScriptY, TimePassed, p.x, p.y );
					printf("click (%d,%d) - %d.\n", p.x, p.y, TimePassed );
//					GlobalData.TrackedMouseScriptX = p.x;
//					GlobalData.TrackedMouseScriptY = p.y;
				}
			} */

            interception_send( GlobalData.InterceptoinContext, GlobalData.MouseDevice, (InterceptionStroke *)&mstroke, 1);

/*			if( GlobalData.PrintKeysPressed )
			{
//				printf( "Mouse %d: state %d, x %d y %d, flags %d, rolling %d, info %d, tracked %d %d\n", GetTickCount() - PrevKeyEventTick, mstroke.state, mstroke.x, mstroke.y, mstroke.flags, mstroke.rolling, mstroke.information, GlobalData.TrackedMouseX, GlobalData.TrackedMouseY );
				printf( "Mouse %d: state %d, x %d y %d, flags %d, rolling %d, info %d\n", GetTickCount() - PrevKeyEventTick, mstroke.state, mstroke.x, mstroke.y, mstroke.flags, mstroke.rolling, mstroke.information );
				PrevKeyEventTick = GetTickCount();
			} */
		}
		if( interception_is_keyboard( LastDevice ) )
		{
			GlobalData.KeyboardDevice = LastDevice;
            InterceptionKeyStroke &kstroke = *(InterceptionKeyStroke *) &LastStroke;

			memcpy( &GlobalData.KeyboardStroke, &LastStroke, sizeof( InterceptionKeyStroke ) );

			interception_send( GlobalData.InterceptoinContext, GlobalData.KeyboardDevice, (InterceptionStroke *)&LastStroke, 1);

/*			if( GlobalData.PrintKeysPressed )
			{
				printf( "Keyboard delay %d: scan code %d %d %d\n", GetTickCount() - PrevKeyEventTick, kstroke.code, kstroke.state, kstroke.information );
				PrevKeyEventTick = GetTickCount();
			}
			else if( kstroke.state == 0 ) //pushdown
			{
				HWND FW = GetForegroundWindow( );
				HWND CW = GetConsoleWindow();
				if( CW == FW )
					printf( "(%d)", kstroke.code );
			} */

/*			if( kstroke.code == GlobalData.PauseToggleKeyCode && kstroke.state == 0 )
			{
				GlobalData.PauseSendKeys = 1 - GlobalData.PauseSendKeys;
				printf( "KeySending thread pause state changed to %d\n", GlobalData.PauseSendKeys );
				std::list<IrcGameKeyStore*>::iterator itr;
				for( itr=GlobalData.MonitoredKeys.begin(); itr!=GlobalData.MonitoredKeys.end(); itr++ )
					if( (*itr)->PushInterval == ONE_TIME_PUSH_KEY_INTERVAL )
					{
						(*itr)->PushInterval = 0;
						(*itr)->LastPushStamp = GetTickCount();
					}

			} */

/*			if( kstroke.code == 10 && kstroke.state == 0 )
			{
				GlobalData.StartedRecording = 1 - GlobalData.StartedRecording;
				if( GlobalData.StartedRecording == 1 )
				{
					//bring mouse to 0 0
//					SendMouseChange( 0, 0, -10000, -10000, SLEEP_BETWEEN_KEYPRESS, SLEEP_BETWEEN_KEYPRESS );
//					SendMouseChange( 1, 0, -10000, -10000, SLEEP_BETWEEN_KEYPRESS, SLEEP_BETWEEN_KEYPRESS );
//					SendMouseChange( 1, 0, 0, 0, SLEEP_BETWEEN_KEYPRESS, SLEEP_BETWEEN_KEYPRESS );
					GlobalData.TrackedMouseScriptX = 0;
					GlobalData.TrackedMouseScriptY = 0;
					printf("Key 9 was pressed. Moved mouse to 0,0 so we can start recording mouse movement for scripts. Not accurate !\n");
				}
			}

			if( GlobalData.StartedRecording == 1 )
			{
				if( kstroke.state != 0 )
				{
					int TimePassed = GetTickCount() - GlobalData.TrackedMouseScriptStamp;
					GlobalData.TrackedMouseScriptStamp = GetTickCount();
					POINT p;
					GetCursorPos( &p );
					printf("click (%d,%d) - %d.\n", p.x, p.y, TimePassed );
				}
			}

			if( kstroke.code == SCANCODE_CONSOLE )
			{
				GlobalData.WorkerThreadAlive = 0;
				printf( "Esc pressed. Shutting down\n" );
				break;
			} */
		}
	}/**/

	interception_destroy_context( GlobalData.InterceptoinContext );
}