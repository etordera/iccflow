#include <iostream>
#include <iomanip>
extern "C" {
#include <jpeglib.h>
}
#include <fstream>
#include <cstring>
#include <cstdio>
#include <setjmp.h>
#include "globals.h"
#include "iccprofile.h"
#include "iccconverter.h"
#include "icc_fogra27.h"

/**
 * IccConverter objects manage the ICC profile color conversion
 * of JPEG images
 *
 */
IccConverter::IccConverter()
:m_intent(0),
 m_jpegQuality(85),
 m_blackPointCompensation(true),
 m_enableOptimization(true),
 m_verbose(false)
{
	// Initialize variables
	m_inputFolder.clear();
	m_outputFolder.clear();
	m_outputProfileName.clear();
	m_defaultRGBProfileName.clear();
	m_defaultCMYKProfileName.clear();
	m_defaultGrayProfileName.clear();

	// Initialize JPEG decompress objects
	m_dinfo.err = jpeg_std_error(&m_derr.jerr);
  	m_derr.jerr.error_exit = my_error_exit;
	jpeg_create_decompress(&m_dinfo);

	// Initialize JPEG compress objects
	m_cinfo.err = jpeg_std_error(&m_cerr.jerr);
  	m_cerr.jerr.error_exit = my_error_exit;
	jpeg_create_compress(&m_cinfo);
}

/**
 * Destructor frees resources associated with JPEG
 * compression and decompression structs
 */
IccConverter::~IccConverter() {
	// Destroy JPEG compress/decompress objects
	jpeg_destroy_decompress(&m_dinfo);
	jpeg_destroy_compress(&m_cinfo);
}

/**
 * Sets the source folder where the application will look for JPEG
 * images to process.
 *
 * @param[in] inputFolder Path to the source folder
 */
void IccConverter::setInputFolder(const std::string& inputFolder) {
	m_inputFolder = removeTrailingSlash(inputFolder);
}

/**
 * Sets the destination folder where processed images will 
 * be output. 
 *
 * @param[out] outputFolder Path to the destination folder
 */
void IccConverter::setOutputFolder(const std::string& outputFolder) {
	m_outputFolder = removeTrailingSlash(outputFolder);
}

/**
 * Sets the output profile for color transforms carried on by the
 * IccConverter object.
 * Can load the profile from an ICC Profile file or a JPEG file
 * (extracts embedded profile).
 * Defaults to sRGB
 * 
 * @param[in] profileName Path to file containing the ICC profile to assign
 */
void IccConverter::setOutputProfile(const std::string& profileName){
	m_outputProfileName = profileName;
}

/**
 * Sets the default input profile for source files in the RGB color space. 
 * Can load the profile from an ICC Profile file or a JPEG file
 * (extracts embedded profile).
 * Defaults to sRGB
 * 
 * @param[in] profileName Path to file containing the ICC profile to assign
 */
void IccConverter::setDefaultRGBProfile(const std::string& profileName){
	m_defaultRGBProfileName = profileName;
}

/**
 * Sets the default input profile for source files in the CMYK color space. 
 * Can load the profile from an ICC Profile file or a JPEG file
 * (extracts embedded profile).
 * Defaults to FOGRA27
 * 
 * @param[in] profileName Path to file containing the ICC profile to assign
 */
void IccConverter::setDefaultCMYKProfile(const std::string& profileName){
	m_defaultCMYKProfileName = profileName;
}

/**
 * Sets the default input profile for source files in Grayscale color space. 
 * Can load the profile from an ICC Profile file or a JPEG file
 * (extracts embedded profile).
 * Defaults to D50 Gamma-2.2 Grayscale 
 * 
 * @param[in] profileName Path to file containing the ICC profile to assign
 */
void IccConverter::setDefaultGrayProfile(const std::string& profileName){
	m_defaultGrayProfileName = profileName;
}


/**
 * Loads the configured output profile to be used in color transforms.
 * If loading fails, defaults to sRGB. 
 *
 * @return true if the configured ICC profile was loaded correctly, false if
 * default had to be loaded instead
 */
bool IccConverter::loadOutputProfile(){
	m_outputProfile.loadFromFile(m_outputProfileName);
	if (!m_outputProfile.isValid()) {
		m_outputProfile.loadSRGB();	
		return false;
	}

	return true;
}

/**
 * Loads the configured default input profile for source files in the
 * RGB color space. If loading fails, defaults to sRGB. 
 *
 * @return true if the configured ICC profile was loaded correctly, false if
 * default had to be loaded instead
 */
bool IccConverter::loadDefaultRGBProfile(){
	m_defaultRGBProfile.loadFromFile(m_defaultRGBProfileName);
	if (!m_defaultRGBProfile.isValid()) {
		m_defaultRGBProfile.loadSRGB();	
		return false;
	}

	return true;
}

/**
 * Loads the configured default input profile for source files in the
 * CMYK color space. If loading fails, defaults to FOGRA27. 
 *
 * @return true if the configured ICC profile was loaded correctly, false if
 * default had to be loaded instead
 */
bool IccConverter::loadDefaultCMYKProfile(){
	m_defaultCMYKProfile.loadFromFile(m_defaultCMYKProfileName);
	if (!m_defaultCMYKProfile.isValid()) {
		m_defaultCMYKProfile.loadFromMem((char*)iccFOGRA27,iccFOGRA27_size);
		return false;
	}

	return true;
}

/**
 * Loads the configured default input profile for source files in the
 * Grayscale color space. If loading fails, defaults to D50 Gamma 2.2 Grayscale. 
 *
 * @return true if the configured ICC profile was loaded correctly, false if
 * default had to be loaded instead
 */
bool IccConverter::loadDefaultGrayProfile() {
	m_defaultGrayProfile.loadFromFile(m_defaultGrayProfileName);
	if (!m_defaultGrayProfile.isValid()) {
		m_defaultGrayProfile.loadGray(2.2);
		return false;
	}

	return true;
}

/**
 * Sets the rendering intent for the color conversion.
 * Possible values:
 *    0: Perceptual
 *    1: Relative colorimetric
 *    2: Saturation
 *    3: Absolute colorimetric
 *
 * @param[in] intent Rendering intent integer identifier, as specified in LittleCMS library
 * @return true if a valid intent has been set, false otherwise
 */
bool IccConverter::setIntent(int intent) {
	bool success = true;
	if ((intent>=0) && (intent<=3)) {
		m_intent = intent;
	} else {
		success = false;

	} 

	return success;
}

/**
 * Sets the quality parameter used for output JPEG compression. Valid values in the range
 * from 0 to 100. 
 *
 * @param[in] jpegQuality JPEG quality parameter (integer from 0 to 100)
 * @return true if a valid parameter has been set, false otherwise
 */
bool IccConverter::setJpegQuality(int jpegQuality) {
	bool success = false;
	if ((jpegQuality>=0) && (jpegQuality<=100)) {
		m_jpegQuality = jpegQuality;
		success = true;
	} 

	return success;
}


/**
 * Enable or disable Black Point Compensation for color conversions
 * 
 * @param[in] blackPointCompensation true for enabling Black Point Compensation, false for disabling
 */
void IccConverter::setBlackPointCompensation(bool blackPointCompensation) {
	m_blackPointCompensation = blackPointCompensation;
}


/**
 * Enable or disable optimization for calculations during color transforms
 * 
 * @param[in] enableOptimization true for enabling optimization, false for disabling
 */
void IccConverter::setOptimization(bool enableOptimization) {
	m_enableOptimization = enableOptimization;
}


/**
 * Enable or disable verbose output during processing.
 * 
 * @param[in] verbose true for enabling verbose output, false for disabling
 */
void IccConverter::setVerboseOutput(bool verbose) {
	m_verbose = verbose;
}



/**
 * Performs ICC color conversion in a JPEG file 
 *
 * The file will be looked for in the source folder previously set
 * by calling (@ref IccConverter#setInputFolder) method. Processed JPEG will be saved
 * with the same name in the output folder previously set by calling
 * (@ref IccConverter#setOutputFolder) method.
 *
 * @param[in] file Name of the file to process 
 * @return true if conversion is successful, false otherwise
 * @see IccConverter#setInputFolder
 * @see IccConverter#setOutputFolder
 */
bool IccConverter::convert(const std::string& file) {
	// Generate input file name
	std::string theFile = m_inputFolder + g_slash + file;
	std::cout << file << ": ";

	// Check valid output profile 
	if (!m_outputProfile.isValid()) {
		loadOutputProfile();		
	}

	FILE* f = NULL;
	FILE* fOut = NULL;
	std::string outputFileTemp("");
	JSAMPLE* buffer_in[1];
	buffer_in[0] = NULL;
	JSAMPLE* buffer_out[1];
	buffer_out[0] = NULL;
	cmsHTRANSFORM hTransform = NULL; 
	try {

		// Handle errors in the JPEG decompression library
		if (setjmp(m_derr.setjmp_buffer)) {
			throw 1;
		}
		// Handle errors in the JPEG compression library
		if (setjmp(m_cerr.setjmp_buffer)) {
			throw 2;
		}

		// Open source file
		if ((f = fopen(theFile.c_str(), "rb")) == NULL) {
			std::cerr << "Failed to open " << theFile << std::endl;
			return false;
		}
		jpeg_stdio_src(&m_dinfo,f);

		// Open temp output file
		std::string outputFile = m_outputFolder + g_slash + file;
		outputFileTemp = outputFile + ".tmp";
		if ((fOut = fopen(outputFileTemp.c_str(), "wb")) == NULL) {
			std::cerr << "Failed to write  " << outputFile << std::endl;
			fclose(f);
			return false;
		}
		jpeg_stdio_dest(&m_cinfo,fOut);

		// Start input decompression
		jpeg_read_header(&m_dinfo, TRUE);
		jpeg_start_decompress(&m_dinfo);

		// Determine input profile
		IccProfile inputProfile;
		if (!inputProfile.loadFromFile(theFile)) {
			switch (m_dinfo.out_color_space) {
				case JCS_GRAYSCALE:
					if (!m_defaultGrayProfile.isValid()) {
						loadDefaultGrayProfile();	
					}
					inputProfile = m_defaultGrayProfile;
					break;
				case JCS_CMYK:
					if (!m_defaultCMYKProfile.isValid()) {
						loadDefaultCMYKProfile();	
					}
					inputProfile = m_defaultCMYKProfile;
					break;
				case JCS_RGB:
					if (!m_defaultRGBProfile.isValid()) {
						loadDefaultRGBProfile();	
					}
					inputProfile = m_defaultRGBProfile;
					break;
				default:
					std::cerr << "Unsupported color space" << std::endl;
					fclose(f);
					fclose(fOut);
					jpeg_finish_decompress(&m_dinfo);
					return false;	
			}
		}
		std::cout << "(" << inputProfile.getSource() << ": " << inputProfile.getName() << ") ";
		std::cout.flush();
		cmsUInt32Number inputFormat = 0;
		switch (inputProfile.getNumChannels()) {
			case 1:
				inputFormat = TYPE_GRAY_8;
				break;
			case 3:
				inputFormat = TYPE_RGB_8;
				break;
			case 4:
				inputFormat = TYPE_CMYK_8_REV;
				break;
			default:
				std::cerr << "Unsupported number of channels in input profile" << std::endl;
				fclose(f);
				fclose(fOut);
				jpeg_finish_decompress(&m_dinfo);
				return false;	
		}

		// Define output compression parameters
		m_cinfo.image_width = m_dinfo.output_width;
		m_cinfo.image_height = m_dinfo.output_height;
		m_cinfo.input_components = m_outputProfile.getNumChannels();
		cmsUInt32Number outputFormat = 0;
		switch (m_outputProfile.getNumChannels()) {
			case 1:
				m_cinfo.in_color_space = JCS_GRAYSCALE;
				outputFormat = TYPE_GRAY_8;
				break;
			case 3:
				m_cinfo.in_color_space = JCS_RGB;
				outputFormat = TYPE_RGB_8;
				break;
			case 4:
				m_cinfo.in_color_space = JCS_CMYK;
				outputFormat = TYPE_CMYK_8_REV;
				break;
			default:
				std::cerr << "Unsupported number of channels in output profile" << std::endl;
				fclose(f);
				fclose(fOut);
				jpeg_finish_decompress(&m_dinfo);
				return false;	
		}
		jpeg_set_defaults(&m_cinfo);
		jpeg_set_quality(&m_cinfo,m_jpegQuality,true);

		// Start output compression
		jpeg_start_compress(&m_cinfo,TRUE);

		// Embed output profile
		embedIccProfile(m_outputProfile,&m_cinfo);

		// Create buffer for input 
		long line_width = m_dinfo.output_width*m_dinfo.output_components;
		buffer_in[0] = new JSAMPLE[line_width];

		// Create buffer for output 
		long line_width_out = m_cinfo.image_width*m_cinfo.input_components;
		buffer_out[0] = new JSAMPLE[line_width_out];

		// Create profile transform
		int flags = 0;
		if (m_blackPointCompensation) {
			flags |= cmsFLAGS_BLACKPOINTCOMPENSATION;
		}
		if (!m_enableOptimization) {
			flags |= cmsFLAGS_NOOPTIMIZE;
		}
		hTransform = cmsCreateTransform(inputProfile.getHandle(),
										inputFormat,
										m_outputProfile.getHandle(),
										outputFormat,
										m_intent,
										flags);

		// Read and process image lines
		while (m_dinfo.output_scanline < m_dinfo.output_height) {
			if (m_verbose) {
				std::cout << std::setw(3) << (100*m_dinfo.output_scanline/m_dinfo.output_height) << "%\b\b\b\b";
			}
			jpeg_read_scanlines(&m_dinfo,&buffer_in[0],1);
			cmsDoTransform(hTransform,(const void *) buffer_in[0],(void *) buffer_out[0],(cmsUInt32Number) m_dinfo.output_width);
			jpeg_write_scanlines(&m_cinfo,&buffer_out[0],1);
		}

		// Delete profile transform
		cmsDeleteTransform(hTransform);
		hTransform = NULL;

		// Finish decompression/compression and close files
		jpeg_finish_decompress(&m_dinfo);
		jpeg_finish_compress(&m_cinfo);
		fclose(f);
		fclose(fOut);

		// Free resources
		delete buffer_in[0];
		delete buffer_out[0];

		// Delete original file when processing in same folder, silently fail otherwise 
		remove(outputFile.c_str());

		// Move temp file to final destination
		int renameStatus = rename(outputFileTemp.c_str(),outputFile.c_str());
		if (renameStatus != 0) {
			std::cerr << "Can't rename " << outputFileTemp << " to " << outputFile << std::endl;
			return false;
		}		

	} catch(int e) {
		// Error during JPEG (de)compression, show message
		switch (e) {
			case 1:
				std::cerr << "Error decompressing source JPEG image." << std::endl;
				break;
			case 2:
				std::cerr << "Error compressing converted JPEG image." << std::endl;
				break;
			default:
				std::cerr << "Unknown exception during conversion." << std::endl;
				break;
		}

		// Clean up
		jpeg_abort_decompress(&m_dinfo);
		jpeg_abort_compress(&m_cinfo);
		if (f != NULL) fclose(f);
		if (fOut != NULL) {
			fclose(fOut);
			remove(outputFileTemp.c_str());
		}
		if (hTransform != NULL) cmsDeleteTransform(hTransform);
		if (buffer_in[0] != NULL) delete buffer_in[0];
		if (buffer_out[0] != NULL) delete buffer_out[0];

		// Finish with error
		return false;
	}	

	std::cout << "Done." << std::endl;

	return true;
}


/**
 * Embeds an ICC profile in a JPEG file.
 *
 * Designed to work on a jpeglib JPEG compression process, requires as parameter
 * the jpeg_compress_struct object used by jpeglib for managing compression data.
 * Has to be called after jpeg_start_compress and before any jpeg_write_scanlines.
 *
 * @param[in] profile The ICC profile to embed
 * @param[in] p_cinfo Pointer to the jpeg_compress_struct used by jpeglib for JPEG compression
 */
void IccConverter::embedIccProfile(const IccProfile& profile, jpeg_compress_struct* p_cinfo) {
	// Save profile to memory 
	cmsUInt32Number outIccLength = 0;
	cmsSaveProfileToMem(profile.getHandle(),NULL,&outIccLength);
	char* outIccBuffer = new char[outIccLength];
	cmsSaveProfileToMem(profile.getHandle(),(void*)outIccBuffer,&outIccLength);

	// Generate profile data and embed into JPEG marker
	char markerBuffer[65533];
	markerBuffer[0] = (unsigned char) 'I';
	markerBuffer[1] = (unsigned char) 'C';
	markerBuffer[2] = (unsigned char) 'C';
	markerBuffer[3] = (unsigned char) '_';
	markerBuffer[4] = (unsigned char) 'P';
	markerBuffer[5] = (unsigned char) 'R';
	markerBuffer[6] = (unsigned char) 'O';
	markerBuffer[7] = (unsigned char) 'F';
	markerBuffer[8] = (unsigned char) 'I';
	markerBuffer[9] = (unsigned char) 'L';
	markerBuffer[10] = (unsigned char) 'E';
	markerBuffer[11] = (unsigned char) 0;
	int iccChunks = (outIccLength / 65517) + 1;
	unsigned long savedBytes = 0;
	for (int i=1; i<=iccChunks; i++) {
		markerBuffer[12] = (unsigned char) i;
		markerBuffer[13] = (unsigned char) iccChunks;
		unsigned long bytesToSave = std::min((unsigned long)(outIccLength - savedBytes),(unsigned long)65517);
		memcpy(&markerBuffer[14],&outIccBuffer[savedBytes],bytesToSave);
		jpeg_write_marker(p_cinfo,JPEG_APP0+2,(unsigned char*)markerBuffer,bytesToSave+14);
		savedBytes += bytesToSave;
	}

	// Free resources
	delete[] outIccBuffer;
}

/**
 * Removes last slash character from a string, only if the string
 * actually ends in a slash. If the original string does not have
 * a trailing slash, it is returned untouched.
 *
 * @param[in] str The string to process
 * @return A string without the trailing slash
 */
std::string IccConverter::removeTrailingSlash(const std::string str) {
	std::string newStr(str);
	if (str.compare(str.length()-1,1,g_slash) == 0) {
		newStr = newStr.substr(0,str.length()-1);
	}
	return newStr;
}

/**
 * Custom handling of errors in the libjpeg library
 */
METHODDEF(void) my_error_exit(j_common_ptr cinfo) {
  /* Return control to the setjmp point */
  my_error_mgr* myerr = (my_error_mgr*) cinfo->err;
  longjmp(myerr->setjmp_buffer, 1);
}
