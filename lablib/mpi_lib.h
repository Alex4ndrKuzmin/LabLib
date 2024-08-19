#include "mpi_base.h"
#include "mpi_types.h"
#include "inout.h"
#include <vector>
#include <functional>


/*-------------------------------------------*/
/* Параллельное применение функции к вектору */
/*-------------------------------------------*/

template <Arithmetic Type>
class mpi_default_vector : public mpi_base
{
private:
    MPI_Datatype current_mpi_type;

    std::vector<Type> buffer;           // Инициализация только если rank == root
    std::vector<Type> result_buffer;    //
    
    std::vector<Type> sub_buffer;       // Юзается как буфер для приёма части данных
    std::vector<Type> sub_result_buffer;// Юзается как буфер для обработанной части данных

    
    std::function<void(std::vector<Type>& sub_buffer, std::vector<Type>& result_buffer)> function;

public:

    mpi_default_vector(int root = 0) : 
        mpi_base(root), 
        current_mpi_type(get_mpi_datatype<Type>())
    {}

    void splitting_data() override
    {

        int buffer_size = buffer.size();
        MPI_Bcast(&buffer_size, 1, MPI_INT, root, MPI_COMM_WORLD);

        if (buffer_size == 0)
            throw "zero buffer size";

        chunks_creating(buffer_size);
        sub_buffer.resize(chunks[rank]);
        sub_result_buffer.resize(chunks[rank]);
        MPI_Scatterv(
            buffer.data(), chunks.data(), offsets.data(), current_mpi_type,
            sub_buffer.data(), chunks[rank], current_mpi_type, root, MPI_COMM_WORLD);
    }

    void assembling_data() override
    {
        result_buffer.resize(buffer.size());
        MPI_Gatherv(
            sub_result_buffer.data(), chunks[rank], current_mpi_type, 
            result_buffer.data(), chunks.data(), offsets.data(), current_mpi_type, root, MPI_COMM_WORLD);
    }

    void process_data() override
    {
        function(sub_buffer, sub_result_buffer);
    }

    void set_function(std::function<void(std::vector<Type>& sub_buffer, std::vector<Type>& result_buffer)> function)
    {
        this->function = function;
    }

    std::function<void(std::vector<Type>& sub_buffer, std::vector<Type>& result_buffer)> get_function()
    {
        return function;
    }

    void set_buffer(std::vector<Type>& input_buffer)
    {
        buffer = std::move(input_buffer);
        result_buffer.resize(buffer.size());
    }

    std::vector<Type>&& get_result_buffer()
    {
        return std::move(result_buffer);
    }
};

/*------------------------------------------------------------------------*/
/* Вычесть из матрицы последнюю строку этой матрицы (коллективные обмены) */
/*------------------------------------------------------------------------*/

template <Arithmetic Type>
class mpi_special_matrix : public mpi_base
{
private:

    MPI_Datatype current_mpi_type;

    std::vector<Type> buffer;           // Инициализация только если rank == root
    std::vector<Type> result_buffer;    //

    std::vector<Type> sub_buffer;       // Юзается как буфер для приёма части данных
    std::vector<Type> sub_result_buffer;// Юзается как буфер для обработанной части данных

    std::vector<Type> last_row;

    int rows_count = 0;
    int cols_count = 0;

public:

    mpi_special_matrix(int root = 0) :
        mpi_base(root),
        current_mpi_type(get_mpi_datatype<Type>())
    {}

    void splitting_data() override
    {
        int buffer_size = rows_count - 1;
        MPI_Bcast(&buffer_size, 1, MPI_INT, root, MPI_COMM_WORLD);

        if (buffer_size == 0)
            throw "zero buffer size";

        chunks_creating(buffer_size, cols_count);
        sub_buffer.resize(chunks[rank]);
        sub_result_buffer.resize(chunks[rank]);
        MPI_Scatterv(
            buffer.data(), chunks.data(), offsets.data(), current_mpi_type,
            sub_buffer.data(), chunks[rank], current_mpi_type, root, MPI_COMM_WORLD);

        last_row.resize(cols_count);
        ROOT_PROCESS
        {
            for (int i = 0; i < cols_count; ++i)
                last_row[i] = buffer[(rows_count - 1) * cols_count + i];
        }
        MPI_Bcast(last_row.data(), cols_count, current_mpi_type, root, MPI_COMM_WORLD);
    }

    void assembling_data() override
    {
        ROOT_PROCESS
        result_buffer.resize(buffer.size());

        MPI_Gatherv(
            sub_result_buffer.data(), chunks[rank], current_mpi_type,
            result_buffer.data(), chunks.data(), offsets.data(), current_mpi_type, root, MPI_COMM_WORLD);

        ROOT_PROCESS
        {
            for (int i = 0; i < cols_count; ++i)
                result_buffer[(rows_count - 1) * cols_count + i] = last_row[i];
        }
    }

    void process_data() override
    {
        for (int i = 0; i < chunks[rank] / cols_count; ++i)
            for (int j = 0; j < cols_count; ++j)
                sub_result_buffer[i * cols_count + j] = sub_buffer[i * cols_count + j] - last_row[j];
    }

    void set_buffer(std::vector<Type>& input_buffer, int rows_count, int cols_count)
    {
        this->rows_count = rows_count;
        this->cols_count = cols_count;

        ROOT_PROCESS
        {
            if (rows_count * cols_count != input_buffer.size())
                throw "Wrong matrix size";

            buffer = std::move(input_buffer);
            result_buffer.resize(buffer.size());    
        }
    }

    std::vector<Type>&& get_result_buffer()
    {
        return std::move(result_buffer);
    }
};


/*--------------------------------------------------------------------*/
/* Вычесть из матрицы последнюю строку этой матрицы (точечные обмены) */
/*--------------------------------------------------------------------*/

template <Arithmetic Type>
class mpi_p2p_special_matrix : public mpi_base
{
private:

    MPI_Datatype current_mpi_type;

    std::vector<Type> buffer;           // Инициализация только если rank == root
    std::vector<Type> result_buffer;    //

    std::vector<Type> sub_buffer;       // Юзается как буфер для приёма части данных
    std::vector<Type> sub_result_buffer;// Юзается как буфер для обработанной части данных

    std::vector<Type> last_row;

    int rows_count = 0;
    int cols_count = 0;

public:

    mpi_p2p_special_matrix(int root = 0) :
        mpi_base(root),
        current_mpi_type(get_mpi_datatype<Type>())
    {}

    void splitting_data() override
    {
        MPI_Status status;
        int buffer_size = rows_count - 1;
        if (buffer_size == 0)
            throw "zero buffer size";

        last_row.resize(cols_count);
        ROOT_PROCESS
        {
            for (int i = 0; i < cols_count; ++i)
                last_row[i] = buffer[(rows_count - 1) * cols_count + i];

            for (int i = 0; i < size; ++i)
                if (i != root)
                { 
                    MPI_Send(&buffer_size, 1, MPI_INT, i, i, MPI_COMM_WORLD);
                    MPI_Send(last_row.data(), cols_count, current_mpi_type, i, 2*i, MPI_COMM_WORLD);
                }
        }
        else
        {
            
            MPI_Recv(&buffer_size, 1, MPI_INT, root, rank, MPI_COMM_WORLD, &status);
            MPI_Recv(last_row.data(), cols_count, current_mpi_type, root, 2*rank, MPI_COMM_WORLD, &status);
        }

        chunks_creating(buffer_size, cols_count);
        sub_buffer.resize(chunks[rank]);
        sub_result_buffer.resize(chunks[rank]);
       
        ROOT_PROCESS
        {
            for (int i = 0; i < size; ++i)
                if (i != root)
                {
                    MPI_Send(&buffer[offsets[i]], chunks[i], current_mpi_type, i, 3*i, MPI_COMM_WORLD);
                }
                else
                {
                    for (int j = 0; j < chunks[rank]; ++j)
                        sub_buffer[j] = buffer[offsets[rank] + j];
                }
        }
        else
        {
            MPI_Recv(sub_buffer.data(), chunks[rank], current_mpi_type, root, 3 * rank, MPI_COMM_WORLD, &status);
        }
    }

    void assembling_data() override
    {
        MPI_Status status;
        ROOT_PROCESS
        {
            result_buffer.resize(buffer.size());
            for (int i = 0; i < size; ++i)
                if (i != root)
                {
                    MPI_Recv(&result_buffer[offsets[i]], chunks[i], current_mpi_type, i, 4 * i, MPI_COMM_WORLD, &status);
                }
                else
                {
                    for (int j = 0; j < chunks[i]; ++j)
                        result_buffer[offsets[i] + j] = sub_result_buffer[j];
                }

            for (int i = 0; i < cols_count; ++i)
                result_buffer[(rows_count - 1) * cols_count + i] = last_row[i];
        }
        else
        {
            MPI_Send(sub_result_buffer.data(), chunks[rank], current_mpi_type, root, 4 * rank, MPI_COMM_WORLD);
        }
    }

    void process_data() override
    {
        for (int i = 0; i < chunks[rank] / cols_count; ++i)
            for (int j = 0; j < cols_count; ++j)
                sub_result_buffer[i * cols_count + j] = sub_buffer[i * cols_count + j] - last_row[j];
    }

    void set_buffer(std::vector<Type>& input_buffer, int rows_count, int cols_count)
    {
        this->rows_count = rows_count;
        this->cols_count = cols_count;

        ROOT_PROCESS
        {
            if (rows_count * cols_count != input_buffer.size())
                throw "Wrong matrix size";

            buffer = std::move(input_buffer);
            result_buffer.resize(buffer.size());
        }
    }

    std::vector<Type>&& get_result_buffer()
    {
        return std::move(result_buffer);
    }
};

/*-------------------------------*/
/* Вычисление двойного интеграла */
/*-------------------------------*/

template <Real Type>
class mpi_double_integral : public mpi_base
{
private:
    MPI_Datatype current_mpi_type;
    std::numeric_limits<Type> limit;

    std::function<Type(Type x, Type y)> function;
    std::function<Type(Type x, Type y)> lower_limit;                     // 
    std::function<Type(Type x, Type y)> upper_limit;                     // r    r
    std::function<Type(Type x, Type y)> lower_internal_limit;            // I dx I function(x, y)dy 
    std::function<Type(Type x, Type y)> upper_internal_limit;            // J    J
    Type epsilon = 0.01;
    Type step = 0.01;
    Type result = 0;

public:

    mpi_double_integral(int root = 0) :
        mpi_base(root),
        current_mpi_type(get_mpi_datatype<Type>())
    {}

    void splitting_data() override { }

    void assembling_data() override { }

    void process_data() override
    {
        Type hh = (upper_limit(0, 0) - lower_limit(0, 0)) / size;
        Type lower_limit_ = lower_limit(0, 0) + (Type)rank * hh;
        Type upper_limit_ = lower_limit_ + hh;      

        Type own_sum = 0;
        for (Type x = lower_limit_; x <= upper_limit_; x += step)
            own_sum += integral(lower_internal_limit(0, 0), upper_internal_limit(x, 0), x, epsilon) * step;

        MPI_Reduce(&own_sum, &result, 1, current_mpi_type, MPI_SUM, root, MPI_COMM_WORLD);
    }

    Type integral(Type c, Type d, Type x, Type eps)
    {
        auto get_sum = [&d, &c, &x](int n, std::function<Type(Type x, Type y)> &function)
        {
            Type width = (d - c) / n;
            Type result = 0;
            for (int i = 0; i < n; ++i)
            {
                Type x1 = c + i * width;
                Type x2 = c + (i + 1) * width;
                result += (x2 - x1) / 6.0 * (function(x, x1) + 4.0 * function(x, 0.5 * (x1 + x2)) + function(x, x2));
            }
            return result;
        };

        int n = 2;
        Type sum1 = get_sum(n, function); n *= 2;
        Type sum2 = get_sum(n, function);
        while (std::fabs(sum1 - sum2) > eps)
        {
            sum1 = sum2;
            n *= 2;
            sum2 = get_sum(n, function);
        }
        return sum2;
    }

    void set_function(std::function<Type(Type x, Type y)> function,
        std::function<Type(Type x, Type y)> lower_limit,
        std::function<Type(Type x, Type y)> upper_limit,
        std::function<Type(Type x, Type y)> lower_internal_limit,
        std::function<Type(Type x, Type y)> upper_internal_limit)
    {
        this->lower_limit = lower_limit;
        this->upper_limit = upper_limit;
        this->lower_internal_limit = lower_internal_limit;
        this->upper_internal_limit = upper_internal_limit;
        this->function = function;
    }

    void set_step_value(Type step)
    {
        this->step = step;
    }

    void set_epsilon_value(Type epsilon)
    {
        this->epsilon = epsilon;
    }

    Type get_result()
    {
        return result;
    }
};