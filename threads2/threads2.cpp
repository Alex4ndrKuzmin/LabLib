#include <iostream>
#include <thread>
#include <vector>
#include <algorithm>
#include <exception>
#include <mutex>

#include "..\lablib\inout.h"

typedef std::vector<std::vector<int>> Matrix;

Matrix mult_of_part_thread(Matrix& matrix, int beginning_row, int ending_row, std::mutex& mutex_array)
{
    mutex_array.lock();
    int n = matrix.size();
    mutex_array.unlock();

    Matrix result(ending_row - beginning_row, std::vector<int>(n, 0));
    for (int i = beginning_row; i < ending_row; ++i)
    {
        for (int j = 0; j < n; ++j)
        {
            for (int k = 0; k < n; ++k)
            {
                mutex_array.lock();
                int temp = matrix[i][k] * matrix[k][j];
                mutex_array.unlock();

                result[i - beginning_row][j] += temp;
            }
        }
    }

    return result;
}


Matrix pow2_thread(Matrix& matrix, int thread_count = 2)
{
    Matrix result;
    std::vector<std::thread> threads;
    std::vector<Matrix> parts(thread_count, Matrix());
    try
    {
        std::vector<int> chunks(thread_count, 0);

        for (int i = 0; i < thread_count; ++i)
            chunks[i] = matrix.size() / thread_count + (i < matrix.size() % thread_count ? 1 : 0);
        int offset = 0;

        std::mutex mutex_parts;
        std::mutex mutex_array;
        for (int i = 0; i < thread_count; ++i)
        {
            threads.push_back(std::thread([&parts, &matrix, offset, chunks, i, &thread_count, &mutex_parts, &mutex_array]()
            { 
                auto result = mult_of_part_thread(matrix, offset, offset + chunks[i], mutex_array);

                mutex_parts.lock();
                parts[i] = result;
                mutex_parts.unlock();
            }));
            offset += chunks[i];
        }       

        for (int i = 0; i < thread_count; ++i)
        {
            if (threads[i].joinable())
            {
                threads[i].join();
            }
        }
    }
    catch (std::out_of_range ex)
    {
        std::cout << "Exception thrown:" << ex.what() << "\n";
        std::cout << "Возможная причина: Матрица не квадратная\n";
    }

    for (int p = 0; p < parts.size(); ++p)
        for (int i = 0; i < parts[p].size(); ++i)
            result.push_back(parts[p][i]);

    return result;
}

int main()
{
    
    int matrix_size;
    int threads_count;
    INITIALIZE(matrix_size);
    INITIALIZE(threads_count);

    Matrix A(matrix_size, std::vector<int>(matrix_size, 0));
    inout::initialize_random(A, 0, 100);

    inout::out(A, std::vector<std::string>{"\n", "\t"});
    inout::out("\n\n");

    Matrix result = pow2_thread(A, 4);

    inout::out(result, std::vector<std::string>{"\n", "\t"});
    inout::out("\n\n");
   
    return 0;
}