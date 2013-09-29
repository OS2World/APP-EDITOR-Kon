#if !defined AUTOSAVE

#include "bowl.h"

#define AUTOSAVE

class AutosaveThread : public Thread
{
	static AutosaveThread *instance;
public:
  AutosaveThread() : Thread(TRUE) {}
  virtual void run();
	static void createInstance();
	static AutosaveThread *getInstance() { return instance; }
	void deleteASVFile( char *buffername );
};

#endif