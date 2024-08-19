#include <iostream>
#include <chrono>
#include "..\lablib\mpi_lib.h"

int main(int *argc, char **argv)
{
    MPI_Init(argc, &argv);

    // Будем искать среднее время за iteration_count итераций
    int iteration_count = 10;
    double T1 = 0, Ts = 0;
    for (int i = 0; i < iteration_count; ++i)
    {
        auto function = [](double x, double y) { return std::sqrt(x + y); };
        double step = 0.000001;
        double epsilon = 0.00001;

        auto a = [](double x = 0, double y = 0) { return 0.0; };  //Пределы интегрирования заданы как функции
        auto b = [](double x = 0, double y = 0) { return 1.0; };
        auto c = [](double x = 0, double y = 0) { return 0.0; };
        auto d = [](double x, double y = 0) { return x; };

        mpi_double_integral<double> mpi(0);
        mpi.set_function(function, a, b, c, d);
        mpi.set_epsilon_value(epsilon);
        mpi.set_step_value(step);
        mpi.mpi_exec();

        if (mpi.is_root())
        {
            Ts += mpi.time();

            const auto time = std::chrono::steady_clock::now();
            double result = 0;
            for (double x = a(); x <= b(); x += step)
                result += mpi.integral(c(), d(x), x, epsilon) * step;
            T1 += std::chrono::duration<double>(std::chrono::steady_clock::now() - time).count();

            if (i == 0)
            {
                std::cout << "result with " << mpi.get_size() << " processes: " << mpi.get_result() << std::endl;
                std::cout << "result with one processes: " << result << std::endl;
            }
            if (i == iteration_count - 1)
            {
                std::cout << "Ts: " << Ts / iteration_count << "s" << std::endl;
                std::cout << "T1: " << T1 / iteration_count << "s" << std::endl;
                std::cout << "acceleration: " << T1 / Ts << std::endl;
            }
        }
    }
    

    MPI_Finalize();
    return 0;
}