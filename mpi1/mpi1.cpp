#include <iostream>
#include "..\lablib\mpi_lib.h"

int main(int *argc, char **argv)
{
    MPI_Init(argc, &argv);
    int root = 0, rows_count = 10, cols_count = 10;

    {
        std::vector<int> vec(0);
        mpi_p2p_special_matrix<int> mpi(root);
        if (mpi.is_root())
        {
            vec.resize(rows_count * cols_count);
            inout::initialize_random(vec, 0, 10);
            inout::out_linear_to_matrix(vec, rows_count, cols_count, ",\t");
            std::cout << std::endl;
        }

        mpi.set_buffer(vec, rows_count, cols_count);
        mpi.mpi_exec();

        if (mpi.is_root())
        {
            std::vector<int> result = mpi.get_result_buffer();
            inout::out_linear_to_matrix(result, rows_count, cols_count, ",\t");
            std::cout << "time of p2p program is: " << mpi.time() << "s" << std::endl << std::endl;
        }
    }

    {
        std::vector<int> vec(0);
        mpi_special_matrix<int> mpi(root);  
        if (mpi.is_root())
        {
            vec.resize(rows_count * cols_count);
            inout::initialize_random(vec, 0, 10);
            inout::out_linear_to_matrix(vec, rows_count, cols_count, ",\t");
            std::cout << std::endl;
        }

        mpi.set_buffer(vec, rows_count, cols_count);
        mpi.mpi_exec();

        if (mpi.is_root())
        {
            std::vector<int> result = mpi.get_result_buffer();
            inout::out_linear_to_matrix(result, rows_count, cols_count, ",\t");
            std::cout << "time of collective program is: " << mpi.time() << "s" << std::endl << std::endl;
        }
    }

    MPI_Finalize();

    return 0;
}