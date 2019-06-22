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
 * CreateBackupDef.h
 *
 * 
 */

#ifndef CreateBackupDef_H_
#define CreateBackupDef_H_


#include "ModelBase.h"

#include <string>

namespace io {
namespace swagger {
namespace server {
namespace model {

/// <summary>
/// 
/// </summary>
class  CreateBackupDef
    : public ModelBase
{
public:
    CreateBackupDef();
    virtual ~CreateBackupDef();

    /////////////////////////////////////////////
    /// ModelBase overrides

    void validate() override;

    nlohmann::json toJson() const override;
    void fromJson(nlohmann::json& json) override;

    /////////////////////////////////////////////
    /// CreateBackupDef members

    /// <summary>
    /// 
    /// </summary>
    std::string getName() const;
    void setName(std::string value);
    bool nameIsSet() const;
    void unsetName();
    /// <summary>
    /// 
    /// </summary>
    std::string getPath() const;
    void setPath(std::string value);
    bool pathIsSet() const;
    void unsetPath();

protected:
    std::string m_Name;
    bool m_NameIsSet;
    std::string m_Path;
    bool m_PathIsSet;
};

}
}
}
}

#endif /* CreateBackupDef_H_ */
