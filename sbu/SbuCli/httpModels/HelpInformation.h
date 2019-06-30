/**
* Smart Backup Utility
* HTTP Server for Smart Backup Utility
*
* OpenAPI spec version: 1.0.0
* Contact: Shai@Roitman.info
*
* NOTE: This class is auto generated by the swagger code generator program.
* https://github.com/swagger-api/swagger-codegen.git
* Do not edit the class manually.
*/
/*
 * HelpInformation.h
 *
 * 
 */

#ifndef HelpInformation_H_
#define HelpInformation_H_


#include "ModelBase.h"

#include <map>

namespace io {
namespace swagger {
namespace server {
namespace model {

/// <summary>
/// 
/// </summary>
class  HelpInformation
    : public ModelBase
{
public:
    HelpInformation();
    virtual ~HelpInformation();

    /////////////////////////////////////////////
    /// ModelBase overrides

    void validate() override;

    nlohmann::json toJson() const override;
    void fromJson(nlohmann::json& json) override;

    /////////////////////////////////////////////
    /// HelpInformation members


protected:
};

}
}
}
}

#endif /* HelpInformation_H_ */
