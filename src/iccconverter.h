#ifndef ICCCONVERTER_H
#define ICCCONVERTER_H

#include <setjmp.h>
#include <jpeglib.h>
#include "iccprofile.h"

/**
 * Custor error manager struct for handling
 * libjpeg errors
 */
struct my_error_mgr {
  struct jpeg_error_mgr jerr;	/**< Standard libjpeg error manager */
  jmp_buf setjmp_buffer;		/**< Additional field for controlling return point on error */
};
METHODDEF(void) my_error_exit(j_common_ptr cinfo);

/**
 * IccConverter objects manage ICC color transforms on JPEG files
 */
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
		void setBlackPointCompensation(bool);
		void setOptimization(bool);
		bool convert(const std::string&);
		void setVerboseOutput(bool);

	private:
		std::string m_inputFolder;				/**< Path to input folder of source images */
		std::string m_outputFolder;				/**< Path to output folder for processed images */
		IccProfile m_outputProfile;				/**< Output ICC profile for color transform */
		IccProfile m_defaultRGBProfile;			/**< Default input RGB ICC profile */
		IccProfile m_defaultCMYKProfile;		/**< Default input CMYK ICC profile */
		IccProfile m_defaultGrayProfile;		/**< Default input Grayscale ICC profile */
		std::string m_outputProfileName;		/**< Name of output ICC profile */
		std::string m_defaultRGBProfileName;	/**< Name of default input RGB ICC profile */
		std::string m_defaultCMYKProfileName;	/**< Name of default input CMYK ICC profile */
		std::string m_defaultGrayProfileName;	/**< Name of default input Grayscale ICC profile */
		int m_intent;							/**< Rendering intent for color transform */
		int m_jpegQuality;						/**< Quality parameter used for output JPEG compression */
		bool m_blackPointCompensation;			/**< Wether to apply Black Point Compensation or not */
		bool m_enableOptimization;			/**< Wether optimitzation is enabled for color transform calculations */
		bool m_verbose;							/**< Verbose output enabled */
		jpeg_decompress_struct m_dinfo;			/**< Info struct for JPEG decompression */
		my_error_mgr m_derr;					/**< Data for JPEG decompression error management */
		jpeg_compress_struct m_cinfo;			/**< Info struct for JPEG compression */
		my_error_mgr m_cerr;					/**< Data for JPEG compression error management */

		bool loadOutputProfile();
		bool loadDefaultRGBProfile();
		bool loadDefaultCMYKProfile();
		bool loadDefaultGrayProfile();
		void embedIccProfile(const IccProfile&,jpeg_compress_struct*);
		std::string removeTrailingSlash(const std::string);
};

#endif
