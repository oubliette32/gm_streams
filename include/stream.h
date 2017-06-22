#pragma once
#include <istream>

class Stream
{
private:
	std::basic_iostream<char>* innerStream;
public:

	///
	/// Destructor
	///
	~Stream()
	{
		if (!this->IsClosed())
			delete innerStream;
	}

	///
	/// Constructor
	///
	Stream(std::basic_iostream<char>* stream)
	{
		innerStream = stream;
	}

	///
	/// Gets the underlying stream
	///
	std::basic_iostream<char>* GetUnderlyingStream()
	{
		return innerStream;
	}

	///
	/// Closes the underlying stream object
	///
	void Close()
	{
		if (innerStream)
		{
			delete innerStream;
			innerStream = 0;
		}
	}

	///
	/// Clear
	///
	void Clear()
	{
		innerStream->clear();
	}

	///
	/// Returns whether or not this stream is closed
	///
	bool IsClosed()
	{
		return innerStream == 0;
	}

	///
	/// WARNING: THIS WONT CHECK IF THE STREAM WAS CLOSED
	///
	template <typename T>
	T Read()
	{
		T var; innerStream->read((char*)&var, sizeof(T));
		return var;
	}

	///
	/// WARNING: THIS WONT CHECK IF THE STREAM WAS CLOSED
	///
	template <typename T>
	void Write(T var)
	{
		innerStream->write((const char*)&var, sizeof(T));
	}

	void ReadString(char* buf, int len)
	{
		innerStream->read(buf, len);
	}

	void WriteString(const char* buf, int len)
	{
		innerStream->write(buf, len);
	}

	std::streamoff TellRead()
	{
		return innerStream->tellg();
	}

	std::streamoff TellWrite()
	{
		return innerStream->tellp();
	}

	void SeekWrite(std::streamoff pos)
	{
		innerStream->seekp(pos);
	}

	void SeekRead(std::streamoff pos)
	{
		innerStream->seekg(pos);
	}

	void Flush()
	{
		innerStream->flush();
	}

	std::streamoff Size()
	{
		std::streamoff old = this->TellRead();
		innerStream->seekg(0, std::ios::end);
		std::streamoff len = this->TellRead();
		innerStream->seekg(old);
		return len;
	}
};