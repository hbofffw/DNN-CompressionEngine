
#include "CompressionEngineWrapper.h"

namespace bp = boost::python;


BOOST_PYTHON_MODULE(CompressionEngine)
{
	Py_Initialize();
	bn::initialize();

	// register exception translators
	bp::register_exception_translator<ArgumentError>(&TranslateArgumentError);
}