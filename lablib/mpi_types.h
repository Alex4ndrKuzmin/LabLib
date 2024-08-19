#pragma once
#include "mpi.h"

#define TYPE_GETTER(type, mpi_type)         \
    template <>                             \
    MPI_Datatype get_mpi_datatype<type>()   \
    {                                       \
        return mpi_type;                    \
    }

template <typename Type>
MPI_Datatype get_mpi_datatype() { return MPI_DATATYPE_NULL; }

TYPE_GETTER(char,               MPI_CHAR)
TYPE_GETTER(unsigned char,      MPI_UNSIGNED_CHAR)
TYPE_GETTER(short,              MPI_SHORT)
TYPE_GETTER(unsigned short,     MPI_UNSIGNED_SHORT)
TYPE_GETTER(int,                MPI_INT)
TYPE_GETTER(unsigned,           MPI_UNSIGNED)
TYPE_GETTER(long,               MPI_LONG)
TYPE_GETTER(unsigned long,      MPI_UNSIGNED_LONG)
TYPE_GETTER(long long,          MPI_LONG_LONG)
TYPE_GETTER(float,              MPI_FLOAT)
TYPE_GETTER(double,             MPI_DOUBLE)
TYPE_GETTER(long double,        MPI_LONG_DOUBLE)
TYPE_GETTER(wchar_t,            MPI_WCHAR)