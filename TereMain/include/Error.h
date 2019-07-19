#ifndef ERROR_H
#define ERROR_H

#include <sstream>

#define THROW_ON_ERROR(...) do {\
	char temp_msg[4096], temp_err[4096];	\
	sprintf_s(temp_msg, 4096, __VA_ARGS__);	\
	sprintf_s(temp_err, 4096, "[ERROR] %s at %s, Line: %d\n", temp_msg, __FILE__, __LINE__);\
	throw std::runtime_error(temp_err);	\
} while(0)

#define RETURN_ON_ERROR(...) do {\
	char temp_msg[4096];	\
	sprintf_s(temp_msg, 4096, __VA_ARGS__);	\
	printf_s("[ERROR] %s at %s, Line: %d\n", temp_msg, __FILE__, __LINE__);\
	return 0;	\
} while(0)

#endif /* ERROR_H */
