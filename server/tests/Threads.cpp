#include "Threads.h"

volatile bool dashee::test::RUN = false;
dashee::Threads::LockMutex dashee::test::mutexRUN 
    = dashee::Threads::LockMutex();

volatile int dashee::test::sharedVariable = 0;

// Initilize the different lock types
dashee::Threads::LockMutex dashee::test::lockMutex
    = dashee::Threads::LockMutex();
dashee::Threads::LockReadWrite dashee::test::lockRead
    = dashee::Threads::LockReadWrite();
dashee::Threads::LockReadWrite dashee::test::lockWrite
    = dashee::Threads::LockReadWrite(
            dashee::Threads::LockReadWrite::LOCKTYPE_WRITE
        );

/**
 * Dummy function which does nothing and returns
 *
 * @param nothing not used
 * 
 * @returns Nothing
 */ 
void * dashee::test::donothing(void * nothing)
{
    CPPUNIT_ASSERT(true);
    return NULL;
}

/**
 * This is a function which infinatly loops till the RUN flag is changed
 *
 * @param nothing not used
 * 
 * @returns Nothing
 */
void * dashee::test::waitTillExit(void * nothing)
{
    while (true)
    {
	mutexRUN.lock();
	if (RUN == false) 
	    break;
	mutexRUN.unlock();

	sleep(100);
    }

    return NULL;
}

/**
 * Do an operation N times and sleep through every iteration
 *
 * @param N Integer pointer to the number of times an operation must be done
 * 
 * @returns Nothing
 */
void * dashee::test::doN(void * N)
{
    for (int x = 0; x < *(reinterpret_cast<int *>(N)); x++)
    {
	mutexRUN.lock();
	if (RUN == false) break;
	mutexRUN.lock();

	sleep(100);
    }

    return NULL;
}

/**
 * This will add 1 to the shared variables, Mutlple threads can be
 * set lose to this to ensure that the value acuumalates correctly
 *
 * @param l The dashee::Threads::Lock object
 *
 * @returns Nothing
 */
void * dashee::test::addNTimes(void * l)
{
    dashee::Threads::Lock * lock 
        = reinterpret_cast<dashee::Threads::Lock *>(l);

    // Do nothing for a while, so other threads can 
    // try to fight for locks
    for (int x = 0; x < 100; x++)
    {
        lock->lock();
        
        dashee::test::sharedVariable++;
        sleep(100);

        lock->unlock();
    }

    return NULL;
}

/**
 * This will call the self function and ensure that the value returned is not
 * null
 *
 * @param nothing not used
 * 
 * @returns Nothing
 */ 
void * dashee::test::callSelf(void * nothing)
{
    CPPUNIT_ASSERT(dashee::Threads::Thread::self() > 0);
    dashee::Threads::Thread::exit();
    return NULL;
}

/**
 * Call exit within the thread to ensure things perform well
 * 
 * @param nothing not used
 * 
 * @returns Nothing
 */
void * dashee::test::callExit(void * nothing)
{
    dashee::Threads::Thread::exit();
    CPPUNIT_ASSERT(true);
    return NULL;
}

/**
 * Try double locking threads, and try to recover.
 *
 * This will fail in a non conventional way, The failiure is 
 * detected by the thread never comming out of lock in some cases
 *
 * So the only way you know it passed, is that if the test finished.
 *
 * Its not ideal but it is better than nothing, completely open to new 
 * ideas
 *
 * @param l The lock that was passed
 *
 * @returns Nothing
 */ 
void * dashee::test::doubleLockWrite(void * l)
{
    dashee::Threads::Lock * lock 
        = reinterpret_cast<dashee::Threads::Lock *>(l);

    try
    {
        // Do nothing for a while, so other threads can 
        // try to fight for locks
        for (int x = 0; x < 100; x++)
        {
            lock->lock();
            lock->lock();
            
            dashee::test::sharedVariable++;
            sleep(100);

            lock->unlock();
        }
    }
    catch (dashee::Threads::Exception ex)
    {
        lock->unlock();
    }
    
    return NULL;
}

void * dashee::test::doubleLockRead(void * l)
{
    dashee::Threads::Lock * lock 
        = reinterpret_cast<dashee::Threads::Lock *>(l);

    try
    {
        // Do nothing for a while, so other threads can 
        // try to fight for locks
        for (int x = 0; x < 100; x++)
        {
            lock->lock();
            lock->lock();
            sleep(1000);

            lock->unlock();
        }
    }
    catch (dashee::Threads::Exception ex)
    {
        lock->unlock();
    }
    
    return NULL;
}

/**
 * Do nothing
 */
void dashee::test::Threads::setUp()
{
}

/**
 * Test a simple 
 */
void dashee::test::Threads::testWorking()
{
    this->thread = new dashee::Threads::Thread(dashee::test::donothing);
    this->thread->start((void *)NULL);
    this->thread->join();

    delete this->thread;
}

/**
 * You can't start a thread twice asynchronisly but you can do it synchronisly.
 */
void dashee::test::Threads::testMultiStarts()
{
    this->thread = new dashee::Threads::Thread(donothing);
    this->thread->start((void *)NULL);
    this->thread->join();
    CPPUNIT_ASSERT(true);

    this->thread->start((void *)NULL);
    this->thread->join();
    CPPUNIT_ASSERT(true);
    
    this->thread->start((void *)NULL);
    this->thread->join();
    CPPUNIT_ASSERT(true);

    delete this->thread;
}   

/**
 * Simple thread just runs callSelf, which is asserted
 */
void dashee::test::Threads::testSelfCall()
{
    this->thread = new dashee::Threads::Thread(callSelf);
    this->thread->start((void *)NULL);
    this->thread->join();
}

/**
 * Simple thread just runs callSelf, which is asserted
 */
void dashee::test::Threads::testExits()
{
    this->thread = new dashee::Threads::Thread(callExit);
    this->thread->start((void *)NULL);
    this->thread->join();
}

/**
 * Test Adding to sharedVariable using locking, lockas are passed in as value
 */
void dashee::test::Threads::testLock()
{
    dashee::Threads::Thread * t1 = new dashee::Threads::Thread(addNTimes);
    dashee::Threads::Thread * t2 = new dashee::Threads::Thread(addNTimes);
    dashee::Threads::Thread * t3 = new dashee::Threads::Thread(addNTimes);
    dashee::Threads::Thread * t4 = new dashee::Threads::Thread(addNTimes);

    t1->start(&lockWrite);
    t2->start(&lockWrite);
    t3->start(&lockWrite);
    t4->start(&lockWrite);

    t1->join();
    t2->join();
    t3->join();
    t4->join();
    
    CPPUNIT_ASSERT(sharedVariable == 400);
    
    t1->start(&lockMutex);
    t2->start(&lockMutex);
    t3->start(&lockMutex);
    t4->start(&lockMutex);
    
    t1->join();
    t2->join();
    t3->join();
    t4->join();

    CPPUNIT_ASSERT(sharedVariable == 800);

    t1->start(&lockWrite);
    t2->start(&lockMutex);
    t3->start(&lockWrite);
    t4->start(&lockMutex);
    
    t1->join();
    t2->join();
    t3->join();
    t4->join();
    
    CPPUNIT_ASSERT(sharedVariable == 1200);
    sharedVariable = 0;

    delete t1;
    delete t2;
    delete t3;
    delete t4;
}

/**
 * Test double locking write and read, All locks should fail and leave the 
 * sharedVariable untouched
 */ 
void dashee::test::Threads::testDoubleLock()
{
    sharedVariable = 0;

    dashee::Threads::Thread * t1 = new dashee::Threads::Thread(doubleLockWrite);
    dashee::Threads::Thread * t2 = new dashee::Threads::Thread(doubleLockWrite);
    dashee::Threads::Thread * t3 = new dashee::Threads::Thread(doubleLockWrite);

    t1->start(&lockWrite);
    t2->start(&lockWrite);
    t3->start(&lockWrite);
    
    t1->join();
    t2->join();
    t3->join();
    
    CPPUNIT_ASSERT(sharedVariable == 0);
    
    dashee::Threads::Thread * t4 = new dashee::Threads::Thread(doubleLockRead);
    dashee::Threads::Thread * t5 = new dashee::Threads::Thread(doubleLockRead);
    dashee::Threads::Thread * t6 = new dashee::Threads::Thread(doubleLockRead);
    
    t1->start(&lockWrite);
    t2->start(&lockWrite);
    t3->start(&lockWrite);
    t4->start(&lockRead);
    t5->start(&lockRead);
    t6->start(&lockRead);
    
    t1->join();
    t2->join();
    t3->join();
    t4->join();
    t5->join();
    t6->join();
    
    CPPUNIT_ASSERT(sharedVariable == 0);

    delete t1;
    delete t2;
    delete t3;
    delete t4;
    delete t5;
    delete t6;
}

/**
 * This function should throw an exception because a thread must only be 
 * started once
 */
void dashee::test::Threads::testCallingOneStartOnly()
{
    this->thread = new dashee::Threads::Thread(donothing);
    this->thread->start((void *)NULL);
    this->thread->start((void *)NULL);
    this->thread->join();
}

/**
 * Join must only be initiated on started threads
 */
void dashee::test::Threads::testCallingJoinOnAStopedThread()
{
    this->thread = new dashee::Threads::Thread(donothing);
    this->thread->join();
}

/**
 * Kill any thread still running, especially detached ones, by
 * changing the flag of RUN, test assumes RUN to be global, and one set
 * may only run a set of threads and must also close any threads. If not
 * teardown should manage to change RUN where all threads should respond
 */ 
void dashee::test::Threads::tearDown()
{
    mutexRUN.lock();
    RUN = false;
    mutexRUN.unlock();
}
