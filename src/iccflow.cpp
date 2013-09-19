#include "iccflowapp.h"

/**
* IccFlow
*
* Batch icc profile conversion 
*
*/
int main(int argc, char** argv) {
	IccFlowApp app(argc,argv);
	return app.run();
}

