#ifndef BOOST_STD_CONFIGURATION_H
#define BOOST_STD_CONFIGURATION_H

#cmakedefine01 STD_SHARED_MUTEX_FOUND
#cmakedefine01 Boost_SHARED_MUTEX_FOUND
#cmakedefine01 STD_FILESYSTEM_FOUND
#cmakedefine01 Boost_FILESYSTEM_FOUND
#cmakedefine01 STD_EXPERIMENTAL_FILESYSTEM_FOUND

#if STD_SHARED_MUTEX_FOUND
#include <shared_mutex>
namespace ${NAMESPACE_ALIAS} {
    typedef std::shared_mutex shared_mutex;
    template <typename T>
    using shared_lock = std::shared_lock<T>;
}
#elif Boost_SHARED_MUTEX_FOUND
#include <boost/thread/shared_mutex.hpp>
namespace ${NAMESPACE_ALIAS} {
    typedef boost::shared_mutex shared_mutex;
    template <typename T>
    using shared_lock = boost::shared_lock<T>;
}
#endif

#if STD_FILESYSTEM_FOUND
#include <filesystem>
namespace ${NAMESPACE_ALIAS} {
namespace filesystem {
    typedef std::filesystem::path path;
}
}
#elif STD_EXPERIMENTAL_FILESYSTEM_FOUND
#include <experimental/filesystem>
namespace ${NAMESPACE_ALIAS} {
namespace filesystem {
    typedef std::experimental::filesystem::path path;
}
}
#elif Boost_FILESYSTEM_FOUND
#include <boost/filesystem.hpp>
namespace ${NAMESPACE_ALIAS} {
namespace filesystem {
    typedef boost::filesystem::path path;
}
}
#endif

#endif /* BOOST_STD_CONFIGURATION_H */
