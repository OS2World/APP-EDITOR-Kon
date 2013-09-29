class Window;

class Application : public AApplication
{
public:
	static AWindow   *tempthis;
	static HINSTANCE hInstance;

  static BOOL initialize( HINSTANCE hInst );

	static HINSTANCE getInstance() { return hInstance; }

	static BOOL run(void);

  static void processMessages();

	static void terminate(void) {
		 PostQuitMessage( 0 );
	}

  static int executeFromMainThread( Window *mExec, int task, void *data );
  static int executeFromMainThreadPost( Window *mExec, int task, void *data );
};


