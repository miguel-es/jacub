#include <LTMemory.cpp>
#include <WMemory.cpp>

class MemoryModule: public RFModule {
protected:
	LTMemoryThread *ltmThread;
	WMemoryThread *wmThread;

public:
	virtual bool configure(ResourceFinder &rf) {
   	 string robotName = rf.check("robot", Value("jacub")).asString();
   	string kb_path = rf.check("kb_path", Value("../../../schemas/kb.json")).asString();
		ltmThread = new LTMemoryThread(robotName,kb_path,CTRL_THREAD_PER);
		if (!ltmThread->start()) {
			delete ltmThread;
			return false;
		}

		wmThread = new WMemoryThread(robotName,CTRL_THREAD_PER);
		if (!wmThread->start()) {
			delete wmThread;
			return false;
		}

		return true;
	}

	virtual bool close() {
		ltmThread->stop();
		delete ltmThread;
		wmThread->stop();
		delete wmThread;
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		return true;
	}
};

