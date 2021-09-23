#include <iostream>
#include <stdexcept>

struct some_struct {
  int x;
};

void test_func() {
  return;
}

int main() {
  auto s = some_struct();
  return 0;  // success
}
