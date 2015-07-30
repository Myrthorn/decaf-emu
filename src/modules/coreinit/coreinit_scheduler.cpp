#include "coreinit.h"
#include "coreinit_scheduler.h"
#include "coreinit_thread.h"
#include "processor.h"
#include "trace.h"

static std::atomic_bool
gSchedulerLock { false };

// Setup a thread fiber, used by OSRunThread and OSResumeThread
static void
InitialiseThreadFiber(OSThread *thread)
{
   // Create a fiber for the thread
   auto fiber = gProcessor.createFiber();
   thread->fiber = fiber;
   fiber->thread = thread;

   // Setup thread state
   memset(&fiber->state, 0, sizeof(ThreadState));

   for (auto i = 0u; i < 32; ++i) {
      fiber->state.gpr[i] = thread->context.gpr[i];
   }

   // Setup entry point
   fiber->state.cia = 0;
   fiber->state.nia = thread->entryPoint;

   // Initialise tracer
   traceInit(&fiber->state, 128);
}

void
OSLockScheduler()
{
   bool locked = false;

   while (!gSchedulerLock.compare_exchange_weak(locked, true, std::memory_order_acquire)) {
      locked = false;
   }
}

void
OSUnlockScheduler()
{
   gSchedulerLock.store(false, std::memory_order_release);
}

void
OSRescheduleNoLock()
{
   gProcessor.reschedule(true);
}

int32_t
OSResumeThreadNoLock(OSThread *thread, int32_t counter)
{
   auto old = thread->suspendCounter;
   thread->suspendCounter -= counter;

   if (thread->suspendCounter < 0) {
      thread->suspendCounter = 0;
      return old;
   }

   if (thread->suspendCounter == 0) {
      if (thread->state == OSThreadState::Ready) {
         // Create a fiber for the thread to run on, if this is the first time!
         if (!thread->fiber) {
            InitialiseThreadFiber(thread);
         }

         gProcessor.queue(thread->fiber);
      }
   }

   return old;
}

void
OSSleepThreadNoLock(OSThreadQueue *queue)
{
   auto thread = OSGetCurrentThread();
   thread->queue = queue;
   thread->state = OSThreadState::Waiting;

   if (queue) {
      OSInsertThreadQueue(queue, thread);
   }
}

void
OSSuspendThreadNoLock(OSThread *thread)
{
   thread->requestFlag = OSThreadRequest::None;
   thread->suspendCounter += thread->needSuspend;
   thread->needSuspend = 0;
   thread->state = OSThreadState::Ready;
   OSWakeupThreadNoLock(&thread->suspendQueue);
}

void
OSTestThreadCancelNoLock()
{
   auto thread = OSGetCurrentThread();

   if (thread->cancelState) {
      if (thread->requestFlag == OSThreadRequest::Suspend) {
         OSSuspendThreadNoLock(thread);
         OSRescheduleNoLock();
      } else if (thread->requestFlag == OSThreadRequest::Cancel) {
         OSUnlockScheduler();
         OSExitThread(-1);
      }
   }
}

void
OSWakeupOneThreadNoLock(OSThread *thread)
{
   thread->state = OSThreadState::Ready;
   gProcessor.queue(thread->fiber);
}

void
OSWakeupThreadNoLock(OSThreadQueue *queue)
{
   for (auto thread = queue->head; thread; thread = thread->link.next) {
      OSWakeupOneThreadNoLock(thread);
   }

   OSClearThreadQueue(queue);
}

void
OSWakeupThreadWaitForSuspensionNoLock(OSThreadQueue *queue, int32_t suspendResult)
{
   for (auto thread = queue->head; thread; thread = thread->link.next) {
      thread->suspendResult = suspendResult;
      gProcessor.queue(thread->fiber);
   }

   OSClearThreadQueue(queue);
}
