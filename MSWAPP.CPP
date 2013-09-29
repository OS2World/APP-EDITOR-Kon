#include "bowl.h"

AWindow *Application::tempthis = NULL;
HINSTANCE Application::hInstance = NULL;

BOOL Application::initialize( HINSTANCE hInst ) {
	SystemParametersInfo( 0x100b/*SPI_SETMENUUNDERLINES*/, 0, (PVOID)TRUE, 0 );
  hInstance = hInst;
	return TRUE;
}

BOOL Application::run(void) {
	BOOL rc;
	MSG msg;
	while( 1 )
	{
		rc = (BOOL)GetMessage(&msg,(HWND)NULL,0,0);
		if( rc != TRUE) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return rc;
}

void Application::processMessages()
{
  MSG msg;
  while( PeekMessage(&msg,(HWND)NULL,0,0, PM_REMOVE) )
		DispatchMessage(&msg);
}

int Application::executeFromMainThread( Window *mExec, int task, void *data )
{
	return (int)SendMessage( mExec->getHWND(), WM_USER+1, (WPARAM)task,
			                      LPARAM(data) );
}

int Application::executeFromMainThreadPost( Window *mExec, int task, void *data )
{
	return (int)PostMessage( mExec->getHWND(), WM_USER+1, (WPARAM)task,
			                      LPARAM(data) );
}

