
#include "MemoryLeak.h"

extern "C" {

  MLPoint MLPointMake(float x, float y) {
    return (MLPoint) { x, y };
  }

};
