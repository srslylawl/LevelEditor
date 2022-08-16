#pragma once
#include <istream>
#include <ostream>
#include <iostream>
#include <bitset>

struct AssetHeader;

namespace Serialization {
	template<class SerializableT>
	class Serializable {
	public:
		Serializable() = delete;
		virtual ~Serializable() = default;

		std::string Name;

		Serializable(const std::string& name) : Name(name) {
		}

		virtual void Serialize(std::ostream& oStream) const = 0;

		static bool Deserialize(std::istream& iStream, SerializableT*& out_T) {
			return SerializableT::Deserialize(iStream, out_T);
		}
	};
//#ifndef DEBUG_SERIALIZATION
//#define DEBUG_SERIALIZATION
//#endif

	template<typename T>
	void writeToStream(std::ostream& oStream, const T& item) {
		static_assert(!std::is_same_v<T, std::string>, "Use 'DeserializeStdString' for string serialization.");
#ifdef DEBUG_SERIALIZATION
		std::cout << "Pos: " << oStream.tellp() << " Writing to stream: " << typeid(T).name() << " size: " << sizeof(T) << " | " << std::bitset<sizeof(T) * 8>(item).to_string() << std::endl;
#endif
		oStream.write((char*)&item, sizeof(T));
	}

	template<typename T>
	void readFromStream(std::istream& iStream, T& item) {
		iStream.read((char*)&item, sizeof(T));
#ifdef DEBUG_SERIALIZATION
		//split up since it froze the program at some point, but didnt do it again after rebuild for some reason
		std::cout << "Pos: ";
		std::cout << iStream.tellg();
		std::cout << " Reading from stream: ";
		std::cout << typeid(T).name();
		std::cout << " size: ";
		std::cout << sizeof(T);
		std::cout << " | ";
		std::cout << std::bitset<sizeof(T) * 8>(item).to_string();
		std::cout << std::endl;
#endif
	}

	inline std::ostream& Serialize(std::ostream& oStream, const std::string& str) {
#ifdef DEBUG_SERIALIZATION
		std::cout << "Pos: " << oStream.tellp() << " Writing string size to stream" << std::endl;
#endif
		int size = static_cast<int>(str.size());
		writeToStream(oStream, size);
#ifdef DEBUG_SERIALIZATION
		std::cout << "Pos: " << oStream.tellp() << " Writing string '" << str << "' data to stream" << std::endl;
#endif
		oStream.write(&str[0], size);
		//oStream << str;
		return oStream;
	}

	inline std::string DeserializeStdString(std::istream& stream, const int stringSizeCap = 2048) {
#ifdef DEBUG_SERIALIZATION
		std::cout << "Pos: " << stream.tellg() << " Reading string from stream" << std::endl;
#endif

		int stringSize;
		readFromStream(stream, stringSize);

		if (stringSize > stringSizeCap) {
			stringSize = stringSizeCap; //limit in case we read from an invalid stream
			std::cerr << "ERROR while reading from Stream: Stringsize limit reached" << std::endl;
		}

		char* chars = new char[stringSize + 1];
		chars[stringSize] = 0;
#ifdef DEBUG_SERIALIZATION
		std::cout << "Pos: " << stream.tellg() << " Reading string data from stream - size: " << stringSize << std::endl;
#endif
		stream.read(chars, stringSize);
		std::string s = { chars };

		//std::string s;
		//std::getline(stream, s); //first should be new line operator
		//std::getline(stream, s);
#ifdef DEBUG_SERIALIZATION
		std::cout << "Read string from stream:" << s << std::endl;
#endif

		delete[] chars;
		return s;
	}
}
