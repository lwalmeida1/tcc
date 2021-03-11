#ifndef ESPBREW_TASK_NODE_H
#define ESPBREW_TASK_NODE_H

enum taskType
{
  byTemperature_and_Time = 0,
  byButtonPressing = 1
};

class TaskNode
{
  public:
    // Properties
    float Temperature;
    char Description[100];
    bool UsePump;
    unsigned long Duration; /* minutes */
    taskType Type;

    // Constructors
    TaskNode();
    TaskNode(float, char *, unsigned long, taskType, bool);

    // Methods
};

TaskNode::TaskNode()
{
  this->Temperature = 0;
  strcpy(this->Description, "nothing to do");
  this->UsePump = false;
  this->Duration = 0;
  this->Type = byTemperature_and_Time;
}

TaskNode::TaskNode(float temperature, char *description, unsigned long duration, taskType type = byTemperature_and_Time, bool usePump = false)
{
  this->Temperature = temperature;
  strcpy(this->Description, description);
  this->Duration = duration * 60 * 1000; /* minutes to milliseconds */
  this->Type = type;
  this->UsePump = usePump;
}

#endif ESPBREW_TASK_NODE_H
