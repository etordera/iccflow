#include <iostream>
#include <jpeglib.h>
#include <fstream>
#include "iccprofile.h"
#include "iccconverter.h"

/**
 * IccConverter objects manage the ICC profile color conversion
 * of JPEG images
 *
 */
IccConverter::IccConverter() {
	// Initialize member variables
	m_file.empty();
	m_inputFolder.empty();
	m_outputFolder.empty();

	// Initialize JPEG decompress objects
	m_dinfo.err = jpeg_std_error(&m_derr);
	jpeg_create_decompress(&m_dinfo);

	// Initialize JPEG compress objects
	m_cinfo.err = jpeg_std_error(&m_cerr);
	jpeg_create_compress(&m_cinfo);
}


IccConverter::~IccConverter() {
	// Destroy JPEG compress/decompress objects
	jpeg_destroy_decompress(&m_dinfo);
	jpeg_destroy_compress(&m_cinfo);
}

void IccConverter::setInputFolder(const std::string& inputFolder) {
	// TODO: check for existing trailing slash
	m_inputFolder = inputFolder;
}

void IccConverter::setOutputFolder(const std::string& outputFolder) {
	// TODO: check for existing trailing slash
	m_outputFolder = outputFolder;
}

bool IccConverter::convert(const std::string& file) {

	// Generate input file name
	m_file = m_inputFolder + "/" + file;
	std::cout << "Processing " << m_file << ": ";

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

	// Define output compression parameters
	m_cinfo.image_width = m_dinfo.output_width;
	m_cinfo.image_height = m_dinfo.output_height;
	m_cinfo.input_components = m_dinfo.output_components;
	m_cinfo.in_color_space = m_dinfo.out_color_space;
	jpeg_set_defaults(&m_cinfo);

	// Start output compression
	jpeg_start_compress(&m_cinfo,TRUE);

	// Create buffer for processing 
	long line_width = m_dinfo.output_width*m_dinfo.output_components;
	JSAMPLE* buffer[1];
	buffer[0] = new JSAMPLE[line_width];

	// Read and process image lines
	while (m_dinfo.output_scanline < m_dinfo.output_height) {
		jpeg_read_scanlines(&m_dinfo,&buffer[0],1);

		// TODO: Perform ICC color conversion

		jpeg_write_scanlines(&m_cinfo,&buffer[0],1);
	}

	// Finish decompression/compression and close files
	jpeg_finish_decompress(&m_dinfo);
	fclose(f);
	jpeg_finish_compress(&m_cinfo);
	fclose(fOut);

	// Free resources
	delete buffer[0];
	
	std::cout << "Done." << std::endl;

	return true;
}
