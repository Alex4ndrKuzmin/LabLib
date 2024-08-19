#include <iostream>
#include <vector>
#include <Functional>
#include "inout.h"
#include <ranges>
#include <random>
#include <map>
#include <iomanip>
#include <atomic>
#include <condition_variable>
#include <thread>

int main()
{
    
    std::thread jt([]() { 
        std::this_thread::sleep_for(std::chrono::seconds(5)); 
        std::cout << "Hello, world!\n"; 
     });
    jt.detach();
}