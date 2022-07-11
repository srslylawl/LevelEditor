#pragma once
#include <istream>
#include <ostream>
#include <vector>

namespace Serialization {
	template<typename T>
	void writeToStream(std::ostream& oStream, const T& item) {
		oStream.write((char*)&item, sizeof T);
	}

	template<typename T>
	void readFromStream(std::istream& iStream, const T& item) {
		iStream.read((char*)&item, sizeof T);
	}

	inline std::ostream& Serialize(std::ostream& oStream, const std::string& str) {
		auto size = str.size();
		oStream.write((char*)&size, sizeof(size));
		oStream << str;
		return oStream;
	}

	inline std::string DeserializeStdString(std::istream& stream) {
		size_t stringSize;
		stream.read((char*)&stringSize, sizeof(stringSize));
		if(stringSize > 2048) stringSize = 2048; //hard coded limit in case we read from an invalid stream

		char* chars = new char[stringSize + 1];
		chars[stringSize] = 0;
		stream.read(chars, stringSize);
		std::string s = { chars };

		delete[] chars;
		return s;
	}
}
