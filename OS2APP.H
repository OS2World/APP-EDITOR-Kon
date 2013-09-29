class Window;

class Application : public AApplication
{
public:
	static HAB      hab;
	static HMQ      hmq;
	static AWindow   *tempthis;

  static BOOL initialize(void) {
		if ((hab = WinInitialize(0)) == (HAB)NULL) return FALSE;
		if ((hmq = WinCreateMsgQueue(hab,0)) == (HMQ)NULL) return FALSE;
		return TRUE;
	}

	static BOOL run(void);

  static void processMessages();

	static void terminate(void) {
    WinPostQueueMsg( hmq, WM_QUIT, NULL, NULL );
	}

  static int executeFromMainThread( Window *mExec, int task, void *data );
  static int executeFromMainThreadPost( Window *mExec, int task, void *data );
};


