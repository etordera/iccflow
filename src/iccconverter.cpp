#include "iccconverter.h"
#include <iostream>

/**
 * IccConverter objects manage the ICC profile color conversion
 * of JPEG images
 *
 */
IccConverter::IccConverter() {
	m_file.empty();
	m_inputFolder.empty();
	m_outputFolder.empty();
	m_outProfile.empty();
}

void IccConverter::setInputFolder(const std::string& inputFolder) {
	m_inputFolder = inputFolder;
}

void IccConverter::setOutputFolder(const std::string& outputFolder) {
	m_outputFolder = outputFolder;
}

bool IccConverter::convert(const std::string& file) {
	m_file = file;

	std::cout << "Processing " << m_file << ": ";
	std::cout << "Done." << std::endl;

	return true;
}
