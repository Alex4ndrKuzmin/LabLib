#pragma once
#include "mpi.h"
#include <map>
#include <vector>

#define ROOT_PROCESS if (rank == root)

#define SUB_PROCESS if (rank != root)


class mpi_base
{
protected:

    int size;
    int rank;
    int root;
    double MPI_time = 0;

    std::vector<int> chunks;
    std::vector<int> offsets;

    virtual void splitting_data() = 0;
    virtual void assembling_data() = 0;
    virtual void process_data() = 0;

    virtual void chunks_creating(int buffer_size, int cols_count = 1)
    {
        chunks.resize(buffer_size);
        offsets.resize(buffer_size);

        offsets[0] = 0;
        for (int i = 0; i < size; ++i)
        {
            chunks[i] = (buffer_size / size + (i < buffer_size % size ? 1 : 0)) * cols_count;
            if (i >= 1)
            {
                offsets[i] = offsets[i - 1] + chunks[i - 1];
            }
        }
    }

public:

    mpi_base(int root_ = 0) : root(root_) 
    {
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);
    }

    virtual void mpi_exec()
    {
        MPI_time = MPI_Wtime();
        splitting_data();
        process_data();
        assembling_data();
        MPI_time = MPI_Wtime() - MPI_time;
    }

    double time()
    {
        return MPI_time;
    }

    bool is_root()
    {
        return rank == root;
    }

    int get_rank()
    {
        return rank;
    }

    int get_size()
    {
        return size;
    }
};