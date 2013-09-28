#ifndef ICCPROFILE_H
#define ICCPROFILE_H

#include <lcms2.h>

class IccProfile {
	public:
		IccProfile();
		~IccProfile();
		bool loadFromFile(const std::string&);
		bool loadFromMem(const char*,const long);
		void loadSRGB();
		bool isValid();
		cmsHPROFILE getHandle();

	private:
		void read_bytes(std::ifstream&, char*, long);
		bool read_bytes_compare(std::ifstream&, char*, long, char*);
		unsigned int exif_read_word(const char*, const bool);
		unsigned long exif_read_long(const char*, const bool);
		bool get_icc_profile(const std::string, char**, unsigned long&, unsigned int&);
		void clear();

		cmsHPROFILE m_hprofile;

};

enum JPEG_MARKERS {
	JPEG_MARKER_APP0  = (char)0xE0,
	JPEG_MARKER_APP1  = (char)0xE1,
	JPEG_MARKER_APP2  = (char)0xE2,
	JPEG_MARKER_APP3  = (char)0xE3,
	JPEG_MARKER_APP4  = (char)0xE4,
	JPEG_MARKER_APP5  = (char)0xE5,
	JPEG_MARKER_APP6  = (char)0xE6,
	JPEG_MARKER_APP7  = (char)0xE7,
	JPEG_MARKER_APP8  = (char)0xE8,
	JPEG_MARKER_APP9  = (char)0xE9,
	JPEG_MARKER_APP10 = (char)0xEA,
	JPEG_MARKER_APP11 = (char)0xEB,
	JPEG_MARKER_APP12 = (char)0xEC,
	JPEG_MARKER_APP13 = (char)0xED,
	JPEG_MARKER_APP14 = (char)0xEE,
	JPEG_MARKER_APP15 = (char)0xEF
};

#endif
