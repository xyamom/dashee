/**
 * @file include/dashee/Threads/Exception/Thread.h
 * @author Shahmir Javaid
 *
 * @section LICENSE
 *
 * This file is licensed under the terms and condition
 * mentioned at http://dashee.co.uk/license. In general
 * you are free to copy, distribute, display, and perform the work, 
 * you are also free to make derivative works, under the following conditions:
 *  - Attribution    — You must give dashee's author credit.
 *  - Non-Commercial - You may not use this dashee for commercial 
 *                     purposes without permission.
 *  - Share Alike    — If you alter, transform, or build upon this work, 
 *                     you may distribute the resulting work only under a 
 *                     licence identical to this one.
 *
 * The terms and conditions are updated at http://dashee.co.uk/license. View the
 * project site for more details
 */

#ifndef DASHEE_THREADS_EXCEPTION_THREAD_H_
#define DASHEE_THREADS_EXCEPTION_THREAD_H_

#include <dashee/Threads/Exception.h>

namespace dashee
{
    namespace Threads
    {
        class ExceptionThread;
    }
}

class dashee::Threads::ExceptionThread: public dashee::Threads::Exception
{
public:
    explicit ExceptionThread() : Exception()
    {
    }
    explicit ExceptionThread(const int ec) : Exception(ec)
    { 
    }
    explicit ExceptionThread(std::string msg) : Exception(msg)
    { 
    }
};

#endif
