#ifndef ICCCONVERTER_H
#define ICCCONVERTER_H

#include <jpeglib.h>

class IccConverter {

	public:
		IccConverter();
		~IccConverter();
		void setInputFolder(const std::string&);
		void setOutputFolder(const std::string&);
		void setOutputProfile(const std::string&);
		void setDefaultRGBProfile(const std::string&);
		void setDefaultCMYKProfile(const std::string&);
		void setDefaultGrayProfile(const std::string&);
		bool convert(const std::string&);

	private:
		std::string m_file;
		std::string m_inputFolder;
		std::string m_outputFolder;
		IccProfile m_outputProfile;
		IccProfile m_defaultRGBProfile;
		IccProfile m_defaultCMYKProfile;
		IccProfile m_defaultGrayProfile;
		jpeg_decompress_struct m_dinfo;
		jpeg_error_mgr m_derr;
		jpeg_compress_struct m_cinfo;
		jpeg_error_mgr m_cerr;
};

#endif
