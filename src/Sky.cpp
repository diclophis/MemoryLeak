// Jon Bardin GPL


#include "MemoryLeak.h"


Sky::Sky(int ts) {
  LOGV("alloc Sky\n");
}


Sky::~Sky() {
  LOGV("dealloc Sky\n");
}
