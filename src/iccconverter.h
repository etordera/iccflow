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
		void setOutputProfile(const std::string&);
		void setDefaultRGBProfile(const std::string&);
		void setDefaultCMYKProfile(const std::string&);
		void setDefaultGrayProfile(const std::string&);
		bool setIntent(int);
		bool setJpegQuality(int);
		bool convert(const std::string&);

	private:
		std::string m_inputFolder;
		std::string m_outputFolder;
		IccProfile m_outputProfile;
		IccProfile m_defaultRGBProfile;
		IccProfile m_defaultCMYKProfile;
		IccProfile m_defaultGrayProfile;
		std::string m_outputProfileName;
		std::string m_defaultRGBProfileName;
		std::string m_defaultCMYKProfileName;
		std::string m_defaultGrayProfileName;
		int m_intent;
		int m_jpegQuality;		/**< Quality parameter used for output JPEG compression */
		jpeg_decompress_struct m_dinfo;
		jpeg_error_mgr m_derr;
		jpeg_compress_struct m_cinfo;
		jpeg_error_mgr m_cerr;

		bool loadOutputProfile();
		bool loadDefaultRGBProfile();
		bool loadDefaultCMYKProfile();
		bool loadDefaultGrayProfile();
		void embedIccProfile(const IccProfile&,jpeg_compress_struct*);
		std::string removeTrailingSlash(const std::string);
};

#endif
