#pragma once

#ifndef BOOST_OVERRIDE
#define BOOST_OVERRIDE override
#endif

#ifndef BOOST_ASSERT
#define BOOST_ASSERT(expr)
#endif

#include "boost-vectorstream.hpp"

template <class CharVector>
using ovectorstream = boost::interprocess::basic_ovectorstream<CharVector, std::char_traits<typename CharVector::value_type>>;

template <class CharVector>
using ivectorstream = boost::interprocess::basic_ivectorstream<CharVector, std::char_traits<typename CharVector::value_type>>;
