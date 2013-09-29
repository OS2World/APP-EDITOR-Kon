#include "bowl.h"

HAB Application::hab;
HMQ Application::hmq;
AWindow *Application::tempthis = NULL;

BOOL Application::run(void) {
	BOOL rc;
	QMSG qmsg;
	while( 1 )
	{
		rc = (BOOL)WinGetMsg(hab,&qmsg,(HWND)NULL,0,0);
		WinDispatchMsg(hab,&qmsg);
		if( !rc ) break;
	}
	return rc;
}

void Application::processMessages()
{
  QMSG qmsg;
  while( WinPeekMsg( hab, &qmsg, (HWND)NULL, 0, 0, PM_REMOVE ) )
		WinDispatchMsg(hab,&qmsg);
}

int Application::executeFromMainThread( Window *mExec, int task, void *data )
{
  return (int)WinSendMsg( mExec->getHWND(), WM_USER+1, (MPARAM)task,
                          MPARAM(data) );
}

int Application::executeFromMainThreadPost( Window *mExec, int task, void *data )
{
  return (int)WinPostMsg( mExec->getHWND(), WM_USER+1, (MPARAM)task,
                          MPARAM(data) );
}

