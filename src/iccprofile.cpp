#include <iostream>
#include <fstream>
#include <cstring>
#include <lcms2.h>
#include <algorithm>
#include <vector>
#include "icc_adobergb.h"
#include "iccprofile.h"

/**
 * Default constructor with empty initializations.
 */
IccProfile::IccProfile():m_hprofile(NULL)
{
	m_profileSource.clear();
	m_profileName.clear();
}

/**
 * Copy constructor.
 */
IccProfile::IccProfile(const IccProfile& iccprofile):m_hprofile(NULL) {
	if (iccprofile.isValid()) {
		// Save original profile to memory
		cmsUInt32Number bytesNeeded = 0;
		cmsSaveProfileToMem(iccprofile.getHandle(),NULL,&bytesNeeded);
		char* buffer = new char[bytesNeeded];
		cmsSaveProfileToMem(iccprofile.getHandle(),(void *)buffer,&bytesNeeded);

		// Load from memory into new profile
		m_hprofile = cmsOpenProfileFromMem((const void*) buffer, bytesNeeded);
		delete[] buffer;
	}

	// Store text descriptions
	m_profileSource = iccprofile.getSource();
	m_profileName = iccprofile.getName();
}

/**
 * Assignment operator overloading.
 */
IccProfile& IccProfile::operator=(const IccProfile& iccprofile) {
	// Clear current profile data
	clear();

	// Replicate data from source profile
	if (iccprofile.isValid()) {
		// Save original profile to memory
		cmsUInt32Number bytesNeeded = 0;
		cmsSaveProfileToMem(iccprofile.getHandle(),NULL,&bytesNeeded);
		char* buffer = new char[bytesNeeded];
		cmsSaveProfileToMem(iccprofile.getHandle(),(void *)buffer,&bytesNeeded);

		// Load from memory into new profile
		m_hprofile = cmsOpenProfileFromMem((const void*) buffer, bytesNeeded);
		delete[] buffer;
	}

	// Store text descriptions
	m_profileSource = iccprofile.getSource();
	m_profileName = iccprofile.getName();

	return *this;
}

/**
 * Loads ICC profile from file.
 * 
 * File can be a standard ICC Profile file, or a JPEG file.
 * JPEG files are scanned for embedded ICC Profile data, and for
 * Exif metadata. 
 *
 * @param[in] filename Path to the file where to look for the ICC profile.
 * @return true if profile was sucessfully loaded, false otherwise 
 */
bool IccProfile::loadFromFile(const std::string& filename) {
	// Clear current profile data
	clear();

	// Check valid filename
	if (filename.empty()) {
		return false;
	}

	// Determine file type by extension and load new data
	std::string lower = filename;
	transform(lower.begin(),lower.end(),lower.begin(),::tolower);
	if ((lower.rfind(".jpg") == lower.size()-4) || (lower.rfind(".jpeg") == lower.size()-5)) {
		// Load ICC Profile from JPEG
		unsigned long profileSize = 0;
		char *profileBuffer = NULL;
		unsigned int exifProfile = 0;
		if (extractIccProfile(filename,&profileBuffer,profileSize,exifProfile)) {
			if (profileSize > 0) {
				// Embedded ICC Profile 
				m_hprofile = cmsOpenProfileFromMem((const void*) profileBuffer, (cmsUInt32Number) profileSize);
				delete[] profileBuffer;
				m_profileSource = "Embedded";
			} else if (exifProfile == 2) { 
				// EXIF AdobeRGB
				m_hprofile = cmsOpenProfileFromMem((const void*) iccAdobeRGB, (cmsUInt32Number) iccAdobeRGB_size);
				m_profileSource = "EXIF";
			} else if (exifProfile == 1) { 
				// EXIF sRGB
				m_hprofile = cmsCreate_sRGBProfile();
				m_profileSource = "EXIF";
			}
		}
	} else {
		// Load standard ICC Profile file
		m_hprofile = cmsOpenProfileFromFile(filename.c_str(),"r");
		if  (m_hprofile != NULL) {
			m_profileSource = "File";
		}
	}

	// Store the name of the profile
	if (m_hprofile != NULL) {
		m_profileName = extractProfileName();
	}

	return (m_hprofile != NULL);
}

/**
 * Loads ICC profile from a memory buffer
 *
 * @param[in] buffer Pointer to the memory buffer holding ICC profile data
 * @param[in] bufferSize Size of the memory buffer
 * @return true if profile is successfully loaded, false otherwise
 */
bool IccProfile::loadFromMem(const char* buffer, const long bufferSize) {
	// Clear current profile data
	clear();

	// Load profile from memory
	m_hprofile = cmsOpenProfileFromMem((const void*) buffer, (cmsUInt32Number) bufferSize);
	if  (m_hprofile != NULL) {
		m_profileSource = "Memory";
		m_profileName = extractProfileName();
	}

	return (m_hprofile != NULL);
}

/**
 * Loads a default sRGB() profile data into the ICC profile object
 */
void IccProfile::loadSRGB() {
	// Clear current profile data
	clear();

	// Load sRGB profile
	m_hprofile = cmsCreate_sRGBProfile();
	m_profileSource = "Library";
	m_profileName = extractProfileName();
}

/**
 * Loads a default Grayscale profile data into the ICC profile object.
 * 
 * @param[in] gamma The Gamma value to be used for the grayscale profile
 */
void IccProfile::loadGray(double gamma) {
	// Clear current profile data
	clear();
	
	// Load grayscale profile
	cmsToneCurve* GammaCurve = cmsBuildGamma(0, gamma);
	m_hprofile = cmsCreateGrayProfile(cmsD50_xyY(), GammaCurve);
	cmsFreeToneCurve(GammaCurve);
	m_profileSource = "Library";
	m_profileName = extractProfileName();
}

/**
 * Destructor.
 */
IccProfile::~IccProfile() {
	clear();
}

/**
 * Gets the profile name stored inside the ICC profile
 *
 * @return a string with the name, empty if no valid name is found
 */
std::string IccProfile::extractProfileName() {
	char textBuffer[512];
	std::string name("");
	if (m_hprofile != NULL) {
		if (cmsGetProfileInfoASCII(m_hprofile,cmsInfoDescription,"en","EN",textBuffer,512) != 0) {
			name.assign(textBuffer);
		}
	}

	return name;
}

/**
 * Returns a text description of how the profile was found (embedded, Exif, file, memory,...)
 *
 * @return a string with a text description of the source of the profile
 */
std::string IccProfile::getSource() {
	return m_profileSource;
}

/**
 * Returns a text description of how the profile was found (embedded, Exif, file, memory,...).
 * This is a const version of @ref IccProfile#getSource
 *
 * @return a string with a text description of the source of the profile
 */
std::string IccProfile::getSource() const {
	return m_profileSource;
}

/**
 * Returns a text description of the name stored inside the ICC profile
 *
 * @return a string with the name stored inside the ICC profile 
 */
std::string IccProfile::getName() {
	return m_profileName;
}

/**
 * Returns a text description of the name stored inside the ICC profile.
 * This is a const version of @ref IccProfile#getName
 *
 * @return a string with the name stored inside the ICC profile 
 */
std::string IccProfile::getName() const {
	return m_profileName;
}

/**
 * Clears all data stored in the IccProfile object, and frees any resource
 * previously allocated
 */
void IccProfile::clear() {
	m_profileSource.clear();
	m_profileName.clear();
	if (m_hprofile != NULL) {
		cmsCloseProfile(m_hprofile);
		m_hprofile = NULL;
	}
}

/**
 * Checks if valid profile data is stored in the IccProfile object.
 * This is a const version of @ref IccProfile#isValid
 *
 * @return true if this object has valid ICC profile data, false otherwise
 */
bool IccProfile::isValid() const {
	return (m_hprofile != NULL);
}

/**
 * Checks if valid profile data is stored in the IccProfile object
 *
 * @return true if this object has valid ICC profile data, false otherwise
 */
bool IccProfile::isValid() {
	return (m_hprofile != NULL);
}

/**
 * Gets a handle to the LittleCMS ICC profile associated to this IccProfile object
 * This is a const version of @ref IccProfile#getHandle
 *
 * @return A handle to the LittleCMS profile, or NULL if none has been previously loaded
 */
cmsHPROFILE IccProfile::getHandle() const {
	return m_hprofile;
}

/**
 * Gets a handle to the LittleCMS ICC profile associated to this IccProfile object
 *
 * @return A handle to the LittleCMS profile, or NULL if none has been previously loaded
 */
cmsHPROFILE IccProfile::getHandle() {
	return m_hprofile;
}

/**
 * Gets the number of channels in the loaded ICC profile.
 *
 * @return Number of channels of the ICC profile, or 0 if no profile has been loaded
 */
cmsUInt32Number IccProfile::getNumChannels() {
	cmsUInt32Number channels = 0;
	if (m_hprofile != NULL) {
		channels = cmsChannelsOf(cmsGetColorSpace(m_hprofile));
	}
	return channels;
}

/**
 * Gets ICC profiles embedded in JPEG files.
 *
 * If an ICC profile is embedded in the JPEG file, it is loaded into a memory buffer. Besides that,
 * EXIF data on color profiles of the image is also stored.
 *
 * \warning If an embedded profile is found, a memory buffer will be allocated into the
 * profileBuffer parameter. The calling code will be responsible for freeing this buffer
 * when it is no longer needed.
 *
 * @param[in] filename Path to the JPEG file
 * @param[out] profileBuffer Memory buffer where the embedded profile will be loaded
 * @param[out] profileSize Size in bytes of the embedded profile if detected, 0 if not found
 * @param[out] exifProfile EXIF color profile code: 0=Not found,  1=sRGB, 2=AdobeRGB, 0xFFFF=undefined
 * @return true if file could be scanned for ICC profiles, false if some error happened 
 */
bool IccProfile::extractIccProfile(std::string filename, char** profileBuffer, unsigned long &profileSize, unsigned int &exifProfile) {

	// JPEG Markers 
	char SOI[] = {0xFF,0xD8};
	char ICC_TAG[] = {'I','C','C','_','P','R','O','F','I','L','E',0};
	char EXIF_TAG[] = {'E','x','i','f',0,0};

	// Variable initialization 
	char buffer[256];
	unsigned long markerLength = 0;
	unsigned long markerStart = 0;
	std::vector<unsigned long> iccPositions;
	std::vector<unsigned long> iccSizes;
	unsigned long iccProfileSize = 0;
	unsigned int exifColorSpace = 0;

	// Open file 
	std::ifstream f(filename.c_str(), std::ios::in | std::ios::binary);

	try {
		// Detect JPEG start 
		if (!readBytesAndCompare(f,buffer,2,SOI)) {
			f.close();
			return false;
		}

		// JPEG markers reading loop
		bool finished = false;
		while (!finished) {
			readBytes(f,buffer,2);
			if (buffer[0] != (char)0xFF) {		// Invalid marker 
				f.close();
				return false;
			}
			switch (buffer[1]) {
				case (char)0xFF:	// Padding
					break;

				case JPEG_MARKER_APP1:	// EXIF
					markerStart = f.tellg();
					readBytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					if (readBytesAndCompare(f,buffer,6,EXIF_TAG)) {
						unsigned long exifStart = f.tellg();
						readBytes(f,buffer,8);
						bool littleEndian = (buffer[0] == (unsigned char) 0x49);
						unsigned long offsetIFD0 =  exifReadLong(&buffer[4],littleEndian);

						// Get offsets to Exif data, WhitePoint and Primaries Cromaticity
						f.seekg(exifStart+offsetIFD0,f.beg);
						readBytes(f,buffer,2);
						unsigned int IFD0count = exifReadWord(buffer,littleEndian);
						unsigned long exifIFDoffset = 0;
						unsigned long whitePointOffset = 0;
						unsigned long primariesOffset = 0;
						unsigned int count = 0;
						while ((count < IFD0count) && ((exifIFDoffset == 0) || (whitePointOffset == 0) || (primariesOffset == 0))) {
							readBytes(f,buffer,12);
							unsigned int exifTag = exifReadWord(&buffer[0],littleEndian);
							if (exifTag == 0x8769) {
								exifIFDoffset = exifReadLong(&buffer[8],littleEndian);
							} else if (exifTag == 0x13e) {
								whitePointOffset = exifReadLong(&buffer[8],littleEndian);
							} else if (exifTag == 0x13f) {
								primariesOffset = exifReadLong(&buffer[8],littleEndian);
							}
							count++;
						}

						// Get Exif ColorSpace
						if (exifIFDoffset != 0) {
							f.seekg(exifStart+exifIFDoffset,f.beg);
							readBytes(f,buffer,2);
							unsigned int ExifIFDcount = exifReadWord(buffer,littleEndian);
							count = 0;
							while ((count < ExifIFDcount) && (exifColorSpace == 0)) {
								readBytes(f,buffer,12);
								unsigned int exifTag = exifReadWord(&buffer[0],littleEndian);
								if (exifTag == 0xA001) {
									exifColorSpace = exifReadWord(&buffer[8],littleEndian);
								}
								count++;
							}
						}

						// Check AdobeRGB white point and primaries
						if (exifColorSpace == 0xFFFF) {
							unsigned long rationals[16];
							for (int r=0; r<16; r++) rationals[r] = 0;

							if (whitePointOffset != 0) {
								f.seekg(exifStart+whitePointOffset,f.beg);
								readBytes(f,buffer,16);
								for (int wp=0; wp<4; wp++) {
									rationals[wp] = exifReadLong(&buffer[wp*4],littleEndian);
								}
							}	

							if (primariesOffset != 0) {
								f.seekg(exifStart+primariesOffset,f.beg);
								readBytes(f,buffer,48);
								for (int p=0; p<12; p++) {
									rationals[4+p] = exifReadLong(&buffer[p*4],littleEndian);
								}
							}	

							unsigned long adobeRGBrationals[] = {313,1000,329,1000,64,100,33,100,21,100,71,100,15,100,6,100};
							bool isAdobeRGB = true;
							for (int cmp=0; cmp<16; cmp++) {
								if (rationals[cmp] != adobeRGBrationals[cmp]) {
									isAdobeRGB = false;
								}
							}
							if (isAdobeRGB) {
								exifColorSpace = 2;
							}
						}
					}
					f.seekg(markerStart+markerLength,f.beg);
					break;

				case JPEG_MARKER_APP2:	// ICC Profile
					readBytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					if (readBytesAndCompare(f,buffer,12,ICC_TAG)) {
						readBytes(f,buffer,2);
						/* iccChunk = (unsigned char) buffer[0]; */
						/* iccNumChunks = (unsigned char) buffer[1]; */
						// Store positions of ICC fragments
						iccPositions.push_back(f.tellg());
						iccSizes.push_back(markerLength-2-12-2);
						iccProfileSize += iccSizes[iccSizes.size()-1];
						// Go forward to next JPEG marker
						f.seekg(markerLength-2-12-2,f.cur);
					} else {
						f.seekg(markerLength-2-12,f.cur);
					}
					break;

				case JPEG_MARKER_APP0:
				case JPEG_MARKER_APP3:
				case JPEG_MARKER_APP4:
				case JPEG_MARKER_APP5:
				case JPEG_MARKER_APP6:
				case JPEG_MARKER_APP7:
				case JPEG_MARKER_APP8:
				case JPEG_MARKER_APP9:
				case JPEG_MARKER_APP10:
				case JPEG_MARKER_APP11:
				case JPEG_MARKER_APP12:
				case JPEG_MARKER_APP13:
				case JPEG_MARKER_APP14:
				case JPEG_MARKER_APP15:
					// Skip other JPEG markers
					readBytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					f.seekg(markerLength-2,f.cur);
					break;

				default:
					// No more JPEG marker data found, finish scanning
					finished = true;
					break;
			}
		}

		// Store ICC profile into memory buffer
		if (iccProfileSize > 0) {
			(*profileBuffer) = new char[iccProfileSize];
			unsigned long bytesRead = 0;
			for (unsigned char i=0; i<iccPositions.size(); i++) {
				f.seekg(iccPositions[i],f.beg);
				readBytes(f,&((*profileBuffer)[bytesRead]),iccSizes[i]);
				bytesRead += iccSizes[i];
			}
		}
		profileSize = iccProfileSize;

		// Store EXIF color space data
		exifProfile = exifColorSpace;

	} catch (int e) {
		f.close();
		return false;
	}

	// Close file 
	f.close();

	return true;
}

/**
 * Reads bytes from a file into a memory buffer
 *
 * @param[in] f The ifstream from where to read
 * @param[out] buffer The memory buffer where data will be stored
 * @param[in] length The number of bytes to read
 * @throw Integer "1" if reading from the file fails
 */
void IccProfile::readBytes(std::ifstream &f, char *buffer, long length) {
	f.read(buffer,length);
	if (f.fail()) {
		throw 1;
	}
}

/**
 * Reads bytes from a file into a memory buffer and compares them to another
 * memory buffer.
 *
 * @param[in] f The ifstream from where to read
 * @param[out] buffer The memory buffer where data will be stored
 * @param[in] length The number of bytes to read
 * @param[in] compare The memory buffer with the data to compare to.
 * @throw Integer "1" if reading from the file fails
 * @return true if the bytes read match the bytes to compare, false if different
 */
bool IccProfile::readBytesAndCompare(std::ifstream &f, char *buffer, long length, char *compare) {
	readBytes(f,buffer,length);
	return (memcmp(buffer,compare,length) == 0);
}

/**
 * Transforms 2 bytes read from an EXIF JPEG marker into a 16 bit value, taking into
 * consideration byte ordering (endianness)
 *
 * @param[in] buffer Pointer to memory buffer with the 2 bytes to read
 * @param[in] littleEndian true if data is little-endian, false if it is big-endian
 */
unsigned int IccProfile::exifReadWord(const char* buffer, const bool littleEndian) {
	if (littleEndian) {
		return (((unsigned char) buffer[1]) << 8) | ((unsigned char) buffer[0]);
	} else {
		return (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
	}
}

/**
 * Transforms 4 bytes read from an EXIF JPEG marker into a 32 bit value, taking into
 * consideration byte ordering (endianness)
 *
 * @param[in] buffer Pointer to memory buffer with the 4 bytes to read
 * @param[in] littleEndian true if data is little-endian, false if it is big-endian
 */
unsigned long IccProfile::exifReadLong(const char* buffer, const bool littleEndian) {
	if (littleEndian) {
		return (((unsigned char) buffer[3]) << 24) | (((unsigned char) buffer[2]) << 16) | (((unsigned char) buffer[1]) << 8) | ((unsigned char) buffer[0]);
	} else {
		return (((unsigned char) buffer[0]) << 24) | (((unsigned char) buffer[1]) << 16) | (((unsigned char) buffer[2]) << 8) | ((unsigned char) buffer[3]);
	}
}

