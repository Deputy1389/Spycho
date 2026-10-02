"""Local UE workflow. Requires Python 3; no third-party Python packages."""
import argparse,json,os,subprocess,sys
from pathlib import Path

project=Path(__file__).resolve().parents[1]
parser=argparse.ArgumentParser()
parser.add_argument('action',choices=['build','test','smoke','assets','play','host','join'])
parser.add_argument('address',nargs='?',default='127.0.0.1:7777')
args=parser.parse_args()
if os.environ.get('SPYCHO_UE_ROOT'): engine=Path(os.environ['SPYCHO_UE_ROOT'])
else:
    manifest=Path(os.environ.get('PROGRAMDATA','C:/ProgramData'))/'Epic/UnrealEngineLauncher/LauncherInstalled.dat'
    installs=json.loads(manifest.read_text())['InstallationList']
    engines=[i for i in installs if i.get('AppName','').startswith('UE_5.')]
    engine=Path(max(engines,key=lambda i:tuple(map(int,i['AppName'][3:].split('.'))))['InstallLocation'])
uproject=str(project/'Spycho.uproject')
if args.action=='build':
    command=['cmd.exe','/c',str(engine/'Engine/Build/BatchFiles/Build.bat'),'SpychoEditor','Win64','Development',uproject,'-WaitMutex','-NoHotReloadFromIDE']
else:
    executable='UnrealEditor.exe' if args.action in ['play','host','join'] else 'UnrealEditor-Cmd.exe'
    command=[str(engine/'Engine/Binaries/Win64'/executable),uproject]
    if args.action in ['play','host','join','smoke']:
        command+=[args.address if args.action=='join' else '/Game/Maps/House'+('?listen' if args.action=='host' else ''),'-game']
    if args.action in ['play','host','join']: command+=['-windowed','-ResX=1280','-ResY=720','-nosplash']
    else: command+=['-unattended','-nosplash','-nullrhi','-stdout','-FullStdOutLogOutput']
    if args.action=='smoke': command+=['-SpychoSmoke']
    if args.action=='test': command+=['-ExecCmds=Automation RunTests Spycho','-TestExit=Automation Test Queue Empty','-ReportExportPath='+str(project/'Saved/Automation')]
    if args.action=='assets':
        subprocess.run([sys.executable,str(project/'Tools/generate_audio.py')],check=True)
        command+=['-ExecutePythonScript='+str(project/'Tools/create_assets.py')]
sys.exit(subprocess.call(command))
