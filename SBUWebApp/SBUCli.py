import SBUGlobals
from SBUUtils import InitApp
import SBUOperations
import argparse

InitApp()
parser = argparse.ArgumentParser(description='SBU CommandLine Wrapper')
parser.add_argument('action', choices=['backup', 'restore'])
parser.add_argument('--backupDef', type=str)
parser.add_argument('--destination', type=str)
parser.add_argument('--timeStamp', type=str)

args = parser.parse_args()

if args.action == 'backup':
    SBUOperations.backup(args.backupDef)
if args.action == 'restore':
    SBUOperations.restore(args.backupDef, args.destination, args.timeStamp)
