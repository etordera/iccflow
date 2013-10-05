#include <iostream>
#include <jpeglib.h>
#include <fstream>
#include <cstring>
#include "iccprofile.h"
#include "iccconverter.h"
#include "icc_fogra27.h"

/**
 * IccConverter objects manage the ICC profile color conversion
 * of JPEG images
 *
 */
IccConverter::IccConverter() {
	// Initialize variables
	m_file.clear();
	m_inputFolder.clear();
	m_outputFolder.clear();

	// Initialize JPEG decompress objects
	m_dinfo.err = jpeg_std_error(&m_derr);
	jpeg_create_decompress(&m_dinfo);

	// Initialize JPEG compress objects
	m_cinfo.err = jpeg_std_error(&m_cerr);
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
 * Sets the ICC profile that will be used as destination in 
 * the color conversion process.
 *
 * Defaults to sRGB. 
 *
 * @param[in] profileName Path to file with the output ICC profile 
 * @return true if the ICC profile was loaded correctly, false otherwise
 */
bool IccConverter::setOutputProfile(const std::string& profileName){
	m_outputProfile.loadFromFile(profileName);
	if (!m_outputProfile.isValid()) {
		m_outputProfile.loadSRGB();	
		return false;
	}

	return true;
}

/**
 * Sets the ICC profile that will be used by default as source profile 
 * when there is no embedded profile in a source image in RGB color space. 
 *
 * Defaults to sRGB. 
 *
 * @param[in] profileName Path to file with the default RGB ICC profile 
 * @return true if the ICC profile was loaded correctly, false otherwise
 */
bool IccConverter::setDefaultRGBProfile(const std::string& profileName){
	m_defaultRGBProfile.loadFromFile(profileName);
	if (!m_defaultRGBProfile.isValid()) {
		m_defaultRGBProfile.loadSRGB();	
		return false;
	}

	return true;
}

/**
 * Sets the ICC profile that will be used by default as source profile 
 * when there is no embedded profile in a source image in CMYK color space. 
 *
 * Defaults to FOGRA27. 
 *
 * @param[in] profileName Path to file with the default CMYK ICC profile 
 * @return true if the ICC profile was loaded correctly, false otherwise
 */
bool IccConverter::setDefaultCMYKProfile(const std::string& profileName){
	m_defaultCMYKProfile.loadFromFile(profileName);
	if (!m_defaultCMYKProfile.isValid()) {
		m_defaultCMYKProfile.loadFromMem((char*)iccFOGRA27,iccFOGRA27_size);
		return false;
	}

	return true;
}

/**
 * Sets the ICC profile that will be used by default as source profile 
 * when there is no embedded profile in a source image in Grayscale color space. 
 *
 * Defaults to D50 Gamma-2.2 Grayscale profile 
 *
 * @param[in] profileName Path to file with the default Grayscale ICC profile 
 * @return true if the ICC profile was loaded correctly, false otherwise
 */
bool IccConverter::setDefaultGrayProfile(const std::string& profileName) {
	m_defaultGrayProfile.loadFromFile(profileName);
	if (!m_defaultGrayProfile.isValid()) {
		m_defaultGrayProfile.loadGray(2.2);
		return false;
	}

	return true;
}

/**
 * Sets the rendering intent for the color conversion.
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
	m_file = m_inputFolder + "/" + file;
	std::cout << "Processing " << m_file << ": ";

	// Check parameters
	if (!m_outputProfile.isValid()) {
		m_outputProfile.loadSRGB();	
	}

	// Open source file
	FILE* f;
	if ((f = fopen(m_file.c_str(), "rb")) == NULL) {
		std::cerr << "Failed to open " << m_file << std::endl;
		return false;
	}
	jpeg_stdio_src(&m_dinfo,f);

	// Open output file
	FILE* fOut;
	std::string outputFile = m_outputFolder + "/" + file;
	if ((fOut = fopen(outputFile.c_str(), "wb")) == NULL) {
		std::cerr << "Failed to write  " << m_file << std::endl;
		fclose(f);
		return false;
	}
	jpeg_stdio_dest(&m_cinfo,fOut);

	// Start input decompression
	jpeg_read_header(&m_dinfo, TRUE);
	jpeg_start_decompress(&m_dinfo);

	// Determine input profile
	IccProfile inputProfile;
	if (!inputProfile.loadFromFile(m_file)) {
		switch (m_dinfo.out_color_space) {
			case JCS_GRAYSCALE:
				if (!m_defaultGrayProfile.isValid()) {
					m_defaultGrayProfile.loadGray(2.2);
				}
				inputProfile = m_defaultGrayProfile;
				break;
			case JCS_CMYK:
				if (!m_defaultCMYKProfile.isValid()) {
					m_defaultCMYKProfile.loadFromMem((char*)iccFOGRA27,iccFOGRA27_size);
				}
				inputProfile = m_defaultCMYKProfile;
				break;
			case JCS_RGB:
				if (!m_defaultRGBProfile.isValid()) {
					m_defaultRGBProfile.loadSRGB();
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

	// Start output compression
	jpeg_start_compress(&m_cinfo,TRUE);

	// Embed output profile
	embedIccProfile(m_outputProfile,&m_cinfo);

	// Create buffer for input 
	long line_width = m_dinfo.output_width*m_dinfo.output_components;
	JSAMPLE* buffer_in[1];
	buffer_in[0] = new JSAMPLE[line_width];

	// Create buffer for output 
	long line_width_out = m_cinfo.image_width*m_cinfo.input_components;
	JSAMPLE* buffer_out[1];
	buffer_out[0] = new JSAMPLE[line_width_out];

	// Create profile transform
	cmsHTRANSFORM hTransform = cmsCreateTransform(	inputProfile.getHandle(),
													inputFormat,
													m_outputProfile.getHandle(),
													outputFormat,
													m_intent,
													0);

	// Read and process image lines
	while (m_dinfo.output_scanline < m_dinfo.output_height) {
		jpeg_read_scanlines(&m_dinfo,&buffer_in[0],1);
		cmsDoTransform(hTransform,(const void *) buffer_in[0],(void *) buffer_out[0],(cmsUInt32Number) m_dinfo.output_width);
		jpeg_write_scanlines(&m_cinfo,&buffer_out[0],1);
	}

	// Delete profile transform
	cmsDeleteTransform(hTransform);

	// Finish decompression/compression and close files
	jpeg_finish_decompress(&m_dinfo);
	fclose(f);
	jpeg_finish_compress(&m_cinfo);
	fclose(fOut);

	// Free resources
	delete buffer_in[0];
	delete buffer_out[0];
	
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
	if (str.compare(str.length()-1,1,"/") == 0) {
		newStr = newStr.substr(0,str.length()-1);
	}
	return newStr;
}
