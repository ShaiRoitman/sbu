
#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupInfoOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		return 0;
	}
}; 
std::shared_ptr<Operation> BackupInfoFactory()
{
	return std::make_shared<BackupInfoOperation>();
}