#ifndef ICCCONVERTER_H
#define ICCCONVERTER_H

#include <string>

class IccConverter {

	public:
		IccConverter();
		void setInputFolder(const std::string&);
		void setOutputFolder(const std::string&);
		void setOutputProfile(const std::string&);
		bool convert(const std::string&);

	private:
		std::string m_file;
		std::string m_inputFolder;
		std::string m_outputFolder;
		std::string m_outProfile;
};

#endif
