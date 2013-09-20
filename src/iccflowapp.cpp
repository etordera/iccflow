#include "iccflowapp.h"
#include "iccconverter.h"
#include <iostream>
#include <dirent.h>
#include <algorithm>

/**
 * Main constructor. Gets the command line arguments for running
 * the application. 
 *
 * @param argc Command line argument count
 * @param argv Command line arguments array
 */
IccFlowApp::IccFlowApp(int argc, char** argv)
:m_argc(argc),
 m_argv(argv)
{
	if (m_argc < 0) {
		m_argc = 0;
	}
	m_inputFolder.empty();
	m_outputFolder.empty();
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
	m_inputFolder.empty();
	m_outputFolder.empty();

	// Traverse and analyze arguments
	bool helpShown = false;
	for (int i=0; i<m_argc; i++) {
		if (std::string(m_argv[i]) == "-h") {
			std::cout << "Help is still not available (yet!)" << std::endl;
			helpShown = true;
		} else if (std::string(m_argv[i]) == "-i") {
			if (++i < m_argc) {
				m_inputFolder = std::string(m_argv[i]);
			}
		} else if (std::string(m_argv[i]) == "-o") {
			if (++i < m_argc) {
				m_outputFolder = std::string(m_argv[i]);
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
	}

	return success;
}

