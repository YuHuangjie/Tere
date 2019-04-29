#ifndef ERROR_H
#define ERROR_H

#include <sstream>

#define THROW_ON_ERROR(...) do {\
	char msg[256], err[256];	\
	sprintf_s(msg, 256, __VA_ARGS__);	\
	sprintf_s(err, 256, "[ERROR] %s at %s, Line: %d\n", msg, __FILE__, __LINE__);\
	throw std::runtime_error(err);	\
} while(0)

#define RETURN_ON_ERROR(...) do {\
	char msg[256];	\
	sprintf_s(msg, 256, __VA_ARGS__);	\
	printf_s("[ERROR] %s at %s, Line: %d\n", msg, __FILE__, __LINE__);\
	return 0;	\
} while(0)

#endif /* ERROR_H */
