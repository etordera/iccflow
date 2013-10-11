#ifndef ICCFLOWAPP_H
#define ICCFLOWAPP_H

#include <string>

/**
 * IccFlowApp class implements the iccflow application
 */
class IccFlowApp {

	private:
		int m_argc;			/**< Command line argument count */
		char** m_argv;		/**< Command line arguments */
		std::string m_inputFolder;		/**< Folder where original images are located */
		std::string m_outputFolder;		/**< Folder where processed images will be generated */
		std::string m_outputProfile;	/**< Path to file with destination ICC Profile after conversion */
		std::string m_defaultRGBProfile;	/**< Path to file with default RGB ICC Profile when none is found in source image */
		std::string m_defaultCMYKProfile;	/**< Path to file with default GCMY ICC Profile when none is found in source image */
		std::string m_defaultGrayProfile;	/**< Path to file with default Grayscale ICC Profile when none is found in source image */
		int m_intent;		/**< Rendering intent to be used during color conversion */
		int m_jpegQuality;	/**< Quality parameter for output JPEG compression (0-100) */

		bool parseArguments();
		void showHelp();
		bool copyFile(const std::string&,const std::string&);

	public:
		IccFlowApp(int,char**);
		int run();
};

#endif
