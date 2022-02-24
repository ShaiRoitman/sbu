#pragma once

#include <iostream>
#include <fstream>
#include <string>

class StandardOutputWrapper
{
protected:
	StandardOutputWrapper();
	~StandardOutputWrapper();
	static StandardOutputWrapper* singleton;

	std::filebuf fb;
	std::ostream* outputStream;
	std::ostream* active;
public:
	StandardOutputWrapper(StandardOutputWrapper& other) = delete;
	void operator =(const StandardOutputWrapper&) = delete;
	static StandardOutputWrapper* GetInstance();
	void Init(const std::string& outputPath);

	std::ostream& GetStream();
	void Output(const std::string& value);
	void OutputLine(const std::string& value);
	void EOL();
	template<typename T> StandardOutputWrapper& operator <<(T& value)
	{
		(*this->active) << value;
		return *this;
	}
};
