"""Process selected CC0 recordings. Optional regeneration needs numpy + soundfile."""
from pathlib import Path
import numpy as np
import soundfile as sf
ROOT=Path(__file__).resolve().parents[1]/'Content/Audio'
SRC=ROOT/'Recordings';OUT=ROOT/'Source';RATE=44100
def read(name):
    x,rate=sf.read(SRC/name,always_2d=True);x=x.mean(axis=1)
    if rate!=RATE:x=np.interp(np.arange(round(len(x)*RATE/rate))*rate/RATE,np.arange(len(x)),x)
    return x-x.mean()
def trim(x,threshold=.012):
    active=np.flatnonzero(np.abs(x)>threshold)
    if len(active):x=x[max(0,active[0]-220):min(len(x),active[-1]+1500)]
    return x
def write(name,x,peak=.65):
    x=x/(max(np.max(np.abs(x)),.00001))*peak
    x=np.pad(x,(64,0));fade=min(220,len(x)//4);x[-fade:]*=np.linspace(1,0,fade)
    assert np.max(np.abs(x))<.98 and np.isfinite(x).all()
    sf.write(OUT/(name+'.wav'),x,RATE,subtype='PCM_16')
    print(name,round(len(x)/RATE,3),'sec','peak',round(float(np.max(np.abs(x))),3))
for name,material in [('StepWood','wood'),('StepCarpet','carpet'),('StepTile','concrete')]:
    for i in range(4):write(name+('' if i==0 else '_'+str(i)),trim(read('footstep_'+material+'_'+str(i).zfill(3)+'.ogg')))
# Use one recorded pistol transient, preserving its short room/outdoor tail.
x=read('pistol.wav');window=441
energy=np.convolve(x*x,np.ones(window)/window,mode='same')
onsets=np.flatnonzero(energy>max(energy)*.30);start=max(0,int(onsets[0])-500)
write('Gunshot',x[start:start+int(.65*RATE)],.85)
x=trim(read('reload.wav'))
# Fit recorded magazine/slide handling to the authoritative 2.1 s reload.
x=np.interp(np.linspace(0,len(x)-1,int(1.95*RATE)),np.arange(len(x)),x)
write('Reload',np.pad(x,(0,int(.15*RATE))),.62)
def layer(parts,duration):
    out=np.zeros(int(duration*RATE))
    for name,t,gain in parts:
        clip=trim(read(name));n=min(len(clip),len(out)-int(t*RATE));out[int(t*RATE):int(t*RATE)+n]+=clip[:n]*gain
    return out
write('Door',layer([('impactWood_light_000.ogg',0,1),('impactMetal_light_000.ogg',.58,.4)],.95),.55)
write('Impact',trim(read('impactPlank_medium_000.ogg')),.5)
write('Creak',layer([('impactWood_light_001.ogg',0,.5),('impactWood_light_000.ogg',.23,.35)],.75),.3)
write('Coin',trim(read('impactMetal_light_000.ogg')),.65)
