#include "mytask.h"

// ===========================================================
// tosk run by Taskscheduler to handle the telegram messages
// ===========================================================
MyTask::~MyTask(){};
MyTask::MyTask(unsigned long interval, Scheduler *aS, void (*myCallback)()) : Task(interval, TASK_FOREVER, aS, true)
{
  _myCallback = myCallback;
};
bool MyTask::Callback()
{
  _myCallback();
  return true;
};
