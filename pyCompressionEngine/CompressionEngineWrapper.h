#pragma once

#define BOOST_PYTHON_STATIC_LIB
#define BOOST_NUMPY_STATIC_LIB

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>

namespace bp = boost::python;
namespace bn = boost::python::numpy;

class ArgumentError : public std::exception {
public:
	ArgumentError() : msg("Unknown error.") {};
	ArgumentError(std::string str) : msg(str) {};
	~ArgumentError() {};

	const char* what() const throw() { return msg.c_str(); }

private:
	std::string msg;
};

// translator for the boost::python
void TranslateArgumentError(const ArgumentError& e);

class CompressionEngineWrapper
{
public:
	CompressionEngineWrapper();
	~CompressionEngineWrapper();
};

