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
 * RestoreOptions.h
 *
 * 
 */

#ifndef RestoreOptions_H_
#define RestoreOptions_H_


#include "ModelBase.h"

#include <string>

namespace io {
namespace swagger {
namespace server {
namespace model {

/// <summary>
/// 
/// </summary>
class  RestoreOptions
    : public ModelBase
{
public:
    RestoreOptions();
    virtual ~RestoreOptions();

    /////////////////////////////////////////////
    /// ModelBase overrides

    void validate() override;

    nlohmann::json toJson() const override;
    void fromJson(nlohmann::json& json) override;

    /////////////////////////////////////////////
    /// RestoreOptions members

    /// <summary>
    /// 
    /// </summary>
    int64_t getId() const;
    void setId(int64_t value);
    bool idIsSet() const;
    void unsetId();
    /// <summary>
    /// 
    /// </summary>
    std::string getDate() const;
    void setDate(std::string value);
    bool dateIsSet() const;
    void unsetdate();

protected:
    int64_t m_Id;
    bool m_IdIsSet;
    std::string m_date;
    bool m_dateIsSet;
};

}
}
}
}

#endif /* RestoreOptions_H_ */
