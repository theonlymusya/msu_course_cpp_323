#include <iostream>
#include <stdexcept>

struct SomeStruct {
  int x;
};

void test_func() {
  return;
}

int main() {
  const auto s = SomeStruct();
  test_func();
  return 0;  // success
}
