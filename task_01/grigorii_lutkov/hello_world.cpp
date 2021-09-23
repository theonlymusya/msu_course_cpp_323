#include <stdexcept>
#include <iostream>

struct some_struct { int x; };

int main()
{

    auto s = some_struct();
    return 0; // success
}
