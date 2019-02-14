import logging
import unittest
import subprocess
import os
import shutil
import tempfile

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

    def Execute(self, cmdLine):
        cmdLineArgs = cmdLine.split(" ")
        cmdLineArgs.insert(0, self.sbuExecutable)
        result = ExecuteResult()

        p = subprocess.Popen(cmdLineArgs, stdout=subprocess.PIPE)
        result.output = str(p.communicate()[0])
        result.returnCode = p.returncode

        return result

    def CreateBackupDef(self, name, path):
        retValue = self.Execute("""--action CreateBackupDef --name {0} --path {1} """.format( name, path))
        return retValue

    def ListBackupDef(self):
        retValue = self.Execute("--action ListBackupDef")
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
        self.testPath = """c:\git\sbu\sbuTests"""


    def executeCmdLine(self, cmdLine):
        cmdLineArgs = cmdLine.split(" ")
        cmdLineArgs.insert(0, self.executablePath)
        cmdLineArgs.append("--config")
        cmdLineArgs.append("""c:\git\sbu\sbu\sbu.config""")
        p = subprocess.Popen(cmdLineArgs, stdout=subprocess.PIPE)
        noArgsOutput = p.communicate()[0]
        print (noArgsOutput)
        return [noArgsOutput, p.returncode]

    def test_Spawn(self):
        output = self.executeCmdLine("hello")

        self.assertEqual( output[1] , 0 )

    def test_Sanity(self):
        workDir = """c:\workdir"""
        srcPath = """c:\git\clu"""
        dstPath = """c:\git\clu2"""
        repo = """c:\git\sbu\Repo"""

        if (os.path.exists(workDir)):
            shutil.rmtree(workDir)
        if (os.path.exists(repo)):
            shutil.rmtree(repo)
        os.makedirs(workDir)
        os.chdir(workDir)
        if (os.path.exists(dstPath)):
            shutil.rmtree(dstPath)

        self.executeCmdLine("""--action CreateBackupDef --name clu --path """ + srcPath)
        self.executeCmdLine("""--action ListBackupDef """)
        self.executeCmdLine("""--action Backup --name clu """)
        self.executeCmdLine("""--action ListBackup --name clu """)
        self.executeCmdLine("""--action Restore --name clu --path """ + dstPath)
        self.assertTrue(self.dirCompare(srcPath,dstPath))
    def test_AddingBackupDef(self):
        pass

class TestNightly(unittest.TestCase):
    tmp = None

    @classmethod
    def setUpClass(cls):
        TestNightly.tmp = os.path.join(tempfile.gettempdir(), '.{}'.format(hash(os.times())))
        os.makedirs(TestNightly.tmp)
        os.chdir(TestNightly.tmp)

    @classmethod
    def tearDownClass(cls):
        shutil.rmtree(TestNightly.tmp, ignore_errors=True)

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
        firstLine = AddResult1.output.split("\n")
        AddResult2 = cmdLine.CreateBackupDef("Ariel", "c:\\Windows")
        AddResult3 = cmdLine.CreateBackupDef("Eitan", "c:\\Temp")
        listResult = cmdLine.ListBackupDef()
        duplicateAdd = cmdLine.CreateBackupDef("Shai", "c:\\blah")
        self.assertEqual(duplicateAdd.returnCode, Sbu_ExitCodes.ExitCode_AlreadyExists)
        listResultPostdup = cmdLine.ListBackupDef()

        self.assertEqual(listResult.output == listResultPostdup.output)

        testCases = """
        Add a 3 different backupdefs -> succeed
        List the backupdefs -> succeed, check format
        Add a non unique backupdef -> fail
        Delete a backupdefs -> success
        Add a backupdef currently deleted -> success
        Delete a non exists backupdef  -> fail
        List backupdefs -> succeed -> duplicate name should exist
        """
    


if __name__ == '__main__':
    try:
        unittest.main()
    except:
        pass