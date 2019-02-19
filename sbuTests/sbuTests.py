import random
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
    ExitCode_GeneralFailure             = 6
    
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
        self.repositoryPath = None
        self.workDir = None

    def formatCmdLine(self, cmdLine):
        return ' '.join(self.executeCmdLine(cmdLineArgs));

    def executeCmdLine(self, cmdLine):
        cmdLineArgs = cmdLine.split(" ")
        cmdLineArgs.insert(0, self.sbuExecutable)
        if (self.configFile != None):
            cmdLineArgs.append("--config")
            cmdLineArgs.append(self.configFile)
        if (self.repositoryPath != None):
            cmdLineArgs.append("--FileRepository.path")
            cmdLineArgs.append(self.repositoryPath)
            cmdLineArgs.append("--FileRepository.name")
            cmdLineArgs.append("fileRepository.db")
        if (self.workDir != None):
            cmdLineArgs.append("--General.Workdir")
            cmdLineArgs.append(self.workDir)

        return cmdLineArgs

    def Execute(self, cmdLine):
        cmdLineArgs = self.executeCmdLine(cmdLine)

        logging.info("Executing cmdline [{0}]".format(' '.join(cmdLineArgs)))
        result = ExecuteResult()
        p = subprocess.Popen(cmdLineArgs, stdout=subprocess.PIPE)
        result.returnCode = p.wait()
        result.output = self.__read_as_utf8(p.stdout)

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

    def __read_as_utf8(self, fileno):
        fp = io.TextIOWrapper(fileno, "utf-8")
        retValue = (fp.read())
        fp.close()
        return retValue    


def dirCompare(left, right):
    cmdLines = "c:\\dropbox\\apps\\bin\\diff.exe -r %s %s" % (left, right)
    cmdLines.split(" ")
    p = subprocess.Popen(cmdLines, stdout=subprocess.PIPE)
    noArgsOutput = p.communicate()[0]
    print (noArgsOutput)
    retValue = p.returncode
    return retValue==0

class TestSanity(unittest.TestCase):
    tmp = None
    origin = None

    @classmethod
    def setUpClass(cls):
        TestNightly.tmp = "C:\\workdir"
        TestNightly.origin = os.getcwd()
        os.makedirs(TestNightly.tmp, exist_ok=True)
        os.chdir(TestNightly.tmp)

    @classmethod
    def tearDownClass(cls):
        os.chdir(TestNightly.origin)
        # shutil.rmtree(TestNightly.tmp, ignore_errors=False)


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
        app.ListBackup("clu")
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
        # shutil.rmtree(TestNightly.tmp, ignore_errors=False)

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
    
    def createRandomFile(self, name, size, var):
        actualSize = size + random.randint(-var, var)
        os.makedirs(os.path.dirname(name),exist_ok=True)
        with open(name, 'wb') as fout:
            fout.write(os.urandom(actualSize))

    def test_BackupRestore(self):
        cmdLine = SbuCmdLine()
        cmdLine.repositoryPath = os.path.join(TestNightly.tmp,"Repo")

        srcDir = os.path.join(TestNightly.tmp, "source")
        targetDir = os.path.join(TestNightly.tmp, "target")
        repoDir = os.path.join(TestNightly.tmp, "Repo")

        self.createRandomFile( os.path.join(srcDir,"FirstFile"), 64*1024, 16*1024)
        self.createRandomFile( os.path.join(srcDir,"SecondFile"), 64*1024, 16*1024)
        self.createRandomFile( os.path.join(srcDir,"ThirdFile"), 64*1024, 16*1024)

        cmdLine.CreateBackupDef("test", srcDir)
        cmdLine.Backup("test")
        cmdLine.Restore("test", targetDir )
        self.assertTrue(dirCompare(srcDir, targetDir))

        logging.info("Done Testing")


if __name__ == '__main__':
    try:
        unittest.main()
    except:
        pass
