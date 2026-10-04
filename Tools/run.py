"""Local UE workflow. Requires Python 3; no third-party Python packages."""
import argparse,json,os,subprocess,sys
from pathlib import Path

project=Path(__file__).resolve().parents[1]
os.environ['COMSPEC']=str(Path(os.environ.get('SystemRoot','C:/Windows'))/'System32/cmd.exe')
parser=argparse.ArgumentParser()
parser.add_argument('action',choices=['build','test','smoke','botsmoke','polishsmoke','experiencesmoke','mansionsmoke','assets','play','host','join','capture'])
parser.add_argument('address',nargs='?',default='127.0.0.1:7777')
parser.add_argument('--view',choices=['hall','aim','fire','reload','bot','plan','door','lounge','menu','foyer','library','ballroom','music','gallery','salon'],default='foyer')
args=parser.parse_args()
if os.environ.get('SPYCHO_UE_ROOT'): engine=Path(os.environ['SPYCHO_UE_ROOT'])
else:
    manifest=Path(os.environ.get('PROGRAMDATA','C:/ProgramData'))/'Epic/UnrealEngineLauncher/LauncherInstalled.dat'
    installs=json.loads(manifest.read_text())['InstallationList']
    engines=[i for i in installs if i.get('AppName','').startswith('UE_5.')]
    engine=Path(max(engines,key=lambda i:tuple(map(int,i['AppName'][3:].split('.'))))['InstallLocation'])
uproject=str(project/'Spycho.uproject')
if args.action=='build':
    subprocess.run([sys.executable,str(project/'Tools/generate_mansion_layout.py')],check=True)
    command=['cmd.exe','/c',str(engine/'Engine/Build/BatchFiles/Build.bat'),'SpychoEditor','Win64','Development',uproject,'-WaitMutex','-NoHotReloadFromIDE']
else:
    executable='UnrealEditor.exe' if args.action in ['play','host','join','capture'] else 'UnrealEditor-Cmd.exe'
    command=[str(engine/'Engine/Binaries/Win64'/executable),uproject]
    if args.action in ['play','host','join','smoke','botsmoke','polishsmoke','experiencesmoke','mansionsmoke','capture']:
        command+=[args.address if args.action=='join' else '/Game/Maps/House'+('?listen' if args.action=='host' else ''),'-game']
    if args.action in ['play','host','join','capture']: command+=['-windowed','-ResX=1280','-ResY=720','-nosplash']
    else: command+=['-unattended','-nosplash','-nullrhi','-stdout','-FullStdOutLogOutput']
    if args.action=='smoke': command+=['-SpychoSmoke']
    if args.action=='botsmoke':command+=['-SpychoBotSmoke']
    if args.action=='polishsmoke':command+=['-SpychoPolishSmoke']
    if args.action=='experiencesmoke':command+=['-SpychoExperienceSmoke']
    if args.action=='mansionsmoke':command+=['-SpychoMansionSmoke']
    if args.action=='capture':
        command+=['-SpychoCapture','-unattended']
        if args.view!='hall':command+=['-SpychoCapture'+args.view.title()]
    if args.action=='test': command+=['-ExecCmds=Automation RunTests Spycho','-TestExit=Automation Test Queue Empty','-ReportExportPath='+str(project/'Saved/Automation')]
    if args.action=='assets':
        # Unreal can return zero after a Python exception; require script completion.
        for script,marker in [('import_recorded_audio.py','SPYCHO_RECORDED_AUDIO_COMPLETE'),('import_duel_assets.py','SPYCHO_DUEL_IMPORT_COMPLETE'),('build_mansion.py','SPYCHO_MANSION_COMPLETE')]:
            result=subprocess.run(command+['-ExecutePythonScript='+str(project/'Tools'/script)],check=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,encoding='utf-8',errors='replace')
            print(result.stdout)
            if marker not in result.stdout or 'Traceback (most recent call last)' in result.stdout:
                raise RuntimeError('Unreal asset script failed: '+script)
        sys.exit(0)
sys.exit(subprocess.call(command))
