"""Build and cook a Development Win64 prototype to a supplied output folder."""
import json,os,subprocess,sys
from pathlib import Path
project=Path(__file__).resolve().parents[1]
os.environ['COMSPEC']=str(Path(os.environ.get('SystemRoot','C:/Windows'))/'System32/cmd.exe')
if len(sys.argv)!=2:raise SystemExit('Usage: python Tools/package.py OUTPUT_DIRECTORY')
dest=Path(sys.argv[1]).resolve()
manifest=Path(os.environ.get('PROGRAMDATA','C:/ProgramData'))/'Epic/UnrealEngineLauncher/LauncherInstalled.dat'
engine=Path(os.environ['SPYCHO_UE_ROOT']) if os.environ.get('SPYCHO_UE_ROOT') else Path(next(i['InstallLocation'] for i in json.loads(manifest.read_text())['InstallationList'] if i['AppName']=='UE_5.8'))
command=['cmd.exe','/c',str(engine/'Engine/Build/BatchFiles/RunUAT.bat'),'BuildCookRun','-project='+str(project/'Spycho.uproject'),'-noP4','-platform=Win64','-clientconfig=Development','-build','-ubtargs=-NoUBTMakefiles','-cook','-map=/Game/Maps/House','-stage','-pak','-archive','-archivedirectory='+str(dest),'-unattended','-utf8output','-prereqs']
sys.exit(subprocess.call(command))
