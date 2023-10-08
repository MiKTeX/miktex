#ifndef LOG4CXX_MAKE_UNIQUE_H
#define LOG4CXX_MAKE_UNIQUE_H
#include <memory>

#cmakedefine01 STD_MAKE_UNIQUE_FOUND

#if !STD_MAKE_UNIQUE_FOUND
namespace std
{
	template <typename T, typename ...Args>
unique_ptr<T> make_unique( Args&& ...args )
{
	return unique_ptr<T>( new T( forward<Args>(args)... ) );
}
} // namespace std
#endif

#endif /* LOG4CXX_MAKE_UNIQUE_H */
