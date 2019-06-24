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


#include "BackupDefs.h"

namespace io {
namespace swagger {
namespace server {
namespace model {

BackupDefs::BackupDefs()
{
    m_BackupdefsIsSet = false;
    
}

BackupDefs::~BackupDefs()
{
}

void BackupDefs::validate()
{
    // TODO: implement validation
}

nlohmann::json BackupDefs::toJson() const
{
    nlohmann::json val = nlohmann::json::object();

    {
        nlohmann::json jsonArray;
        for( auto& item : m_Backupdefs )
        {
            jsonArray.push_back(ModelBase::toJson(item));
        }
        
        if(jsonArray.size() > 0)
        {
            val["backupdefs"] = jsonArray;
        }
    }
    

    return val;
}

void BackupDefs::fromJson(nlohmann::json& val)
{
    {
        m_Backupdefs.clear();
        nlohmann::json jsonArray;
        if(val.find("backupdefs") != val.end())
        {
        for( auto& item : val["backupdefs"] )
        {
            
            if(item.is_null())
            {
                m_Backupdefs.push_back( std::shared_ptr<BackupDef>(nullptr) );
            }
            else
            {
                std::shared_ptr<BackupDef> newItem(new BackupDef());
                newItem->fromJson(item);
                m_Backupdefs.push_back( newItem );
            }
            
        }
        }
    }
    
}


std::vector<std::shared_ptr<BackupDef>>& BackupDefs::getBackupdefs()
{
    return m_Backupdefs;
}
bool BackupDefs::backupdefsIsSet() const
{
    return m_BackupdefsIsSet;
}
void BackupDefs::unsetBackupdefs()
{
    m_BackupdefsIsSet = false;
}

}
}
}
}

