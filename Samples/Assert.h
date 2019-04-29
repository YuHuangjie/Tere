#ifndef ASSERT_H
#define ASSERT_H

#ifndef ASSERT

#include <stdexcept>
#include <sstream>

#define ASSERT(a) do { \
	if (!a) { \
		std::ostringstream oss;	\
		oss << __FILE__ << " : " << __LINE__ << " return false";	\
		throw std::runtime_error(oss.str());	\
	}\
} while (0)

#endif

#endif /* ASSERT_H */
