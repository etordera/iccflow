#ifndef ICCFLOWAPP_H
#define ICCFLOWAPP_H

#include <string>

class IccFlowApp {

	private:
		int m_argc;
		char** m_argv;
		std::string m_inputFolder;
		std::string m_outputFolder;
		std::string m_outputProfile;
		std::string m_defaultRGBProfile;
		std::string m_defaultCMYKProfile;
		std::string m_defaultGrayProfile;

		bool parseArguments();

	public:
		IccFlowApp(int,char**);
		int run();
};


#endif
