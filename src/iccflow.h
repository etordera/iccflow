#include <string>

#if defined _WIN32 || defined _WIN64
extern const std::string g_slash = "\\";
#else
extern const std::string g_slash = "/";
#endif
