// module.cpp
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

extern "C" { // To prevent C++ name mangling

  EMSCRIPTEN_KEEPALIVE // Keep this function when optimizing
  int add(int a, int b) {
    std::cout << "C++ add function called with " << a << " and " << b << std::endl;
    return a + b;
  }
}