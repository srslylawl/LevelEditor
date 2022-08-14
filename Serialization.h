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

		Serializable(const std::string& name) : Name(name) {}

		virtual void Serialize(std::ostream& oStream) const = 0;

		static bool Deserialize(std::istream& iStream, SerializableT*& out_T) {
			return SerializableT::Deserialize(iStream, out_T);
		}
	};

	template<typename T>
	void writeToStream(std::ostream& oStream, const T& item) {
		static_assert(!std::is_same_v<T, std::string>, "Use 'DeserializeStdString' for string serialization.");
		std::cout << "Pos: " << oStream.tellp() << " Writing to stream: " << typeid(T).name() << " size: " << sizeof(T) << " | " << std::bitset<sizeof(T)*8>(item).to_string() << std::endl;
		oStream.write((char*)&item, sizeof(T));
	}

	template<typename T>
	void readFromStream(std::istream& iStream, T& item) {
		iStream.read((char*)&item, sizeof(T));
		std::cout << "Pos: " << iStream.tellg() << " Reading from stream: " << typeid(T).name() << " size: " << sizeof(T) << " | " << std::bitset<sizeof(T)*8>(item).to_string() << std::endl;
	}

	inline std::ostream& Serialize(std::ostream& oStream, const std::string& str) {
		std::cout << "Pos: " << oStream.tellp() << " Writing string size to stream" << std::endl;
		int size = static_cast<int>(str.size());
		writeToStream(oStream, size);
		//oStream << "\n" << str << "\n";
		std::cout << "Pos: " << oStream.tellp()<< " Writing string '" << str << "' data to stream" << std::endl;
		oStream.write(&str[0], size);
		//oStream << str;
		return oStream;
	}

	inline std::string DeserializeStdString(std::istream& stream, const int stringSizeCap = 2048) {
		std::cout << "Pos: " << stream.tellg() << " Reading string from stream" << std::endl;
		int stringSize;
		readFromStream(stream, stringSize);

		if (stringSize > stringSizeCap) {
			stringSize = stringSizeCap; //limit in case we read from an invalid stream
			std::cout << "ERROR while reading from Stream: Stringsize limit reached" << std::endl;
		}

		char* chars = new char[stringSize + 1];
		chars[stringSize] = 0;
		std::cout << "Pos: " << stream.tellg() << " Reading string data from stream - size: " << stringSize <<  std::endl;
		stream.read(chars, stringSize);
		std::string s = { chars };

		//std::string s;
		//std::getline(stream, s); //first should be new line operator
		//std::getline(stream, s);

		std::cout << "Read string from stream:" << s << std::endl;

		delete[] chars;
		return s;
	}
}
