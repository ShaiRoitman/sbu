#pragma once

#include <exception>

class sbu_alreadyexists : public std::exception
{
public:
};

class sbu_missingParameter : public std::exception
{
public:
};
