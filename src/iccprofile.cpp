#include <iostream>
#include <fstream>
#include <cstring>
#include <lcms2.h>
#include <algorithm>
#include "icc_adobergb.h"
#include "iccprofile.h"

IccProfile::IccProfile():m_hprofile(NULL) {
}

/**
 * Loads ICC profile from file.
 * 
 * File can be a standard ICC Profile file, or a JPEG file.
 * JPEG files are scanned for embedded ICC Profile data, and for
 * Exif metadata. 
 *
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
		if (get_icc_profile(filename,&profileBuffer,profileSize,exifProfile)) {
			if (profileSize > 0) {
				// Embedded ICC Profile 
				m_hprofile = cmsOpenProfileFromMem((const void*) profileBuffer, (cmsUInt32Number) profileSize);
				delete[] profileBuffer;
			} else if (exifProfile == 0xFFFF) { 
				// EXIF AdobeRGB
				m_hprofile = cmsOpenProfileFromMem((const void*) iccAdobeRGB, (cmsUInt32Number) iccAdobeRGB_size);
			} else if (exifProfile == 1) { 
				// EXIF sRGB
				m_hprofile = cmsCreate_sRGBProfile();
			}
		}
	} else {
		// Load standard ICC Profile file
		m_hprofile = cmsOpenProfileFromFile(filename.c_str(),"r");
	}

	return (m_hprofile != NULL);
}

/**
 * Loads ICC profile from a memory buffer
 *
 */
bool IccProfile::loadFromMem(const char* buffer, const long bufferSize) {
	// Clear current profile data
	clear();

	// Load profile from memory
	m_hprofile = cmsOpenProfileFromMem((const void*) buffer, (cmsUInt32Number) bufferSize);

	return (m_hprofile != NULL);
}

void IccProfile::loadSRGB() {
	// Clear current profile data
	clear();

	// Load sRGB profile
	m_hprofile = cmsCreate_sRGBProfile();
}


IccProfile::~IccProfile() {
	clear();
}

void IccProfile::clear() {
	if (m_hprofile != NULL) {
		cmsCloseProfile(m_hprofile);
		m_hprofile = NULL;
	}
}

bool IccProfile::isValid() {
	return (m_hprofile != NULL);
}

cmsHPROFILE IccProfile::getHandle() {
	return m_hprofile;
}


bool IccProfile::get_icc_profile(std::string filename, char** profileBuffer, unsigned long &profileSize, unsigned int &exifProfile) {

	// Marcadors JPEG
	char SOI[] = {0xFF,0xD8};
	char ICC_TAG[] = {'I','C','C','_','P','R','O','F','I','L','E',0};
	char EXIF_TAG[] = {'E','x','i','f',0,0};

	// Inicialització de variables
	char buffer[256];
	unsigned long markerLength = 0;
	unsigned long markerStart = 0;
	unsigned char iccChunk = 0;
	unsigned char iccNumChunks = 0;
	unsigned long *iccPositions = NULL;
	unsigned long *iccSizes = NULL;
	unsigned long iccProfileSize = 0;
	unsigned int exifColorSpace = 0;

	// Obrim el fitxer
	std::ifstream f(filename.c_str(), std::ios::in | std::ios::binary);

	try {
		// Detectem inici de fitxer JPEG
		if (!read_bytes_compare(f,buffer,2,SOI)) {
			f.close();
			return false;
		}

		// Bucle de lectura dels marcadors JPEG
		bool finished = false;
		while (!finished) {
			read_bytes(f,buffer,2);
			if (buffer[0] != (char)0xFF) {		// Marcador invàlid
				f.close();
				return false;
			}
			switch (buffer[1]) {
				case (char)0xFF:	// Padding
					break;

				case JPEG_MARKER_APP1:	// EXIF
					markerStart = f.tellg();
					read_bytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					if (read_bytes_compare(f,buffer,6,EXIF_TAG)) {
						unsigned long exifStart = f.tellg();
						read_bytes(f,buffer,8);
						bool littleEndian = (buffer[0] == (unsigned char) 0x49);
						unsigned long offsetIFD0 =  exif_read_long(&buffer[4],littleEndian);

						f.seekg(exifStart+offsetIFD0,f.beg);
						read_bytes(f,buffer,2);
						unsigned int IFD0count = exif_read_word(buffer,littleEndian);
						unsigned long exifIFDoffset = 0;
						unsigned int count = 0;
						while ((count < IFD0count) && (exifIFDoffset == 0)) {
							read_bytes(f,buffer,12);
							unsigned int exifTag = exif_read_word(&buffer[0],littleEndian);
							if (exifTag == 0x8769) {
								exifIFDoffset = exif_read_long(&buffer[8],littleEndian);
							}
							count++;
						}

						if (exifIFDoffset != 0) {
							f.seekg(exifStart+exifIFDoffset,f.beg);
							read_bytes(f,buffer,2);
							unsigned int ExifIFDcount = exif_read_word(buffer,littleEndian);
							count = 0;
							while ((count < ExifIFDcount) && (exifColorSpace == 0)) {
								read_bytes(f,buffer,12);
								unsigned int exifTag = exif_read_word(&buffer[0],littleEndian);
								if (exifTag == 0xA001) {
									exifColorSpace = exif_read_word(&buffer[8],littleEndian);
								}
								count++;
							}
						}
					}
					f.seekg(markerStart+markerLength,f.beg);
					break;



				case JPEG_MARKER_APP2:	// ICC Profile
					read_bytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					if (read_bytes_compare(f,buffer,12,ICC_TAG)) {
						read_bytes(f,buffer,2);
						iccChunk = (unsigned char) buffer[0];
						iccNumChunks = (unsigned char) buffer[1];
						// Guardem posicions dels fragments ICC
						if (iccPositions == NULL) {
							iccPositions = new unsigned long[iccNumChunks];
							iccSizes = new unsigned long[iccNumChunks];
						}
						iccPositions[iccChunk-1] = f.tellg();
						iccSizes[iccChunk-1] = markerLength-2-12-2;
						iccProfileSize += iccSizes[iccChunk-1];
						// Avancem al següent marcador JPEG
						f.seekg(markerLength-2-12-2,f.cur);
					} else {
						f.seekg(markerLength-2-12,f.cur);
					}
					break;

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
					read_bytes(f,buffer,2);
					markerLength = (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
					f.seekg(markerLength-2,f.cur);
					break;

				default:
					finished = true;
					break;
			}
		}

		// Guardem el perfil en el buffer
		if (iccProfileSize > 0) {
			(*profileBuffer) = new char[iccProfileSize];
			unsigned long bytesRead = 0;
			for (unsigned char i=0; i<iccNumChunks; i++) {
				f.seekg(iccPositions[i],f.beg);
				read_bytes(f,&((*profileBuffer)[bytesRead]),iccSizes[i]);
				bytesRead += iccSizes[i];
			}
		}
		profileSize = iccProfileSize;

		// Guardem info de perfil Exif
		exifProfile = exifColorSpace;

		// Alliberem recursos
		if (iccPositions != NULL) delete[] iccPositions;
		if (iccSizes != NULL) delete[] iccSizes;

	} catch (int e) {
		if (iccPositions != NULL) delete[] iccPositions;
		if (iccSizes != NULL) delete[] iccSizes;
		f.close();
		return false;
	}

	// Tanquem el fitxer
	f.close();

	return true;
}

/**
 * Llig "length" bytes del fitxer "f" dins del "buffer".
 * Provoca excepció int 1 si falla la lectura.
 *
 */
void IccProfile::read_bytes(std::ifstream &f, char *buffer, long length) {
	f.read(buffer,length);
	if (f.fail()) {
		throw 1;
	}
}

/**
 * Llig "length" bytes del fitxer "f" dins del "buffer", i compara amb "compare".
 * Torna true si la comparació és correcta, false si hi ha diferències.
 * Provoca excepció int 1 si falla la lectura.
 */
bool IccProfile::read_bytes_compare(std::ifstream &f, char *buffer, long length, char *compare) {
	read_bytes(f,buffer,length);
	return (memcmp(buffer,compare,length) == 0);
}


unsigned int IccProfile::exif_read_word(const char* buffer, const bool littleEndian) {
	if (littleEndian) {
		return (((unsigned char) buffer[1]) << 8) | ((unsigned char) buffer[0]);
	} else {
		return (((unsigned char) buffer[0]) << 8) | ((unsigned char) buffer[1]);
	}
}

unsigned long IccProfile::exif_read_long(const char* buffer, const bool littleEndian) {
	if (littleEndian) {
		return (((unsigned char) buffer[3]) << 24) | (((unsigned char) buffer[2]) << 16) | (((unsigned char) buffer[1]) << 8) | ((unsigned char) buffer[0]);
	} else {
		return (((unsigned char) buffer[0]) << 24) | (((unsigned char) buffer[1]) << 16) | (((unsigned char) buffer[2]) << 8) | ((unsigned char) buffer[3]);
	}
}

