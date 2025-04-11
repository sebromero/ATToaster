#ifndef _ZTimer_H_
#define _ZTimer_H_

class ZTimer {
public:
	typedef void (*TimerCallback)(void);// Create a type to point to a funciton.
	ZTimer();
	bool checkTime();
	ZTimer& setCallBack(TimerCallback callback);
	ZTimer& stopTimer();
	ZTimer& startTimer();
	ZTimer& resetTimer(bool restart = true);
	ZTimer& setWaitTime(unsigned long Time);
	ZTimer& setLastTime(unsigned long Time);
	ZTimer& pause();
	ZTimer& enableFireEverytime();
	ZTimer& enableFireAfter();
	ZTimer& useMillis();
	ZTimer& useMicros();
	
	unsigned long getNow();
	unsigned long getWaitTime();
	unsigned long getLastTime();

private:
	volatile TimerCallback _callback = nullptr;
	unsigned long _waitTime;
	bool          _fireEveryTime;
	bool          _useMicroSeconds;
	bool          _shouldRestart;
	bool          _enabled;
	unsigned long _lastCheckedTime;
	unsigned long _remainingTime;
};
#endif
