import datetime
import logging
import unittest
import subprocess
import os
import shutil
import tempfile
import io

class Sbu_ExitCodes:
    ExitCode_Success					= 0
    
    ExitCode_AlreadyExists			    = 1
    ExitCode_InvalidArgument			= 2
    ExitCode_ConfigFileMissing		    = 3
    ExitCode_MissingAction			    = 4
    ExitCode_InvalidAction			    = 5
    
    ExitCode_HelpCalled				    = 1001
    ExitCode_VersionCalled			    = 1002

class ExecuteResult:
    def __init__(self):
        self.returnCode = None
        self.output = None

class SbuCmdLine:

    def __init__(self):
        self.sbuExecutable = """C:\git\sbu\sbu\SbuCli\Debug\sbu.exe"""
        self.configFile = None

    def Execute(self, cmdLine):
        cmdLineArgs = cmdLine.split(" ")
        cmdLineArgs.insert(0, self.sbuExecutable)
        if (self.configFile != None):
            cmdLineArgs.append("--config")
            cmdLineArgs.append(self.configFile)

        result = ExecuteResult()

        p = subprocess.Popen(cmdLineArgs, stdout=subprocess.PIPE)
        result.returnCode = p.wait()
        result.output = __read_as_utf8(p.stdout)

        return result

    def CreateBackupDef(self, name, path):
        retValue = self.Execute("""--action CreateBackupDef --name {0} --path {1} """.format( name, path))
        return retValue

    def ListBackupDef(self):
        retValue = self.Execute("--action ListBackupDef")
        return retValue

    def Backup(self, name):
        retValue = self.Execute("--action Backup --name {0}".format(name))
        return retValue

    def Restore(self, name, dest):
        retValue = self.Execute("--action Restore --name {0} --path {1}".format(name, dest))
        return retValue

    def ListBackup(self, name):
        retValue = self.Execute("--action ListBackup --name {0}".format(name))
        return retValue

    def __read_as_utf8(fileno):
        fp = io.TextIOWrapper(fileno, "utf-8")
        retValue = (fp.read())
        fp.close()
        return retValue    

class TestSanity(unittest.TestCase):

    def dirCompare(self, left, right):
        cmdLines = """c:\dropbox\apps\bin\diff.exe -r %s %s""" % (left, right)
        cmdLines.split(" ")
        p = subprocess.Popen(cmdLines, stdout=subprocess.PIPE)
        noArgsOutput = p.communicate()[0]
        print (noArgsOutput)
        retValue = p.returncode
        return retValue==0

    def setUp(self):
        self.executablePath = """C:\git\sbu\sbu\SbuCli\Debug\sbu.exe"""

    def test_Sanity(self):
        srcPath = """c:\git\clu"""
        dstPath = """c:\git\clu2"""

        app = SbuCmdLine()
        app.configFile = """c:\git\sbu\sbu\sbu.config"""

        app.CreateBackupDef("clu", srcPath)
        app.ListBackupDef()
        app.Backup("clu")
        app.ListBackup()
        app.Restore("clu", dstPath)
        self.assertTrue(self.dirCompare(srcPath,dstPath))

class TestNightly(unittest.TestCase):
    tmp = None
    origin = None

    @classmethod
    def setUpClass(cls):
        TestNightly.tmp = os.path.join(tempfile.gettempdir(), '{}'.format(datetime.datetime.now().strftime('%Y-%m-%d_%H_%M_%S')))
        TestNightly.origin = os.getcwd()
        os.makedirs(TestNightly.tmp)
        os.chdir(TestNightly.tmp)

    @classmethod
    def tearDownClass(cls):
        os.chdir(TestNightly.origin)
        shutil.rmtree(TestNightly.tmp, ignore_errors=False)

    def test_NonFunctionalCommandLine(self):
        cmdLine = SbuCmdLine()

        helpResult = cmdLine.Execute("--help")
        self.assertEqual( helpResult.returnCode, Sbu_ExitCodes.ExitCode_HelpCalled, "Help command line variable failed")

        versionResult = cmdLine.Execute("--version")
        self.assertEqual( versionResult.returnCode, Sbu_ExitCodes.ExitCode_VersionCalled, "Version  command line variable failed")

        actionUnknown = cmdLine.Execute("--action Unknown")
        self.assertEqual( actionUnknown.returnCode, Sbu_ExitCodes.ExitCode_InvalidAction, "Invalid action command line variable failed")

        actionMissing = cmdLine.Execute("--name shai")
        self.assertEqual( actionMissing.returnCode, Sbu_ExitCodes.ExitCode_MissingAction , "Missing action command line variable failed")

        sbuConfigPath = os.path.join(TestNightly.tmp, "sbu.config")

        os.environ["SBU_CONFIG"] = sbuConfigPath
        configFileMissingByEnv = cmdLine.Execute("--action ListBackupDef ")
        self.assertEqual( configFileMissingByEnv.returnCode, Sbu_ExitCodes.ExitCode_ConfigFileMissing, "Config file missing set by environment ")
        del os.environ["SBU_CONFIG"]

        configFileMissingByEnv = cmdLine.Execute("--action ListBackupDef --config " + sbuConfigPath)
        self.assertEqual( configFileMissingByEnv.returnCode, Sbu_ExitCodes.ExitCode_ConfigFileMissing, "Config file missing set by file")

    def test_Logging(self):
        cmdLine = SbuCmdLine()
        d = cwd = os.getcwd()
        noLoggingResult = cmdLine.Execute("--version")
        self.assertTrue(noLoggingResult.output.find("Init Application") == -1)

        consoleLoggingResult = cmdLine.Execute("--version --Logging.Console true")
        self.assertTrue(consoleLoggingResult.output.find("Init Application") != -1)

        fileLoggingResult = cmdLine.Execute("--version --Logging.FileOutput sbu.log")
        with open('sbu.log', 'r') as myfile:
            data=myfile.read().replace('\n', '')
        self.assertTrue(data.find("Init Application") != -1)
        os.remove("sbu.log")

        bothResult = cmdLine.Execute("--version --Logging.FileOutput sbu.log --Logging.Console true")
        self.assertTrue(bothResult.output.find("Init Application") != -1)
        with open('sbu.log', 'r') as myfile:
            data=myfile.read().replace('\n', '')
        self.assertTrue(data.find("Init Application") != -1)
        os.remove("sbu.log")

    def test_BackupDefs(self):
        cmdLine = SbuCmdLine()
        
        AddResult1 = cmdLine.CreateBackupDef("Shai", "c:\\dev")
        firstLine = AddResult1.output.split("\n")[0].split(',')

        logging.info("Testing CreateBackupDef Format")
        self.assertEqual(int(str(firstLine[0])),1)
        self.assertEqual(firstLine[1],"Shai")
        self.assertEqual(firstLine[3].replace("\"",""),"c:\\dev")
        
        AddResult2 = cmdLine.CreateBackupDef("Ariel", "c:\\Windows")
        AddResult3 = cmdLine.CreateBackupDef("Eitan", "c:\\Temp")
        
        listResult = cmdLine.ListBackupDef()
        self.assertEqual(len(listResult.output.strip().split('\n')), 3)

        duplicateAdd = cmdLine.CreateBackupDef("Shai", "c:\\blah")
        self.assertEqual(duplicateAdd.returnCode, Sbu_ExitCodes.ExitCode_AlreadyExists)

        listResultPostdup = cmdLine.ListBackupDef()
        self.assertEqual(listResult.output,listResultPostdup.output)
    
if __name__ == '__main__':
    try:
        unittest.main()
    except:
        pass
