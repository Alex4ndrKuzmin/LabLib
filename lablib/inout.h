#include <iterator>
#include <iostream>
#include <fstream>
#include <random>

#define NAMED_VARIABLE(x) #x << " = "
#define INITIALIZE(x) \
	std::cout << NAMED_VARIABLE(x); \
	std::cin >> x;

template <class Type>
concept Iterator = std::_Is_iterator_v<Type>;

template <class Type>
concept Printable = !requires(Type type) { typename Type::iterator; } && requires(Type type) { std::cout << type; };

template <class Type>
concept ConsoleInitializable = requires(Type type) { std::cin >> type; };

template <class Type>
concept ConsoleInOut = requires(Type type) { std::cin >> type; } && requires(Type type) { std::cout << type; };

template <class Type>
concept Iterable = requires(Type type) { typename Type::iterator; };

template <class Type>
concept ArithmeticCollection = std::is_arithmetic_v<Type> && requires(Type type) { typename Type::iterator; };

template <class Type>
concept Arithmetic = std::is_arithmetic_v<Type>;

template <class Type>
concept Real = std::is_floating_point_v<Type>;

template <class Type>
concept Integer = std::is_integral_v<Type>;

class inout
{
public:

	template <Iterable Container>
	static inline void out_linear_to_matrix(Container& container, int rows_count, int cols_count, std::string delimiter = " ")
	{
		int common_counter = 0;
		int horizontal_counter = 0;
		for (auto it = container.begin(); it != container.end(); it++)
			if (common_counter < rows_count * cols_count)
			{
				out(*it, delimiter);
				common_counter++;
				horizontal_counter++;
				if (horizontal_counter >= cols_count)
				{
					horizontal_counter = 0;
					std::cout << std::endl;
				}
			}
			else
			{
				break;
			}
	}

	template <Printable Element>
	static inline void out(Element element)
	{
		std::cout << element;
	}

	template <Iterable Element>
	static inline void out(Element element)
	{
		out_collection_range(element.begin(), element.end());
	}

	template <Printable Element>
	static inline void out(Element element, std::string end)
	{
		std::cout << element << end;
	}

	template <Iterable Element>
	static inline void out(Element element, std::vector<std::string> delimiters)
	{
		out_collection_range(element.begin(), element.end(), delimiters);
	}

	template <Iterator Iterator>
	static inline void out_collection_range(Iterator first, Iterator last, std::string delimiter = " ")
	{
		//_STL_ASSERT(first < last, "");
		for (Iterator it = first; it != last; ++it)
		{
			if (it != first)
			{
				std::cout << delimiter;
			}
			out(*it);
		}
	}

	template <Iterator Iterator>
	static inline void out_collection_range(Iterator first, Iterator last, std::vector<std::string> delimiters)
	{
		_STL_ASSERT(first < last, "");

		std::string delimiter = " ";
		if (delimiters.size() > 0)
		{
			delimiter = *delimiters.begin();
			delimiters.erase(delimiters.begin());
		}
		
		for (Iterator it = first; it != last; ++it)
		{
			if (it != first)
			{
				std::cout << delimiter;
			}
			out(*it, delimiters);
		}
	}

	template <ConsoleInOut Type>
	static inline void initialize(Type& var)
	{
		INITIALIZE(var);
	}

	template <Iterable Collection, typename ArithmetcType>
	static inline void initialize_random(Collection& collection, ArithmetcType a, ArithmetcType b)
	{
		for (auto it = collection.begin(); it != collection.end(); ++it)
		{
			initialize_random(*it, a, b);
		}
	}

	template <Arithmetic Number>
	static inline void initialize_random(Number& num, Number a, Number b)
	{
		num = generate_number(a, b);
	}

	template <Integer Type>
	static inline Type generate_number(Type a, Type b)
	{
		std::mt19937 engine(device());
		std::uniform_int_distribution<Type> uniform_dist(a, b);
		return uniform_dist(engine);
	}

	template <Real Type>
	static inline Type generate_number(Type a, Type b)
	{
		std::mt19937 engine(device());
		std::uniform_real_distribution<Type> float_uniform_dist(a, b);
		return float_uniform_dist(engine);
	}

	static inline bool open_file(std::string &path, std::ios_base::openmode mode = 3)
	{
		file_iostream.open(path, mode);
		return file_iostream.is_open();
	}

	static inline void close_file()
	{
		if (file_iostream.is_open())
			file_iostream.close();
	}

private:

	static std::random_device device;

	static std::fstream file_iostream;

	template <Printable Element>
	static inline void out(Element element, std::vector<std::string> delimiters)
	{
		std::cout << element << (delimiters.size() > 0 ? *delimiters.erase(delimiters.begin()) : " ");
	}
};

std::random_device inout::device = std::random_device();


