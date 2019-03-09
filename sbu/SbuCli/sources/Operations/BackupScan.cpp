#include "Operations.h"

#include <iostream>
#include "utils.h"
#include "factories.h"
#include "sbu_exceptions.h"
#include "ExitCodes.h"

static auto logger = LoggerFactory::getLogger("Operations");

class BackupScanOperation : public Operation
{
public:
	int Operate(boost::program_options::variables_map& vm)
	{
		return ExitCode_Success;
	}
};

std::shared_ptr<Operation> BackupScanFactory()
{
	return std::make_shared<BackupScanOperation>();
}
