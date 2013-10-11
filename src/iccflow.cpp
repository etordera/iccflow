#include "iccflowapp.h"

/**
 * Definition of global constants
 */

#if defined _WIN32 || defined _WIN64
extern const std::string g_slash = "\\";
#else
extern const std::string g_slash = "/";
#endif

/**
* IccFlow main function
*
* Batch ICC profile conversion 
*
*/
int main(int argc, char** argv) {
	IccFlowApp app(argc,argv);
	return app.run();
}

