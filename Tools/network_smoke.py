"""Two separate real processes; a remote RPC kills through the saved map's wall."""
import json,os,subprocess,time,sys
from pathlib import Path
project=Path(__file__).resolve().parents[1]
if len(sys.argv)>1:base=[str(Path(sys.argv[1]).resolve())]
else:
    manifest=Path(os.environ.get('PROGRAMDATA','C:/ProgramData'))/'Epic/UnrealEngineLauncher/LauncherInstalled.dat'
    engine=Path(os.environ['SPYCHO_UE_ROOT']) if os.environ.get('SPYCHO_UE_ROOT') else Path(next(i['InstallLocation'] for i in json.loads(manifest.read_text())['InstallationList'] if i['AppName']=='UE_5.8'))
    base=[str(engine/'Engine/Binaries/Win64/UnrealEditor-Cmd.exe'),str(project/'Spycho.uproject')]
common=['-game','-nullrhi','-unattended','-nosplash','-stdout','-FullStdOutLogOutput']
logs=project/'Saved/NetworkSmoke';logs.mkdir(parents=True,exist_ok=True)
hostlog=logs/'host.log';clientlog=logs/'client.log'
host=None;client=None
try:
    with hostlog.open('w') as h,clientlog.open('w') as c:
        host=subprocess.Popen(base+['/Game/Maps/House?listen']+common+['-SpychoNetSmoke','-port=17777'],stdout=h,stderr=subprocess.STDOUT)
        deadline=time.monotonic()+90
        while time.monotonic()<deadline:
            if host.poll() is not None: raise RuntimeError('Host exited before connection')
            if 'GameNetDriver' in hostlog.read_text(errors='replace') and '17777' in hostlog.read_text(errors='replace'):break
            time.sleep(.5)
        else:raise RuntimeError('Host did not begin listening')
        client=subprocess.Popen(base+['127.0.0.1:17777']+common,stdout=c,stderr=subprocess.STDOUT)
        client.wait(timeout=90);host.wait(timeout=30)
    ht=hostlog.read_text(errors='replace');ct=clientlog.read_text(errors='replace')
    for text in [ht,ct]:
        for line in text.splitlines():
            if 'SPYCHO_NET_' in line:print(line)
    passed=host.returncode==0 and client.returncode==0 and 'SPYCHO_NET_SERVER PASS final reset' in ht and 'SPYCHO_NET_CLIENT PASS replicated automatic reset' in ct and 'SPYCHO_NET_CLIENT FAIL' not in ct
    print('NETWORK SMOKE:', 'PASS' if passed else 'FAIL', 'logs:',logs)
    sys.exit(0 if passed else 1)
finally:
    for p in [host,client]:
        if p and p.poll() is None:p.terminate();p.wait(timeout=10)
