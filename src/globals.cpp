/**
 * Definition of global constants
 */

#include <string>

extern const std::string g_version = "1.0";

#if defined _WIN32 || defined _WIN64
extern const std::string g_slash = "\\";
#else
extern const std::string g_slash = "/";
#endif
