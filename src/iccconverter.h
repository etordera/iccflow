#ifndef ICCCONVERTER_H
#define ICCCONVERTER_H

#include <jpeglib.h>
#include "iccprofile.h"

class IccConverter {

	public:
		IccConverter();
		~IccConverter();
		void setInputFolder(const std::string&);
		void setOutputFolder(const std::string&);
		bool setOutputProfile(const std::string&);
		bool setDefaultRGBProfile(const std::string&);
		bool setDefaultCMYKProfile(const std::string&);
		bool setDefaultGrayProfile(const std::string&);
		bool setIntent(int);
		bool convert(const std::string&);

	private:
		std::string m_file;
		std::string m_inputFolder;
		std::string m_outputFolder;
		IccProfile m_outputProfile;
		IccProfile m_defaultRGBProfile;
		IccProfile m_defaultCMYKProfile;
		IccProfile m_defaultGrayProfile;
		int m_intent;
		jpeg_decompress_struct m_dinfo;
		jpeg_error_mgr m_derr;
		jpeg_compress_struct m_cinfo;
		jpeg_error_mgr m_cerr;

		void embedIccProfile(const IccProfile&,jpeg_compress_struct*);
		std::string removeTrailingSlash(const std::string);
};

#endif
