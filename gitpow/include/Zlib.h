/*
* Author: Gasparyan Sokrat
* Date: 2019/12/28
* Zlib.h - this header file is a application definition.
*/
#pragma once
#include "stdafx.h"
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>
using namespace boost::iostreams;

/*
Implementation taked from:
	https://gist.github.com/yfnick/6ba33efa7ba12e93b148
	
*/

class Zlib {
public:
	static std::string compress(const std::string& data)
	{
		

		std::stringstream compressed;
		std::stringstream origin(data);

		filtering_streambuf<input> out;
		out.push(zlib_compressor(zlib_params(zlib::best_compression)));
		out.push(origin);
		copy(out, compressed);

		return compressed.str();
	}

	static std::string decompress(const std::string& data)
	{
		

		std::stringstream compressed(data);
		std::stringstream decompressed;

		filtering_streambuf<input> out;
		out.push(zlib_decompressor());
		out.push(compressed);
		copy(out, decompressed);

		return decompressed.str();
	}
};