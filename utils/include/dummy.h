#ifndef DUMMY_H
#define DUMMY_H

#include <blensor_wrapper.h>

class dummy {
  public:
    dummy();
    ~dummy();
  private:
    BlensorWrapper *blensor;
};

#endif
