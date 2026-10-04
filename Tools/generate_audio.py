"""Original mono placeholder sounds; deterministic, no external packages."""
from pathlib import Path
import math, random, wave, struct

ROOT=Path(__file__).resolve().parents[1]/'Content/Audio/Source'
ROOT.mkdir(parents=True,exist_ok=True)
RATE=44100
random.seed(1389)
def write(name,seconds,fn):
    previous=0.0
    frames=[]
    for i in range(int(RATE*seconds)):
        t=i/RATE; n=random.uniform(-1,1)
        previous=previous*.75+n*.25
        v=max(-.98,min(.98,fn(t,n,previous)))
        frames.append(struct.pack('<h',int(v*32767)))
    with wave.open(str(ROOT/(name+'.wav')),'wb') as w:
        w.setnchannels(1);w.setsampwidth(2);w.setframerate(RATE);w.writeframes(b''.join(frames))
def pulse(t,start,duration):
    x=t-start
    return (1-math.exp(-x*500))*math.exp(-x/duration) if x>=0 else 0
write('StepWood',.32,lambda t,n,l: pulse(t,0,.035)*(.4*l+.22*math.sin(t*2*math.pi*105))+pulse(t,.075,.07)*l*.2)
write('StepCarpet',.24,lambda t,n,l:pulse(t,0,.045)*l*.5)
write('StepTile',.28,lambda t,n,l:pulse(t,0,.018)*(.55*n+.23*math.sin(t*2*math.pi*850))+pulse(t,.04,.055)*l*.25)
write('Gunshot',.8,lambda t,n,l:(n*.8*math.exp(-t*100)+l*.85*math.exp(-t*17)+math.sin(t*2*math.pi*(100-30*t))*.32*math.exp(-t*28)))
write('Reload',2.1,lambda t,n,l:sum(pulse(t,s,.018)*(n*.6+math.sin(t*2*math.pi*1200)*.15) for s in [.04,.35,1.12,1.58,1.89]))
write('Door',.9,lambda t,n,l:pulse(t,0,.09)*l*.35+math.sin(t*2*math.pi*(240+70*math.sin(t*9)))*.12*math.sin(math.pi*min(t/.9,1)) + pulse(t,.74,.025)*n*.3)
write('Impact',.22,lambda t,n,l:pulse(t,0,.025)*n*.65+pulse(t,.03,.055)*l*.28)
write('Creak',.7,lambda t,n,l:math.sin(math.pi*t/.7)*(.18*math.sin(t*2*math.pi*(310+20*math.sin(t*22)))+.04*l))
print('Generated 8 original positional audio WAVs in',ROOT)
