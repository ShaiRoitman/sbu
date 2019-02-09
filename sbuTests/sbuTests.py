import unittest
import subprocess
import os
import shutil
import filecmp

class TestStringMethods(unittest.TestCase):
    def setUp(self):
        self.executablePath = """C:\git\sbu\sbu\SbuCli\Debug\sbu.exe"""
        self.testPath = """c:\git\sbu\sbuTests"""

    def executeCmdLine(self, cmdLine):
        cmdLineArgs = cmdLine.split(" ")
        cmdLineArgs.insert(0, self.executablePath)
        p = subprocess.Popen(cmdLineArgs, stdout=subprocess.PIPE)
        noArgsOutput = p.communicate()
        return [noArgsOutput, p.returncode]

    def test_Spawn(self):
        output = self.executeCmdLine("hello")

        self.assertEqual( output[1] , 0 )

    def test_Sanity(self):
        workDir = """c:\workdir"""
        srcPath = """c:\git\clu"""
        dstPath = """c:\git\clu2"""

        if (os.path.exists(workDir)):
            shutil.rmtree(workDir)
        os.makedirs(workDir)
        os.chdir(workDir)
        if (os.path.exists(dstPath)):
            shutil.rmtree(dstPath)

        self.executeCmdLine("""--action CreateBackupDef --name clu --path """ + srcPath)
        self.executeCmdLine("""--action ListBackupDef """)
        self.executeCmdLine("""--action Backup --name clu """)
        self.executeCmdLine("""--action ListBackup --name clu """)
        self.executeCmdLine("""--action Restore --name clu --path """ + dstPath)
        self.assertTrue( filecmp.cmp(srcPath,dstPath))

    def test_AddingBackupDef(self):
        pass



if __name__ == '__main__':
    try:
        unittest.main()
    except:
        pass