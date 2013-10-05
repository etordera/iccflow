#include "iccflowapp.h"

/**
* IccFlow main function
*
* Batch ICC profile conversion 
*
*/
int main(int argc, char** argv) {
	IccFlowApp app(argc,argv);
	int result = app.run();
	return result; 
}

