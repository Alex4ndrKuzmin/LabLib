#include <iostream>
#include <thread>
#include <string>
#include <mutex>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>
#include "..\lablib\inout.h"

int even_count_threads(std::vector<int>& array_in, int thread_count = 2)
{
    std::vector<std::thread> threads;
    std::vector<int> array_of_sums(thread_count, 0);
    std::mutex mutex_array;

    auto count_even = [](std::vector<int>& array_in, int begin_index, int element_count, int& even_count, std::mutex& mutex_array)
    {
        std::vector<int>::iterator it_begin = array_in.begin();
        std::vector<int>::iterator it_end = array_in.begin();
        std::advance(it_begin, begin_index);
        std::advance(it_end, begin_index + element_count);
        even_count = std::count_if(it_begin, it_end, [&mutex_array](int& number)
            {
                std::lock_guard<std::mutex> lock(mutex_array);
                return !(number & 1);
            });
    };

    std::vector<int> chunks(thread_count, 0);
    for (int i = 0; i < thread_count; ++i)
        chunks[i] = array_in.size() / thread_count + (i < array_in.size() % thread_count ? 1 : 0);
    int offset = 0;
    try
    {
        for (int i = 0; i < thread_count; offset += chunks[i], ++i)
            threads.push_back(std::thread(count_even, std::ref(array_in), offset, chunks[i], std::ref(array_of_sums[i]), std::ref(mutex_array)));

        int level = 1;
        while ((level *= 2) < thread_count)
        {
            for (int i = 0; i < std::ceil((float)thread_count / level); ++i)
            {
                threads[level * i].join();
                if (level * i + level / 2 < thread_count)
                    threads[level * i + level / 2].join();

                auto sum_func = [](int& a, int b) { a = a + b; };
                auto a = std::ref(array_of_sums[level * i]);
                auto b = (level * i + level / 2 < thread_count) ? (array_of_sums[level * i + level / 2]) : 0;

                threads[level * i] = std::thread(sum_func, a, b);
            }
        }
        threads[0].join();
        threads[level / 2].join();
        array_of_sums[0] += array_of_sums[level / 2];
        return array_of_sums[0];
    }
    catch (std::exception ex)
    {
        inout::out(ex.what());
        std::cout << "\n";
    }
    return -1;
}

int main()
{
    int thread_count = 8;
    int array_size = 16;
    int result = 0;
    
    std::vector<int> array_in(array_size, 0);
    inout::initialize_random(array_in, 0, 100);
    inout::out(array_in);

    std::cout << "\nWith threads: " + std::to_string(even_count_threads(array_in, thread_count)) + "\n";
    std::cout << "Without threads: " + std::to_string(std::count_if(array_in.begin(), array_in.end(), [](int& number) { return !(number & 1); })) + "\n";

    return 0;
}
