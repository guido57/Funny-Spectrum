#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>
#include <Arduino.h>


class  MyTask : public Task {
  public:
    void (*_myCallback)();
    ~MyTask();
    MyTask(unsigned long interval, Scheduler* aS, void (* myCallback)() );// :  Task(interval, TASK_FOREVER, aS, true) {
    bool Callback();
    //void SendMessage(String message);
};

extern MyTask * myTask; 