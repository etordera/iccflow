#include "iccflowapp.h"
#include "iccconverter.h"
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <lcms2.h>

/**
 * Main constructor. Gets the command line arguments for running
 * the application. 
 *
 * @param argc Command line argument count
 * @param argv Command line arguments array
 */
IccFlowApp::IccFlowApp(int argc, char** argv)
:m_argc(argc),
 m_argv(argv),
 m_intent(INTENT_PERCEPTUAL),
 m_jpegQuality(85)
{
	if (m_argc < 0) {
		m_argc = 0;
	}
	m_inputFolder.clear();
	m_outputFolder.clear();
	m_outputProfile.clear();
	m_defaultRGBProfile.clear();
	m_defaultCMYKProfile.clear();
	m_defaultGrayProfile.clear();
}


/**
 * Runs the application.
 *
 * @return 0 on success, a non-zero error code otherwise
 */
int IccFlowApp::run() {
	// Parse command line arguments
	if (!parseArguments()) {
		return 1;
	}

	// Prepare the ICC profile converter
	IccConverter converter;
	converter.setInputFolder(m_inputFolder);
	converter.setOutputFolder(m_outputFolder);
	converter.setOutputProfile(m_outputProfile);
	converter.setDefaultRGBProfile(m_defaultRGBProfile);
	converter.setDefaultCMYKProfile(m_defaultCMYKProfile);
	converter.setDefaultGrayProfile(m_defaultGrayProfile);
	converter.setIntent(m_intent);
	converter.setJpegQuality(m_jpegQuality);

	// Open input folder
	DIR* dir = NULL;
	dir = opendir(m_inputFolder.c_str());
	if (dir == NULL) {
		std::cerr << "Failed to open input folder: " << m_inputFolder << std::endl;
		return 2;
	}

	// Traverse directory and process JPEG files
	dirent* ent = NULL;
	std::string file;
	bool success = true;
	while ((ent = readdir(dir))) {
		if (ent->d_type == DT_REG) {
			file = ent->d_name;
			transform(file.begin(),file.end(),file.begin(),::tolower);
			if ((file.rfind(".jpg") == file.size()-4) || (file.rfind(".jpeg") == file.size()-5)) {
				file = ent->d_name;
				if (!converter.convert(file)) {
					success = false;
				}
			}
		}
	}
	closedir(dir);

	// Return exit code
	return (success ? 0 : 3);
}


/**
* Parses command line arguments and sets the corresponding parameters
* in the application object.
*
* @return true if valid parameters where parsed, false otherwise
*
*/
bool IccFlowApp::parseArguments() {
	// Initialize app parameters
	m_inputFolder.clear();
	m_outputFolder.clear();
	m_outputProfile.clear();
	m_defaultRGBProfile.clear();
	m_defaultCMYKProfile.clear();
	m_defaultGrayProfile.clear();
	m_intent = INTENT_PERCEPTUAL;
	m_jpegQuality = 85;

	// Traverse and analyze arguments
	bool helpShown = false;
	for (int i=0; i<m_argc; i++) {
		if (std::string(m_argv[i]) == "-h") {
			showHelp();
			helpShown = true;
		} else if (std::string(m_argv[i]) == "-i") {
			if (++i < m_argc) {
				m_inputFolder = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-o") {
			if (++i < m_argc) {
				m_outputFolder = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-p") {
			if (++i < m_argc) {
				m_outputProfile = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-prgb") {
			if (++i < m_argc) {
				m_defaultRGBProfile = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-pcmyk") {
			if (++i < m_argc) {
				m_defaultCMYKProfile = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-pgray") {
			if (++i < m_argc) {
				m_defaultGrayProfile = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-c") {
			if (++i < m_argc) {
				m_intent = atoi(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-q") {
			if (++i < m_argc) {
				m_jpegQuality = atoi(m_argv[i]);
			}
		}	
	}

	bool success = false;
	if (!helpShown) {
		// Check if required parameters have been set
		success = true;
		if (m_inputFolder == "") {
			std::cerr << "Input folder required, please specify with -i option" << std::endl;
			success = false;
		}
		if (m_outputFolder == "") {
			std::cerr << "Output folder required, please specify with -o option" << std::endl;
			success = false;
		}
		if ((m_jpegQuality < 0) || (m_jpegQuality > 100)) {
			std::cerr << "Invalid JPEG quality value (should be 0 to 100)" << std::endl;
			success = false;
		}
	}

	return success;
}

/**
 * Output help message showing command-line options
 */
void IccFlowApp::showHelp() {
	std::cout << "iccflow -i inputFolder -o outputFolder [options]" << std::endl;
	std::cout << "Performs ICC color transformation on JPEG files." << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Mandatory parameters:" << std::endl;
	std::cout << "  -i inputFolder:   Source folder containing the original JPEG images." << std::endl;
	std::cout << "  -o outputFolder:  Destination folder where converted images will be saved." << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;
	std::cout << "Optional parameters:" << std::endl;
	std::cout << "  -p outputProfile:   Output profile for the color transformation (path to .icc/.icm file)." << std::endl;
	std::cout << "                      Defaults to sRGB" << std::endl;
	std::cout << std::endl;
	std::cout << "  -prgb rgbProfile:   Default RGB input profile when none is found in the source JPEG file (path to .icc/.icm file)." << std::endl;
	std::cout << "                      Defaults to sRGB" << std::endl;
	std::cout << std::endl;
	std::cout << "  -pcmyk cmykProfile: Default CMYK input profile when none is found in the source JPEG file (path to .icc/.icm file)." << std::endl;
	std::cout << "                      Defaults to FOGRA27" << std::endl;
	std::cout << std::endl;
	std::cout << "  -pgray grayProfile: Default Grayscale input profile when none is found in the source JPEG file (path to .icc/.icm file)." << std::endl;
	std::cout << "                      Defaults to D50 Gamma-2.2 Grayscale" << std::endl;
	std::cout << std::endl;
	std::cout << "  -c intent_code:     Rendering intent to be used during color transformation. Possible values:" << std::endl;
	std::cout << "                      0: Perceptual (DEFAULT)" << std::endl;
	std::cout << "                      1: Relative colorimetric" << std::endl;
	std::cout << "                      2: Saturation" << std::endl;
	std::cout << "                      3: Absolute colorimetric" << std::endl;
	std::cout << std::endl;
	std::cout << "  -q jpegQuality:    JPEG quality level for output compression (0-100, defaults to 85)" << std::endl; 
}
