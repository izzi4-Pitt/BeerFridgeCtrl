/*
  ValueSmoothing.h - Library for smoothing signal readings
*/
#ifndef ValueSmoothing_h
#define ValueSmoothing_h

#include "Arduino.h"

class ValueSmoothing
{
  public:
    ValueSmoothing(int ValueCount);
    float AddandReturn();
    void Add();
    float Return();
  private:
    int _Values;
};

#endif
