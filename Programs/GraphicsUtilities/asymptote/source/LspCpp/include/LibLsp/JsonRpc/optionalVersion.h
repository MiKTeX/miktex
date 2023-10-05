#ifndef boost
#if __cplusplus < 201703L

#include <boost/optional.hpp>
using boost::optional;

#else

#include <optional>
using std::optional;

#endif
#endif
