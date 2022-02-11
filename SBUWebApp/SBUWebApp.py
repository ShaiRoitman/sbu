import SBUGlobals

from SBUUtils import InitApp
import uvicorn

InitApp()

if __name__ == "__main__":
    webServerConfig = SBUGlobals.configuration["WebServer"]
    uvicorn.run("SBUWebServer:app",
                host=webServerConfig["IP"],
                port=webServerConfig["Port"],
                log_level = webServerConfig["LogLevel"],
                )
