import unittest
import subprocess
import os
import shutil

class TestStringMethods(unittest.TestCase):

    def dirCompare(self, left, right):
        cmdLines = """c:\dropbox\apps\bin\diff.exe -r %s %s""" % (left, right)
        cmdLines.split(" ")
        p = subprocess.Popen(cmdLines, stdout=subprocess.PIPE)
        noArgsOutput = p.communicate()
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
        noArgsOutput = p.communicate()
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



if __name__ == '__main__':
    try:
        unittest.main()
    except:
        pass