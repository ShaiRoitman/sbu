#include "StandardOutputWrapper.h"

StandardOutputWrapper* StandardOutputWrapper::singleton = nullptr;

StandardOutputWrapper::StandardOutputWrapper()
{
	this->active = &std::cout;
	outputStream = nullptr;
}

StandardOutputWrapper::~StandardOutputWrapper()
{
	if (this->outputStream != nullptr)
	{
		this->fb.close();
		delete this->outputStream;
		this->outputStream = nullptr;
		this->active = &std::cout;
	}
}

StandardOutputWrapper* StandardOutputWrapper::GetInstance()
{
	if (singleton == nullptr) {
		singleton = new StandardOutputWrapper();
	}
	return singleton;
}

void StandardOutputWrapper::Init(const std::string& outputPath)
{
	if (this->fb.open(outputPath, std::ios::out) != nullptr)
	{
		this->outputStream = new std::ostream(&this->fb);
		this->active = this->outputStream;
	}
}

std::ostream& StandardOutputWrapper::GetStream()
{
	return *this->active;
}

void StandardOutputWrapper::Output(const std::string& value)
{
	(*this->active) << value;
}

void StandardOutputWrapper::OutputLine(const std::string& value)
{
	(*this->active) << value << std::endl;
}

void StandardOutputWrapper::EOL()
{
	(*this->active) << std::endl;
}