#ifndef TIMER_H
#define TIMER_H

#if ENABLE_TIMING

#include <chrono>
#include "Log.hpp"
using std::chrono::high_resolution_clock;
using std::chrono::time_point;
using std::chrono::milliseconds;
using std::chrono::duration_cast;


#define DECLARE_TIMER(TYPE, FRQ)	\
	static time_point<high_resolution_clock> sTimerStart_ ## TYPE;	\
	static time_point<high_resolution_clock> sTimerStop_ ## TYPE;	\
	static milliseconds sTimerMs_ ## TYPE = milliseconds::zero();	\
	static uint32_t sTimerFrq_ ## TYPE = 0;	\
	static const uint32_t sTIMER_MAX_FRQ_ ## TYPE = (FRQ);	\

#define START_TIMER(TYPE) {	\
	sTimerStart_ ## TYPE = high_resolution_clock::now();	\
}
#define STOP_TIMER(TYPE) {	\
	sTimerStop_ ## TYPE = high_resolution_clock::now();	\
	sTimerMs_ ## TYPE += duration_cast<milliseconds>(sTimerStop_ ## TYPE - sTimerStart_ ## TYPE);	\
	sTimerFrq_ ## TYPE ++;	\
	if (sTimerFrq_ ## TYPE == sTIMER_MAX_FRQ_ ## TYPE) {	\
		LOGD("TIMER (Avg %d): %s: %f ms\n", sTIMER_MAX_FRQ_ ## TYPE, #TYPE, \
			static_cast<double>(sTimerMs_ ## TYPE.count()) / sTIMER_MAX_FRQ_ ## TYPE);	\
		sTimerFrq_ ## TYPE = 0;	\
		sTimerMs_ ## TYPE = milliseconds::zero();	\
	}	\
}
#else
#define DECLARE_TIMER(TYPE, FRQ)
#define START_TIMER(TYPE)
#define STOP_TIMER(TYPE)

#endif // ENABLE_TIMING

#endif