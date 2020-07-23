#pragma once

#include "FileRepositoryDB.h"
#include "RepositoryDB.h"
#include <boost/program_options.hpp>

std::shared_ptr<IFileRepositoryDB> getFileRepository(boost::program_options::variables_map& vm);
std::shared_ptr<IRepositoryDB> getRepository(boost::program_options::variables_map& vm);
