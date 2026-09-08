/*
*
*   This program is free software; you can redistribute it and/or modify it
*   under the terms of the GNU General Public License as published by the
*   Free Software Foundation; either version 2 of the License, or (at
*   your option) any later version.
*
*   This program is distributed in the hope that it will be useful, but
*   WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*   General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program; if not, write to the Free Software Foundation,
*   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
*   In addition, as a special exception, the author gives permission to
*   link the code of this program with the Half-Life Game Engine ("HL
*   Engine") and Modified Game Libraries ("MODs") developed by Valve,
*   L.L.C ("Valve").  You must obey the GNU General Public License in all
*   respects for all of the code used other than the HL Engine and MODs
*   from Valve.  If you modify this file, you may extend this exception
*   to your version of the file, but you are not obligated to do so.  If
*   you do not wish to do so, delete this exception statement from your
*   version.
*
*/

#ifndef PERF_COUNTER_H
#define PERF_COUNTER_H

	#include <sys/stat.h>
	#include <sys/types.h>
	#include <fcntl.h>
	#include <unistd.h>
		#include <limits.h>
	#include <sys/time.h>

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>

class CPerformanceCounter
{
public:
	CPerformanceCounter();

	void InitializePerformanceCounter();
	double GetCurTime();

private:
	int m_iLowShift;
	double m_flPerfCounterFreq;
	double m_flCurrentTime;
	double m_flLastCurrentTime;
};

/* <2ebfc> ../game_shared/perf_counter.h:61 */
inline CPerformanceCounter::CPerformanceCounter() :
	m_iLowShift(0),
	m_flPerfCounterFreq(0),
	m_flCurrentTime(0),
	m_flLastCurrentTime(0)
{
	InitializePerformanceCounter();
}

/* <2ebdd> ../game_shared/perf_counter.h:69 */
inline void CPerformanceCounter::InitializePerformanceCounter()
{
}

/* <2ec16> ../game_shared/perf_counter.h:97 */
inline double CPerformanceCounter::GetCurTime()
{

	struct timeval tp;
	static int secbase = 0;

	gettimeofday(&tp, NULL);

	if (!secbase)
	{
		secbase = tp.tv_sec;
		return (tp.tv_usec / 1000000.0);
	}

	return ((tp.tv_sec - secbase) + tp.tv_usec / 1000000.0);

}

#endif // PERF_COUNTER_H
