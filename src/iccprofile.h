#ifndef ICCPROFILE_H
#define ICCPROFILE_H

#include <lcms2.h>

/**
 * IccProfile objects represent an ICC color profile.
 *
 */
class IccProfile {
	public:
		IccProfile();
		~IccProfile();
		IccProfile(const IccProfile&);
		IccProfile& operator=(const IccProfile&);
		bool loadFromFile(const std::string&);
		bool loadFromMem(const char*,const long);
		void loadSRGB();
		void loadGray(double);
		bool isValid() const;
		bool isValid();
		cmsHPROFILE getHandle();
		cmsHPROFILE getHandle() const;
		cmsUInt32Number getNumChannels();
		std::string getSource();
		std::string getSource() const;
		std::string getName();
		std::string getName() const;

	private:
		void readBytes(std::ifstream&, char*, long);
		bool readBytesAndCompare(std::ifstream&, char*, long, char*);
		unsigned int exifReadWord(const char*, const bool);
		unsigned long exifReadLong(const char*, const bool);
		bool extractIccProfile(const std::string, char**, unsigned long&, unsigned int&);
		void clear();
		std::string extractProfileName();

		cmsHPROFILE m_hprofile;			/**< Handle to corresponding LittleCMS library icc profile */
		std::string m_profileSource; 	/**< Tells how the ICC profile was found (embedded, EXIF,...) */
		std::string m_profileName;		/**< Name embedded in the ICC profile */

};

/**
 * Enumeration of JPEG marker identification bytes
 */
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
