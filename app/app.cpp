#include "kernel.hpp"
#include "test.hpp"

int main()
{
    //clock initialisaion must be done here
    ThreadX::Kernel::start();
}

void ThreadX::application([[maybe_unused]] void *firstUnusedMemory)
{
    runTestCode();
}
