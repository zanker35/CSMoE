//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================

#include "source_sdk/tier0/pch_tier0.h"

#include "source_sdk/public/tier1/strtools.h"
#include "source_sdk/public/tier0/dynfunction.h"
#if   defined(POSIX)

#if !defined(OSX)
	#include <sys/fcntl.h>
	#include <sys/unistd.h>
	#define sem_unlink( arg )
	#define OS_TO_PTHREAD(x) (x)
#else
	#define pthread_yield pthread_yield_np
	#include <mach/thread_act.h>
	#include <mach/mach.h>
	#define OS_TO_PTHREAD(x) pthread_from_mach_thread_np( x )
#endif // !OSX

#ifdef LINUX
#include <dlfcn.h> // RTLD_NEXT
#endif

typedef int (*PTHREAD_START_ROUTINE)(
    void *lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;
#include <sched.h>
#include <exception>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include <inttypes.h>
#define GetLastError() errno
typedef void *LPVOID;
#endif

#include "source_sdk/public/tier0/valve_minmax_off.h"
#include <memory>
#include "source_sdk/public/tier0/valve_minmax_on.h"

#include "source_sdk/public/tier0/threadtools.h"
#include "source_sdk/public/tier0/vcrmode.h"

#ifdef _X360
#include "xbox/xbox_win32stubs.h"
#endif

#include "source_sdk/public/tier0/vprof_telemetry.h"

// Must be last header...
#include "source_sdk/public/tier0/memdbgon.h"

#define THREADS_DEBUG 1

// Need to ensure initialized before other clients call in for main thread ID


//-----------------------------------------------------------------------------
// Simple thread functions. 
// Because _beginthreadex uses stdcall, we need to convert to cdecl
//-----------------------------------------------------------------------------
struct ThreadProcInfo_t
{
	ThreadProcInfo_t( ThreadFunc_t pfnThread, void *pParam )
	  : pfnThread( pfnThread),
		pParam( pParam )
	{
	}
	
	ThreadFunc_t pfnThread;
	void *		 pParam;
};

//---------------------------------------------------------

#if   defined(POSIX)
static void *ThreadProcConvert( void *pParam )
#else
#error
#endif
{
	ThreadProcInfo_t info = *((ThreadProcInfo_t *)pParam);
	delete ((ThreadProcInfo_t *)pParam);
#if   defined(POSIX)
	return (void *)(*info.pfnThread)(info.pParam);
#else
#error
#endif
}


//---------------------------------------------------------

ThreadHandle_t CreateSimpleThread( ThreadFunc_t pfnThread, void *pParam, ThreadId_t *pID, unsigned stackSize )
{
#if   defined(POSIX)
	pthread_t tid;

	// If we need to create threads that are detached right out of the gate, we would need to do something like this:
	//   pthread_attr_t attr;
	//   int rc = pthread_attr_init(&attr);
	//   rc = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	//   ... pthread_create( &tid, &attr, ... ) ...
	//   rc = pthread_attr_destroy(&attr);
	//   ... pthread_join will now fail

	int ret = pthread_create( &tid, NULL, ThreadProcConvert, new ThreadProcInfo_t( pfnThread, pParam ) );
	if ( ret )
	{
		// There are only PTHREAD_THREADS_MAX number of threads, and we're probably leaking handles if ret == EAGAIN here?
		Error( "CreateSimpleThread: pthread_create failed. Someone not calling pthread_detach() or pthread_join. Ret:%d\n", ret );
	}
	if ( pID )
		*pID = (ThreadId_t)tid;
	Plat_ApplyHardwareDataBreakpointsToNewThread( (long unsigned int)tid );
	return (ThreadHandle_t)tid;
#endif
}

ThreadHandle_t CreateSimpleThread( ThreadFunc_t pfnThread, void *pParam, unsigned stackSize )
{
	return CreateSimpleThread( pfnThread, pParam, NULL, stackSize );
}

PLATFORM_INTERFACE void ThreadDetach( ThreadHandle_t hThread )
{
#if defined( POSIX )
	// The resources of this thread will be freed immediately when it terminates,
	//  instead of waiting for another thread to perform PTHREAD_JOIN.
	pthread_t tid = ( pthread_t )hThread;

	pthread_detach( tid );
#endif
}

bool ReleaseThreadHandle( ThreadHandle_t hThread )
{
	return true;
}

//-----------------------------------------------------------------------------
//
// Wrappers for other simple threading operations
//
//-----------------------------------------------------------------------------

void ThreadSleep(unsigned nMilliseconds)
{
#if   defined(POSIX)
   usleep( nMilliseconds * 1000 ); 
#endif
}

//-----------------------------------------------------------------------------

#ifndef ThreadGetCurrentId
ThreadId_t ThreadGetCurrentId()
{
#if   defined(POSIX)
	return (ThreadId_t)pthread_self();
#endif
}
#endif

//-----------------------------------------------------------------------------
ThreadHandle_t ThreadGetCurrentHandle()
{
#if   defined(POSIX)
	return (ThreadHandle_t)pthread_self();
#endif
}

// On PS3, this will return true for zombie threads
bool ThreadIsThreadIdRunning( ThreadId_t uThreadId )
{
#if   defined( _PS3 )
	
	// will return CELL_OK for zombie threads
	int priority;
	return (sys_ppu_thread_get_priority( uThreadId, &priority ) == CELL_OK );

#elif defined(POSIX)
	pthread_t thread = OS_TO_PTHREAD(uThreadId);
	if ( thread )
	{
		int iResult = pthread_kill( thread, 0 );
		if ( iResult == 0 )
			return true;
	}
	else
	{
		// We really ought not to be passing NULL in to here
		AssertMsg( false, "ThreadIsThreadIdRunning received a null thread ID" );
	}

	return false;
#endif
}

//-----------------------------------------------------------------------------

int ThreadGetPriority( ThreadHandle_t hThread )
{
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

	struct sched_param thread_param;
	int policy;
	pthread_getschedparam( (pthread_t)hThread, &policy, &thread_param );
	return thread_param.sched_priority;
}

//-----------------------------------------------------------------------------

bool ThreadSetPriority( ThreadHandle_t hThread, int priority )
{
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

#if   defined(POSIX)
	struct sched_param thread_param; 
	thread_param.sched_priority = priority; 
	pthread_setschedparam( (pthread_t)hThread, SCHED_OTHER, &thread_param );
	return true;
#endif
}

//-----------------------------------------------------------------------------

void ThreadSetAffinity( ThreadHandle_t hThread, int nAffinityMask )
{
	if ( !hThread )
	{
		hThread = ThreadGetCurrentHandle();
	}

#if   defined(POSIX)
// 	cpu_set_t cpuSet;
// 	CPU_ZERO( cpuSet );
// 	for( int i = 0 ; i < 32; i++ )
// 	  if ( nAffinityMask & ( 1 << i ) )
// 	    CPU_SET( cpuSet, i );
// 	sched_setaffinity( hThread, sizeof( cpuSet ), &cpuSet );
#endif

}

//-----------------------------------------------------------------------------

ThreadId_t InitMainThread()
{
#ifndef LINUX
	// Skip doing the setname on Linux for the main thread. Here is why...

	// From Pierre-Loup e-mail about why pthread_setname_np() on the main thread
	// in Linux will cause some tools to display "MainThrd" as the executable name:
	//
	// You have two things in procfs, comm and cmdline.  Each of the threads have
	// a different `comm`, which is the value you set through pthread_setname_np
	// or prctl(PR_SET_NAME).  Top can either display cmdline or comm; it
	// switched to display comm by default; htop still displays cmdline by
	// default. Top -c will output cmdline rather than comm.
	// 
	// If you press 'H' while top is running it will display each thread as a
	// separate process, so you will have different entries for MainThrd,
	// MatQueue0, etc with their own CPU usage.  But when that mode isn't enabled
	// it just displays the 'comm' name from the first thread.
	ThreadSetDebugName( "MainThrd" );
#endif

#if   defined(POSIX)
	return (ThreadId_t)pthread_self();
#endif
}

ThreadId_t g_ThreadMainThreadID = InitMainThread();

bool ThreadInMainThread()
{
	return ( ThreadGetCurrentId() == g_ThreadMainThreadID );
}

//-----------------------------------------------------------------------------
void DeclareCurrentThreadIsMainThread()
{
	g_ThreadMainThreadID = ThreadGetCurrentId();
}

bool ThreadJoin( ThreadHandle_t hThread, unsigned timeout )
{
	// You should really never be calling this with a NULL thread handle. If you
	// are then that probably implies a race condition or threading misunderstanding.
	Assert( hThread );
	if ( !hThread )
	{
		return false;
	}

#if   defined(POSIX)
	if ( pthread_join( (pthread_t)hThread, NULL ) != 0 )
		return false;
#endif
	return true;
}

#ifdef RAD_TELEMETRY_ENABLED
void TelemetryThreadSetDebugName( ThreadId_t id, const char *pszName );
#endif

//-----------------------------------------------------------------------------

void ThreadSetDebugName( ThreadId_t id, const char *pszName )
{
	if( !pszName )
		return;

#ifdef RAD_TELEMETRY_ENABLED
	TelemetryThreadSetDebugName( id, pszName );
#endif

#if   defined( _LINUX )
	// As of glibc v2.12, we can use pthread_setname_np.
	typedef int (pthread_setname_np_func)(pthread_t, const char *);
	static pthread_setname_np_func *s_pthread_setname_np_func = (pthread_setname_np_func *)dlsym(RTLD_DEFAULT, "pthread_setname_np");

	if ( s_pthread_setname_np_func )
	{
		if ( id == (uint32)-1 )
			id = pthread_self();

		/* 
			pthread_setname_np() in phthread_setname.c has the following code:
		
			#define TASK_COMM_LEN 16
			  size_t name_len = strlen (name);
			  if (name_len >= TASK_COMM_LEN)
				return ERANGE;
		
			So we need to truncate the threadname to 16 or the call will just fail.
		*/
		char szThreadName[ 16 ];
		strncpy( szThreadName, pszName, ARRAYSIZE( szThreadName ) );
		szThreadName[ ARRAYSIZE( szThreadName ) - 1 ] = 0;
		(*s_pthread_setname_np_func)( id, szThreadName );
	}
#endif
}


//-----------------------------------------------------------------------------



//-----------------------------------------------------------------------------
// Used to thread LoadLibrary on the 360
//-----------------------------------------------------------------------------
static ThreadedLoadLibraryFunc_t s_ThreadedLoadLibraryFunc = 0;
PLATFORM_INTERFACE void SetThreadedLoadLibraryFunc( ThreadedLoadLibraryFunc_t func )
{
	s_ThreadedLoadLibraryFunc = func;
}

PLATFORM_INTERFACE ThreadedLoadLibraryFunc_t GetThreadedLoadLibraryFunc()
{
	return s_ThreadedLoadLibraryFunc;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadSyncObject::CThreadSyncObject()
#if   defined(POSIX)
  : m_bInitalized( false )
#endif
{
}

//---------------------------------------------------------

CThreadSyncObject::~CThreadSyncObject()
{
#if   defined(POSIX)
   if ( m_bInitalized )
   {
	pthread_cond_destroy( &m_Condition );
        pthread_mutex_destroy( &m_Mutex );
	m_bInitalized = false;
   }
#endif
}

//---------------------------------------------------------

bool CThreadSyncObject::operator!() const
{
#if   defined(POSIX)
   return !m_bInitalized;
#endif
}

//---------------------------------------------------------

void CThreadSyncObject::AssertUseable()
{
#ifdef THREADS_DEBUG
#if   defined(POSIX)
   AssertMsg( m_bInitalized, "Thread synchronization object is unuseable" );
#endif
#endif
}

//---------------------------------------------------------

bool CThreadSyncObject::Wait( uint32 dwTimeout )
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
#if   defined(POSIX)
    pthread_mutex_lock( &m_Mutex );
    bool bRet = false;
    if ( m_cSet > 0 )
    {
		bRet = true;
		m_bWakeForEvent = false;
    }
    else
    {
		volatile int ret = 0;

		while ( !m_bWakeForEvent && ret != ETIMEDOUT )
		{
			struct timeval tv;
			gettimeofday( &tv, NULL );
			volatile struct timespec tm;
			
			uint64 actualTimeout = dwTimeout;
			
			if ( dwTimeout == TT_INFINITE && m_bManualReset )
				actualTimeout = 10; // just wait 10 msec at most for manual reset events and loop instead
				
			volatile uint64 nNanoSec = (uint64)tv.tv_usec*1000 + (uint64)actualTimeout*1000000;
			tm.tv_sec = tv.tv_sec + nNanoSec /1000000000;
			tm.tv_nsec = nNanoSec % 1000000000;

			do
			{   
				ret = pthread_cond_timedwait( &m_Condition, &m_Mutex, (const timespec *)&tm );
			} 
			while( ret == EINTR );

			bRet = ( ret == 0 );
			
			if ( m_bManualReset )
			{
				if ( m_cSet )
					break;
				if ( dwTimeout == TT_INFINITE && ret == ETIMEDOUT )
					ret = 0; // force the loop to spin back around
			}
		}
		
		if ( bRet )
			m_bWakeForEvent = false;
    }
    if ( !m_bManualReset && bRet )
		m_cSet = 0;
    pthread_mutex_unlock( &m_Mutex );
    return bRet;
#endif
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadEvent::CThreadEvent( bool bManualReset )
{
#if   defined( POSIX )
    pthread_mutexattr_t Attr;
    pthread_mutexattr_init( &Attr );
    pthread_mutex_init( &m_Mutex, &Attr );
    pthread_mutexattr_destroy( &Attr );
    pthread_cond_init( &m_Condition, NULL );
    m_bInitalized = true;
    m_cSet = 0;
	m_bWakeForEvent = false;
    m_bManualReset = bManualReset;
#else
#error "Implement me"
#endif
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------


//---------------------------------------------------------

bool CThreadEvent::Set()
{
   AssertUseable();
#if   defined(POSIX)
    pthread_mutex_lock( &m_Mutex );
    m_cSet = 1;
	m_bWakeForEvent = true;
    int ret = pthread_cond_signal( &m_Condition );
    pthread_mutex_unlock( &m_Mutex );
    return ret == 0;
#endif
}

//---------------------------------------------------------

bool CThreadEvent::Reset()
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
#if   defined(POSIX)
	pthread_mutex_lock( &m_Mutex );
	m_cSet = 0;
	m_bWakeForEvent = false;
	pthread_mutex_unlock( &m_Mutex );
	return true; 
#endif
}

//---------------------------------------------------------

bool CThreadEvent::Check()
{
#ifdef THREADS_DEBUG
   AssertUseable();
#endif
	return Wait( 0 );
}



bool CThreadEvent::Wait( uint32 dwTimeout )
{
	return CThreadSyncObject::Wait( dwTimeout );
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------

CThreadLocalBase::CThreadLocalBase()
{
#if   defined(POSIX)
	if ( pthread_key_create( &m_index, NULL ) != 0 )
		Error( "Out of thread local storage!\n" );
#endif
}

//---------------------------------------------------------

CThreadLocalBase::~CThreadLocalBase()
{
#if   defined(POSIX)
	pthread_key_delete( m_index );
#endif
}

//---------------------------------------------------------

void * CThreadLocalBase::Get() const
{
#if   defined(POSIX)
	void *value = pthread_getspecific( m_index );
	return value;
#endif
}

//---------------------------------------------------------

void CThreadLocalBase::Set( void *value )
{
#if   defined(POSIX)
	if ( pthread_setspecific( m_index, value ) != 0 )
		AssertMsg( 0, "Bad thread local" );
#endif
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

#if   defined(GNUC)

int32 ThreadInterlockedIncrement( int32 volatile *pDest )
{
	return __sync_fetch_and_add( pDest, 1 ) + 1;
}

int64 ThreadInterlockedIncrement64( int64 volatile *pDest )
{
    return  __sync_fetch_and_add( pDest, 1 ) + 1;
}

int32 ThreadInterlockedDecrement( int32 volatile *pDest )
{
	return __sync_fetch_and_sub( pDest, 1 ) - 1;
}

int64 ThreadInterlockedDecrement64( int64 volatile *pDest )
{
    return  __sync_fetch_and_sub( pDest, 1 ) - 1;
}

int32 ThreadInterlockedExchange( int32 volatile *pDest, int32 value )
{
	return __sync_lock_test_and_set( pDest, value );
}

int64 ThreadInterlockedExchange64( int64 volatile *pDest, int64 value )
{
    return  __sync_lock_test_and_set( pDest, value );
}

int32 ThreadInterlockedExchangeAdd( int32 volatile *pDest, int32 value )
{
	return  __sync_fetch_and_add( pDest, value );
}

int64 ThreadInterlockedExchangeAdd64( int64 volatile *pDest, int64 value )
{
    return  __sync_fetch_and_add( pDest, value );
}

int32 ThreadInterlockedCompareExchange( int32 volatile *pDest, int32 value, int32 comperand )
{
	return  __sync_val_compare_and_swap( pDest, comperand, value );
}

bool ThreadInterlockedAssignIf( int32 volatile *pDest, int32 value, int32 comperand )
{
	return __sync_bool_compare_and_swap( pDest, comperand, value );
}

void *ThreadInterlockedExchangePointer( void * volatile *pDest, void *value )
{
	return __sync_lock_test_and_set( pDest, value );
}

void *ThreadInterlockedCompareExchangePointer( void *volatile *pDest, void *value, void *comperand )
{	
	return  __sync_val_compare_and_swap( pDest, comperand, value );
}

bool ThreadInterlockedAssignPointerIf( void * volatile *pDest, void *value, void *comperand )
{
	return  __sync_bool_compare_and_swap( pDest, comperand, value );
}

int64 ThreadInterlockedCompareExchange64( int64 volatile *pDest, int64 value, int64 comperand )
{
	return __sync_val_compare_and_swap( pDest, comperand, value  );
}

bool ThreadInterlockedAssignIf64( int64 volatile * pDest, int64 value, int64 comperand ) 
{
	return __sync_bool_compare_and_swap( pDest, comperand, value );
}

#ifdef PLATFORM_64BITS
bool ThreadInterlockedAssignIf128( int128 volatile *pDest, const int128 &value, const int128 &comperand )
{
    return __sync_bool_compare_and_swap( pDest, comperand, value );
}
#endif


#else
// This will perform horribly,
#error "Falling back to mutexed interlocked operations, you really don't have intrinsics you can use?"ß
CThreadMutex g_InterlockedMutex;

int32 ThreadInterlockedIncrement( int32 volatile *pDest )
{
	AUTO_LOCK( g_InterlockedMutex );
	return ++(*pDest);
}

int32 ThreadInterlockedDecrement( int32 volatile *pDest )
{
	AUTO_LOCK( g_InterlockedMutex );
	return --(*pDest);
}

int32 ThreadInterlockedExchange( int32 volatile *pDest, int32 value )
{
	AUTO_LOCK( g_InterlockedMutex );
	int32 retVal = *pDest;
	*pDest = value;
	return retVal;
}

void *ThreadInterlockedExchangePointer( void * volatile *pDest, void *value )
{
	AUTO_LOCK( g_InterlockedMutex );
	void *retVal = *pDest;
	*pDest = value;
	return retVal;
}

int32 ThreadInterlockedExchangeAdd( int32 volatile *pDest, int32 value )
{
	AUTO_LOCK( g_InterlockedMutex );
	int32 retVal = *pDest;
	*pDest += value;
	return retVal;
}

int32 ThreadInterlockedCompareExchange( int32 volatile *pDest, int32 value, int32 comperand )
{
	AUTO_LOCK( g_InterlockedMutex );
	int32 retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}

void *ThreadInterlockedCompareExchangePointer( void * volatile *pDest, void *value, void *comperand )
{
	AUTO_LOCK( g_InterlockedMutex );
	void *retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}


int64 ThreadInterlockedCompareExchange64( int64 volatile *pDest, int64 value, int64 comperand )
{
	Assert( (size_t)pDest % 8 == 0 );
	AUTO_LOCK( g_InterlockedMutex );
	int64 retVal = *pDest;
	if ( *pDest == comperand )
		*pDest = value;
	return retVal;
}

int64 ThreadInterlockedExchange64( int64 volatile *pDest, int64 value )
{
	Assert( (size_t)pDest % 8 == 0 );
	int64 Old;

	do 
	{
		Old = *pDest;
	} while (ThreadInterlockedCompareExchange64(pDest, value, Old) != Old);

	return Old;
}

bool ThreadInterlockedAssignIf64(volatile int64 *pDest, int64 value, int64 comperand ) 
{
	Assert( (size_t)pDest % 8 == 0 );
	return ( ThreadInterlockedCompareExchange64( pDest, value, comperand ) == comperand ); 
}

bool ThreadInterlockedAssignIf( int32 volatile *pDest, int32 value, int32 comperand )
{
	Assert( (size_t)pDest % 4 == 0 );
	return ( ThreadInterlockedCompareExchange( pDest, value, comperand ) == comperand ); 
}

#endif

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//
// CThreadMutex
//
//-----------------------------------------------------------------------------

#ifndef POSIX
CThreadMutex::CThreadMutex()
{
#ifdef THREAD_MUTEX_TRACING_ENABLED
	memset( &m_CriticalSection, 0, sizeof(m_CriticalSection) );
#endif
	InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION *)&m_CriticalSection, 4000);
#ifdef THREAD_MUTEX_TRACING_SUPPORTED
	// These need to be initialized unconditionally in case mixing release & debug object modules
	// Lock and unlock may be emitted as COMDATs, in which case may get spurious output
	m_currentOwnerID = m_lockCount = 0;
	m_bTrace = false;
#endif
}

CThreadMutex::~CThreadMutex()
{
	DeleteCriticalSection((CRITICAL_SECTION *)&m_CriticalSection);
}
#endif // !POSIX

#if   defined( _X360 )
#define DynTryEnterCriticalSection TryEnterCriticalSection
#endif

bool CThreadMutex::TryLock()
{

#if   defined( POSIX )
	 return pthread_mutex_trylock( &m_Mutex ) == 0;
#else
#error "Implement me!"
	return true;
#endif
}

//-----------------------------------------------------------------------------
//
// CThreadFastMutex
//
//-----------------------------------------------------------------------------

#define THREAD_SPIN (8*1024)

void CThreadFastMutex::Lock( const uintp threadId, unsigned nSpinSleepTime ) volatile
{
	int i;
	if ( nSpinSleepTime != TT_INFINITE )
	{
		for ( i = THREAD_SPIN; i != 0; --i )
		{
			if ( TryLock( threadId ) )
			{
				return;
			}
			ThreadPause();
		}

		for ( i = THREAD_SPIN; i != 0; --i )
		{
			if ( TryLock( threadId ) )
			{
				return;
			}
			ThreadPause();
			if ( i % 1024 == 0 )
			{
				ThreadSleep( 0 );
			}
		}


		if ( nSpinSleepTime )
		{
			for ( i = THREAD_SPIN; i != 0; --i )
			{
				if ( TryLock( threadId ) )
				{
					return;
				}

				ThreadPause();
				ThreadSleep( 0 );
			}

		}

		for ( ;; ) // coded as for instead of while to make easy to breakpoint success
		{
			if ( TryLock( threadId ) )
			{
				return;
			}

			ThreadPause();
			ThreadSleep( nSpinSleepTime );
		}
	}
	else
	{
		for ( ;; ) // coded as for instead of while to make easy to breakpoint success
		{
			if ( TryLock( threadId ) )
			{
				return;
			}

			ThreadPause();
		}
	}
}

//-----------------------------------------------------------------------------
//
// CThreadRWLock
//
//-----------------------------------------------------------------------------

void CThreadRWLock::WaitForRead()
{
	m_nPendingReaders++;

	do
	{
		m_mutex.Unlock();
		m_CanRead.Wait();
		m_mutex.Lock();
	}
	while (m_nWriters);

	m_nPendingReaders--;
}


void CThreadRWLock::LockForWrite()
{
	m_mutex.Lock();
	bool bWait = ( m_nWriters != 0 || m_nActiveReaders != 0 );
	m_nWriters++;
	m_CanRead.Reset();
	m_mutex.Unlock();

	if ( bWait )
	{
		m_CanWrite.Wait();
	}
}

void CThreadRWLock::UnlockWrite()
{
	m_mutex.Lock();
	m_nWriters--;
	if ( m_nWriters == 0)
	{
		if ( m_nPendingReaders )
		{
			m_CanRead.Set();
		}
	}
	else
	{
		m_CanWrite.Set();
	}
	m_mutex.Unlock();
}

//-----------------------------------------------------------------------------
//
// CThreadSpinRWLock
//
//-----------------------------------------------------------------------------

void CThreadSpinRWLock::SpinLockForWrite( const uintp threadId )
{
	int i;

	for ( i = 1000; i != 0; --i )
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}
		ThreadPause();
	}

	for ( i = 20000; i != 0; --i )
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}

		ThreadPause();
		ThreadSleep( 0 );
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if ( TryLockForWrite( threadId ) )
		{
			return;
		}

		ThreadPause();
		ThreadSleep( 1 );
	}
}

void CThreadSpinRWLock::LockForRead()
{
	int i;

	// In order to grab a read lock, the number of readers must not change and no thread can own the write lock
	LockInfo_t oldValue;
	LockInfo_t newValue;

	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	oldValue.m_writerId = 0;
	newValue.m_nReaders = oldValue.m_nReaders + 1;
	newValue.m_writerId = 0;

	if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
		return;
	ThreadPause();
	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	newValue.m_nReaders = oldValue.m_nReaders + 1;

	for ( i = 1000; i != 0; --i )
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}

	for ( i = 20000; i != 0; --i )
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 0 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if( m_nWriters == 0 && AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 1 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders + 1;
	}
}

void CThreadSpinRWLock::UnlockRead()
{
	int i;

	Assert( m_lockInfo.m_nReaders > 0 && m_lockInfo.m_writerId == 0 );
	LockInfo_t oldValue;
	LockInfo_t newValue;

	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	oldValue.m_writerId = 0;
	newValue.m_nReaders = oldValue.m_nReaders - 1;
	newValue.m_writerId = 0;

	if( AssignIf( newValue, oldValue ) )
		return;
	ThreadPause();
	oldValue.m_nReaders = m_lockInfo.m_nReaders;
	newValue.m_nReaders = oldValue.m_nReaders - 1;

	for ( i = 500; i != 0; --i )
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}

	for ( i = 20000; i != 0; --i )
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 0 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}

	for ( ;; ) // coded as for instead of while to make easy to breakpoint success
	{
		if( AssignIf( newValue, oldValue ) )
			return;
		ThreadPause();
		ThreadSleep( 1 );
		oldValue.m_nReaders = m_lockInfo.m_nReaders;
		newValue.m_nReaders = oldValue.m_nReaders - 1;
	}
}

void CThreadSpinRWLock::UnlockWrite()
{
	Assert( m_lockInfo.m_writerId == ThreadGetCurrentId()  && m_lockInfo.m_nReaders == 0 );
	static const LockInfo_t newValue = { 0, 0 };
#if defined(_X360)
	// X360TBD: Serious Perf implications, not yet. __sync();
#endif
	int64 val; memcpy( &val, &newValue, sizeof( val ) );
	ThreadInterlockedExchange64(  (int64 *)&m_lockInfo, val );
	m_nWriters--;
}



//-----------------------------------------------------------------------------
//
// CThread
//
//-----------------------------------------------------------------------------

CThreadLocalPtr<CThread> g_pCurThread;

//---------------------------------------------------------

CThread::CThread()
:	
	m_threadId( 0 ),
	m_result( 0 ),
	m_flags( 0 )
{
	m_szName[0] = 0;
}

//---------------------------------------------------------

CThread::~CThread()
{
#if   defined(POSIX)
	if ( m_threadId )
#endif
	{
		if ( IsAlive() )
		{
			Msg( "Illegal termination of worker thread! Threads must negotiate an end to the thread before the CThread object is destroyed.\n" ); 
			if ( GetCurrentCThread() == this )
			{
				Stop(); // BUGBUG: Alfred - this doesn't make sense, this destructor fires from the hosting thread not the thread itself!!
			}
		}

	}
}


//---------------------------------------------------------

const char *CThread::GetName()
{
	AUTO_LOCK( m_Lock );
	if ( !m_szName[0] )
	{
#if   defined(POSIX)
		_snprintf( m_szName, sizeof(m_szName) - 1, "Thread(0x" PRIxPTR "/0x" PRIxPTR ")", (ThreadId_t)this, (ThreadId_t)m_threadId );
#endif
		m_szName[sizeof(m_szName) - 1] = 0;
	}
	return m_szName;
}

//---------------------------------------------------------

void CThread::SetName(const char *pszName)
{
	AUTO_LOCK( m_Lock );
	strncpy( m_szName, pszName, sizeof(m_szName) - 1 );
	m_szName[sizeof(m_szName) - 1] = 0;
}

//---------------------------------------------------------

bool CThread::Start( unsigned nBytesStack )
{
	AUTO_LOCK( m_Lock );

	if ( IsAlive() )
	{
		AssertMsg( 0, "Tried to create a thread that has already been created!" );
		return false;
	}

	bool  bInitSuccess = false;
	CThreadEvent createComplete;
	ThreadInit_t init = { this, &createComplete, &bInitSuccess };

#if   defined(POSIX)
	pthread_attr_t attr;
	pthread_attr_init( &attr );
	// From http://www.kernel.org/doc/man-pages/online/pages/man3/pthread_attr_setstacksize.3.html
	//  A thread's stack size is fixed at the time of thread creation. Only the main thread can dynamically grow its stack.
	pthread_attr_setstacksize( &attr, MAX( nBytesStack, 1024u*1024 ) );
	if ( pthread_create( &m_threadId, &attr, (void *(*)(void *))GetThreadProc(), new ThreadInit_t( init ) ) != 0 )
	{
		AssertMsg1( 0, "Failed to create thread (error 0x%x)", GetLastError() );
		return false;
	}
	Plat_ApplyHardwareDataBreakpointsToNewThread( (long unsigned int)m_threadId );
	bInitSuccess = true;
#endif

#if !defined( OSX )
	ThreadSetDebugName( m_threadId, m_szName );
#endif

	if ( !WaitForCreateComplete( &createComplete ) )
	{
		Msg( "Thread failed to initialize\n" );
#if   defined(POSIX)
		m_threadId = 0;
#endif
		return false;
	}

	if ( !bInitSuccess )
	{
		Msg( "Thread failed to initialize\n" );
#if   defined(POSIX)
		m_threadId = 0;
#endif
		return false;
	}


#if   defined(POSIX)
	return !!m_threadId;
#endif
}

//---------------------------------------------------------
//
// Return true if the thread exists. false otherwise
//

bool CThread::IsAlive()
{
#if   defined(POSIX)
	return m_threadId;
#endif
}

//---------------------------------------------------------

bool CThread::Join(unsigned timeout)
{
#if   defined(POSIX)
	if ( m_threadId )
#endif
	{
		AssertMsg(GetCurrentCThread() != this, _T("Thread cannot be joined with self"));

#if   defined(POSIX)
		return ThreadJoin( (ThreadHandle_t)m_threadId );
#endif
	}
	return true;
}

//---------------------------------------------------------


#if defined( _WIN32 ) || defined( LINUX )

//---------------------------------------------------------

uint CThread::GetThreadId()
{
	return m_threadId;
}

#endif

//---------------------------------------------------------

int CThread::GetResult()
{
	return m_result;
}

//---------------------------------------------------------
//
// Forcibly, abnormally, but relatively cleanly stop the thread
//

void CThread::Stop(int exitCode)
{
	if ( !IsAlive() )
		return;

	if ( GetCurrentCThread() == this )
	{
		m_result = exitCode;
		if ( !( m_flags & SUPPORT_STOP_PROTOCOL ) )
		{
			OnExit();
			g_pCurThread = NULL;

			Cleanup();
		}
		throw exitCode;
	}
	else
		AssertMsg( 0, "Only thread can stop self: Use a higher-level protocol");
}

//---------------------------------------------------------

int CThread::GetPriority() const
{
#if   defined(POSIX)
	struct sched_param thread_param;
	int policy;
	pthread_getschedparam( m_threadId, &policy, &thread_param );
	return thread_param.sched_priority;
#endif
}

//---------------------------------------------------------

bool CThread::SetPriority(int priority)
{
	return ThreadSetPriority( (ThreadHandle_t)m_threadId, priority );
}


//---------------------------------------------------------

void CThread::SuspendCooperative()
{
	if ( ThreadGetCurrentId() == (ThreadId_t)m_threadId )
	{
		m_SuspendEventSignal.Set();
		m_nSuspendCount = 1;
		m_SuspendEvent.Wait();
		m_nSuspendCount = 0;
	}
	else
	{
		Assert( !"Suspend not called from worker thread, this would be a bug" );
	}
}

//---------------------------------------------------------

void CThread::ResumeCooperative()
{
	//Assert( m_nSuspendCount == 1 );
	m_SuspendEvent.Set();
}


void CThread::BWaitForThreadSuspendCooperative()
{
	m_SuspendEventSignal.Wait();
}


#ifndef LINUX
//---------------------------------------------------------

unsigned int CThread::Suspend()
{
#if   defined(OSX)
	int susCount = m_nSuspendCount++;
	while ( thread_suspend( pthread_mach_thread_np(m_threadId) ) != KERN_SUCCESS )
	{
	};
	return ( susCount) != 0;
#else
#error
#endif
}

//---------------------------------------------------------

unsigned int CThread::Resume()
{
#if   defined(OSX)
	int susCount = m_nSuspendCount++;
	while ( thread_resume( pthread_mach_thread_np(m_threadId) )  != KERN_SUCCESS )
	{
	};	
	return ( susCount - 1) != 0;
#else
#error
#endif
}
#endif


//---------------------------------------------------------

bool CThread::Terminate(int exitCode)
{
#ifndef _X360
#if   defined(POSIX)
	pthread_kill( m_threadId, SIGKILL );
	Cleanup();
#endif

	return true;
#else
	AssertMsg( 0, "Cannot terminate a thread on the Xbox!" );
	return false;
#endif
}

//---------------------------------------------------------
//
// Get the Thread object that represents the current thread, if any.
// Can return NULL if the current thread was not created using
// CThread
//

CThread *CThread::GetCurrentCThread()
{
	return g_pCurThread;
}

//---------------------------------------------------------
//
// Offer a context switch. Under Win32, equivalent to Sleep(0)
//

void CThread::Yield()
{
#if   defined(POSIX)
	pthread_yield();
#endif
}

//---------------------------------------------------------
//
// This method causes the current thread to yield and not to be
// scheduled for further execution until a certain amount of real
// time has elapsed, more or less.
//

void CThread::Sleep(unsigned duration)
{
#if   defined(POSIX)
	usleep( duration * 1000 );
#endif
}

//---------------------------------------------------------

bool CThread::Init()
{
	return true;
}

//---------------------------------------------------------

void CThread::OnExit()
{
}

//---------------------------------------------------------

void CThread::Cleanup()
{
	m_threadId = 0;
}

//---------------------------------------------------------
bool CThread::WaitForCreateComplete(CThreadEvent * pEvent)
{
	// Force serialized thread creation...
	if (!pEvent->Wait(60000))
	{
		AssertMsg( 0, "Probably deadlock or failure waiting for thread to initialize." );
		return false;
	}
	return true;
}

//---------------------------------------------------------

bool CThread::IsThreadRunning()
{
#ifdef _PS3
    // ThreadIsThreadIdRunning() doesn't work on PS3 if the thread is in a zombie state
    return m_eventTheadExit.Check();
#else
    return ThreadIsThreadIdRunning( (ThreadId_t)m_threadId );
#endif
}

//---------------------------------------------------------

CThread::ThreadProc_t CThread::GetThreadProc()
{
	return ThreadProc;
}

//---------------------------------------------------------

unsigned __stdcall CThread::ThreadProc(LPVOID pv)
{
  std::unique_ptr<ThreadInit_t> pInit((ThreadInit_t *)pv);
  
#ifdef _X360
        // Make sure all threads are consistent w.r.t floating-point math
	SetupFPUControlWord();
#endif
	
	CThread *pThread = pInit->pThread;
	g_pCurThread = pThread;
	
	g_pCurThread->m_pStackBase = AlignValue( &pThread, 4096 );
	
	pInit->pThread->m_result = -1;
	
	bool bInitSuccess = true;
	if ( pInit->pfInitSuccess )
		*(pInit->pfInitSuccess) = false;
	
	try
	{
		bInitSuccess = pInit->pThread->Init();
	}
	
	catch (...)
	{
		pInit->pInitCompleteEvent->Set();
		throw;
	}
	
	if ( pInit->pfInitSuccess )
		*(pInit->pfInitSuccess) = bInitSuccess;
	pInit->pInitCompleteEvent->Set();
	if (!bInitSuccess)
		return 0;
	
	if ( pInit->pThread->m_flags & SUPPORT_STOP_PROTOCOL )
	{
		try
		{
			pInit->pThread->m_result = pInit->pThread->Run();
		}
		
		catch (...)
		{
		}
	}
	else
	{
		pInit->pThread->m_result = pInit->pThread->Run();
	}
	
	pInit->pThread->OnExit();
	g_pCurThread = NULL;
	pInit->pThread->Cleanup();
	
	return pInit->pThread->m_result;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
CWorkerThread::CWorkerThread()
:	m_EventSend(true),                 // must be manual-reset for PeekCall()
	m_EventComplete(true),             // must be manual-reset to handle multiple wait with thread properly
	m_Param(0),
	m_pParamFunctor(NULL),
	m_ReturnVal(0)
{
}

//---------------------------------------------------------

int CWorkerThread::CallWorker(unsigned dw, unsigned timeout, bool fBoostWorkerPriorityToMaster, CFunctor *pParamFunctor)
{
	return Call(dw, timeout, fBoostWorkerPriorityToMaster, NULL, pParamFunctor);
}

//---------------------------------------------------------

int CWorkerThread::CallMaster(unsigned dw, unsigned timeout)
{
	return Call(dw, timeout, false);
}

//---------------------------------------------------------

CThreadEvent &CWorkerThread::GetCallHandle()
{
	return m_EventSend;
}

//---------------------------------------------------------

unsigned CWorkerThread::GetCallParam( CFunctor **ppParamFunctor ) const
{
	if( ppParamFunctor )
		*ppParamFunctor = m_pParamFunctor;
	return m_Param;
}

//---------------------------------------------------------

int CWorkerThread::BoostPriority()
{
	int iInitialPriority = GetPriority();
	const int iNewPriority = ThreadGetPriority( (ThreadHandle_t)GetThreadID() );
	if (iNewPriority > iInitialPriority)
		ThreadSetPriority( (ThreadHandle_t)GetThreadID(), iNewPriority);
	return iInitialPriority;
}

//---------------------------------------------------------

static uint32 __stdcall DefaultWaitFunc( int nEvents, CThreadEvent * const *pEvents, int bWaitAll, uint32 timeout )
{
	return ThreadWaitForEvents( nEvents, pEvents, bWaitAll!=0, timeout );
//	return VCRHook_WaitForMultipleObjects( nHandles, (const void **)pHandles, bWaitAll, timeout );
}


int CWorkerThread::Call(unsigned dwParam, unsigned timeout, bool fBoostPriority, WaitFunc_t pfnWait, CFunctor *pParamFunctor)
{
	AssertMsg(!m_EventSend.Check(), "Cannot perform call if there's an existing call pending" );

	AUTO_LOCK( m_Lock );

	if (!IsAlive())
		return WTCR_FAIL;

	int iInitialPriority = 0;
	if (fBoostPriority)
	{
		iInitialPriority = BoostPriority();
	}

	// set the parameter, signal the worker thread, wait for the completion to be signaled
	m_Param = dwParam;
	m_pParamFunctor = pParamFunctor;

	m_EventComplete.Reset();
	m_EventSend.Set();

	WaitForReply( timeout, pfnWait );

	// MWD: Investigate why setting thread priorities is killing the 360
#ifndef _X360
	if (fBoostPriority)
		SetPriority(iInitialPriority);
#endif

	return m_ReturnVal;
}

//---------------------------------------------------------
//
// Wait for a request from the client
//
//---------------------------------------------------------
int CWorkerThread::WaitForReply( unsigned timeout )
{
	return WaitForReply( timeout, NULL );
}

int CWorkerThread::WaitForReply( unsigned timeout, WaitFunc_t pfnWait )
{
	if (!pfnWait)
	{
		pfnWait = DefaultWaitFunc;
	}

	
	CThreadEvent *waits[] =
	{
		&m_EventComplete
	};
	

	unsigned result;
	bool bInDebugger = Plat_IsInDebugSession();

	do
	{
		result = (*pfnWait)((sizeof(waits) / sizeof(waits[0])), waits, false,
			(timeout != TT_INFINITE) ? timeout : 30000);

		AssertMsg(timeout != TT_INFINITE || result != WAIT_TIMEOUT, "Possible hung thread, call to thread timed out");

	} while ( bInDebugger && ( timeout == TT_INFINITE && result == WAIT_TIMEOUT ) );

	if ( result != WAIT_OBJECT_0 + 1 )
	{
		if (result == WAIT_TIMEOUT)
			m_ReturnVal = WTCR_TIMEOUT;
		else if (result == WAIT_OBJECT_0)
		{
			DevMsg( 2, "Thread failed to respond, probably exited\n");
			m_EventSend.Reset();
			m_ReturnVal = WTCR_TIMEOUT;
		}
		else
		{
			m_EventSend.Reset();
			m_ReturnVal = WTCR_THREAD_GONE;
		}
	}

	return m_ReturnVal;
}


//---------------------------------------------------------
//
// Wait for a request from the client
//
//---------------------------------------------------------

bool CWorkerThread::WaitForCall(unsigned * pResult)
{
	return WaitForCall(TT_INFINITE, pResult);
}

//---------------------------------------------------------

bool CWorkerThread::WaitForCall(unsigned dwTimeout, unsigned * pResult)
{
	bool returnVal = m_EventSend.Wait(dwTimeout);
	if (pResult)
		*pResult = m_Param;
	return returnVal;
}

//---------------------------------------------------------
//
// is there a request?
//

bool CWorkerThread::PeekCall(unsigned * pParam, CFunctor **ppParamFunctor)
{
	if (!m_EventSend.Check())
	{
		return false;
	}
	else
	{
		if (pParam)
		{
			*pParam = m_Param;
		}
		if( ppParamFunctor )
		{
			*ppParamFunctor = m_pParamFunctor;
		}
		return true;
	}
}

//---------------------------------------------------------
//
// Reply to the request
//

void CWorkerThread::Reply(unsigned dw)
{
	m_Param = 0;
	m_ReturnVal = dw;

	// The request is now complete so PeekCall() should fail from
	// now on
	//
	// This event should be reset BEFORE we signal the client
	m_EventSend.Reset();

	// Tell the client we're finished
	m_EventComplete.Set();
}

//-----------------------------------------------------------------------------
