"""Authored, single-floor faded Hollywood mansion, in Unreal centimetres.

This is the source of truth for geometry, bot routes, rooms and spawn pairs.
Run generate_mansion_layout.py after changing it, before compiling C++.
"""
BOUNDS=(-1200,-1200,1800,1200)
HEIGHT=330
ROOMS=[
 ('Drawing room',(-700,-450,-300,450),'wood',(-450,0)),
 ('Private hall',(-300,-85,700,85),'wood',(200,0)),
 ('Study',(-300,85,200,450),'wood',(-70,230)),
 ('Guest sitting room',(200,85,700,450),'carpet',(470,270)),
 ('Breakfast room',(-300,-450,200,-85),'wood',(-70,-270)),
 ('Bedroom',(200,-450,700,-85),'carpet',(470,-270)),
 ('West gallery',(-1200,-1200,-700,1200),'tile',(-950,0)),
 ('Library',(-700,450,200,1200),'wood',(-250,850)),
 ('North gallery',(200,450,1800,650),'wood',(900,550)),
 ('Ballroom',(200,650,1800,1200),'wood',(1000,900)),
 ('Grand foyer',(700,-450,1800,450),'tile',(1300,0)),
 ('Music room',(-700,-1200,200,-450),'wood',(-250,-850)),
 ('South gallery',(200,-650,1800,-450),'wood',(900,-550)),
 ('Screening salon',(200,-1200,1100,-650),'carpet',(650,-900)),
 ('Dressing suite',(1100,-1200,1800,-650),'carpet',(1450,-900)),
]
# axis, coordinate, span start/end, opening centre, width, door/arch
PARTITIONS=[
 ('x',-300,-450,450,0,100,'door'),
 ('y',85,-300,200,-70,100,'door'),('y',85,200,700,470,100,'door'),
 ('y',-85,-300,200,-70,100,'door'),('y',-85,200,700,470,100,'door'),
 ('x',200,85,450,270,100,'door'),('x',200,-450,-85,-270,100,'door'),
 ('x',-700,-450,450,0,100,'door'),('x',-700,450,1200,950,100,'door'),
 ('x',-700,-1200,-450,-950,100,'door'),
 ('y',450,-700,200,-70,100,'door'),('y',450,200,700,470,100,'door'),
 ('y',-450,-700,200,-70,100,'door'),('y',-450,200,700,470,100,'door'),
 ('x',700,85,450,270,100,'door'),('x',700,-450,-85,-270,100,'door'),
 ('x',700,-85,85,0,170,'arch'),
 ('x',200,450,650,550,100,'door'),('x',200,650,1200,900,100,'door'),
 ('y',650,200,1800,1300,250,'arch'),('y',450,700,1800,1300,250,'arch'),
 ('y',-450,700,1800,1300,250,'arch'),
 ('x',200,-650,-450,-550,100,'door'),('x',200,-1200,-650,-900,100,'door'),
 ('y',-650,200,1100,650,100,'door'),('y',-650,1100,1800,1450,100,'door'),
 ('x',1100,-1200,-650,-900,100,'door'),
]
# Keep the private-wing fixtures stable; existing combat tests exercise it.
NODES=[(-550,-250),(-450,0),(-300,0),(-100,0),(-70,85),(-70,230),
 (200,270),(470,270),(470,85),(470,0),(470,-85),(470,-270),(200,-270),
 (-70,-270),(-70,-85),(200,0)]
LINKS=[(0,1),(1,2),(2,3),(3,4),(4,5),(5,6),(6,7),(7,8),(8,9),
 (9,10),(10,11),(11,12),(12,13),(13,14),(14,3),(3,15),(15,9)]
ROOM_NODES=[1,15,5,7,13,11]
DOOR_NODES=[2,4,6,8,10,12,14]

def node(p):
    p=tuple(p)
    if p not in NODES:NODES.append(p)
    return NODES.index(p)
def link(a,b):
    if a!=b and (a,b) not in LINKS and (b,a) not in LINKS:LINKS.append((a,b))
def chain(*points):
    ids=[node(p) for p in points]
    for a,b in zip(ids,ids[1:]):link(a,b)
def room_at(p):
    for i,(_,bounds,_,_) in enumerate(ROOMS):
        x0,y0,x1,y1=bounds
        if x0<=p[0]<=x1 and y0<=p[1]<=y1:return i
    return -1

for _,_,_,centre in ROOMS[6:]:ROOM_NODES.append(node(centre))
chain((-450,0),(-700,0),(-950,0),(-950,950),(-700,950),(-250,950),(-250,850))
chain((-950,0),(-950,-950),(-700,-950),(-250,-950),(-250,-850))
chain((-70,230),(-70,450),(-70,850),(-250,850))
chain((-70,-270),(-70,-450),(-70,-850),(-250,-850))
chain((470,270),(470,450),(470,550),(900,550),(1300,550),(1300,450),(1300,0))
chain((470,-270),(470,-450),(470,-550),(900,-550),(1300,-550),(1300,-450),(1300,0))
chain((470,0),(700,0),(1000,0),(1300,0))
chain((470,270),(700,270),(1000,270),(1000,0))
chain((470,-270),(700,-270),(1000,-270),(1000,0))
chain((-250,850),(-70,850),(-70,550),(200,550),(470,550))
chain((-70,850),(-70,900),(200,900),(600,900),(1000,900),(1300,900),(1300,650),(1300,550))
chain((-250,-850),(-70,-850),(-70,-550),(200,-550),(470,-550))
chain((-70,-850),(-70,-900),(200,-900),(650,-900),(1100,-900),(1450,-900),(1450,-650),(1450,-550),(1300,-550))
chain((650,-900),(650,-650),(650,-550),(470,-550))
for axis,c,lo,hi,opening,width,kind in PARTITIONS:
    if kind=='door':DOOR_NODES.append(node((c,opening) if axis=='x' else (opening,c)))
# Production duels rotate through both mansion wings; regression harnesses
# retain their nearby paired positions to isolate gun/door assertions.
STARTS=[((-430,-240),(1450,250)),((-220,1000),(1450,-950)),
 ((800,800),(-400,-950)),((-1030,850),(1450,-250)),
 ((-200,200),(1250,1000)),((620,-830),(-1050,-850))]
LEGACY_STARTS=[((-560,-300),(530,300)),((-190,160),(530,-130)),((-230,-160),(340,360))]
