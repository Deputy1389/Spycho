"""Build the playable faded-luxury mansion from mansion_layout.py.

The former intimate rooms become a private wing. New public rooms and two
gallery loops surround it. All ornament stays out of gameplay collision.
"""
from pathlib import Path
import math,sys
import unreal as u
HERE=Path(__file__).resolve().parent
sys.path.insert(0,str(HERE))
import mansion_layout as layout
style={'__name__':'mansion_style'}
exec((HERE/'style_reference_house.py').read_text(encoding='utf-8').split('old=list(actors.get_all_level_actors())')[0],style)
base={'__name__':'mansion_base'}
exec((HERE/'build_duel_house.py').read_text(encoding='utf-8').split("box('Ground'")[0],base)
actors=base['actors'];levels=base['levels'];cube=base['cube']
material=style['material'];sm=style['mats']
sm.update({
 'aged':material('MansionAgedPlaster',(.42,.44,.35),.7,'Plaster001',190,.18),
 'wine':material('MansionWine',(.15,.055,.047),.8,'Plaster001',140,.12),
 'velvet':material('MansionVelvet',(.055,.085,.075),.93,'Plaster001',25,.15),
 'stone':material('MansionStone',(.34,.32,.25),.48,'Plaster001',120,.12),
 'brass':material('MansionTarnishedBrass',(.23,.16,.065),.4,'Plaster001',45,.05,metal=.65),
 'wood':material('MansionWalnut',(.12,.065,.025),.48,'WoodFloor051',160,.07),
 'darktile':material('MansionDarkTile',(.055,.07,.06),.26),
 'mirror':material('MansionCloudedMirror',(.20,.22,.20),.18,metal=.85),
})
base['mats'].update({'Wall':sm['aged'],'Floor':sm['floor'],'Trim':sm['trim'],'Carpet':sm['velvet'],'Ceiling':sm['ceiling'],'Brass':sm['brass'],'Dark':sm['frame']})
box=base['box'];furnish=base['furnishing'];door=base['door']

def decor(name,p,size,mat='trim',yaw=0,mesh=None):
    a=style['box'](name,p,size,mat,yaw,mesh)
    a.set_actor_label('Mansion '+name)
    return a
def light(name,p,power=160,radius=450,temp=3300,shadow=False):
    a=style['light'](name,p,power*.55,radius,temp);a.set_actor_label('Mansion '+name)
    a.point_light_component.set_editor_property('cast_shadows',shadow)
    return a
def sconce(x,y,facing,power=120):
    # Warm, tarnished fixtures retain dark pockets between light pools.
    before=set(actors.get_all_level_actors());style['sconce'](x,y,facing,power*.55)
    for a in set(actors.get_all_level_actors())-before:
        a.set_actor_label('Mansion '+a.get_actor_label().removeprefix('Reference '))
        if isinstance(a,u.PointLight):
            a.point_light_component.set_editor_property('cast_shadows',False)
            a.point_light_component.set_editor_property('temperature',3400.0)
            a.point_light_component.set_editor_property('attenuation_radius',420.0)
def painting(x,y,z,w,h,facing):style['frame'](x,y,z,w,h,facing)
def line(axis,c,a,b,z,h,w,mat='trim',side=0):
    if b-a<.1:return
    decor('layered moulding',((a+b)/2,c+side,z) if axis=='y' else (c+side,(a+b)/2,z),(b-a,w,h) if axis=='y' else (w,b-a,h),mat)
def trim(axis,c,lo,hi,opening=None,width=100):
    runs=[(lo,hi)] if opening is None else [(lo,opening-width/2-5),(opening+width/2+5,hi)]
    for side in [-1,1]:
        for a,b in runs:
            line(axis,c,a,b,10,20,3,side=side*3)
            line(axis,c,a,b,21,2,4,side=side*4)
        for z,h,w in [(310,3,4),(316,6,6),(323,5,9)]:line(axis,c,lo,hi,z,h,w,side=side*4)

def partition(index,spec):
    axis,c,lo,hi,o,width,kind=spec
    for a,b in [(lo,o-width/2),(o+width/2,hi)]:
        if b-a>.1:
            box('Mansion partition '+str(index), (c,(a+b)/2,layout.HEIGHT/2) if axis=='x' else ((a+b)/2,c,layout.HEIGHT/2),(2.5,b-a,layout.HEIGHT) if axis=='x' else (b-a,2.5,layout.HEIGHT))
            for side in [-1,1]:
                pos=(c+side*2,(a+b)/2,47) if axis=='x' else ((a+b)/2,c+side*2,47)
                room=layout.room_at((pos[0]+side*10 if axis=='x' else pos[0],pos[1]+side*10 if axis=='y' else pos[1]))
                if room in [6,7,9,10,11,13,14]:
                    decor('walnut wainscot',pos,(2,b-a,94) if axis=='x' else (b-a,2,94),'wood')
                    line(axis,c,a,b,97,4,4,'brass',side*3)
                if room in [7,13]:
                    p=(pos[0],pos[1],200)
                    decor('faded wine wallpaper',p,(.5,b-a,200) if axis=='x' else (b-a,.5,200),'wine')
    top=210 if kind=='door' else 260
    box('Mansion lintel '+str(index),(c,o,(top+layout.HEIGHT)/2) if axis=='x' else (o,c,(top+layout.HEIGHT)/2),(2.5,width,layout.HEIGHT-top) if axis=='x' else (width,2.5,layout.HEIGHT-top))
    trim(axis,c,lo,hi,o,width)
    if kind=='door':
        a=door('Mansion door '+str(index),(c,o-50.5,0) if axis=='x' else (o+50.5,c,0),0 if axis=='x' else 90,95)
        a.get_editor_property('panel').set_material(0,sm['door']);a.get_editor_property('inset').set_material(0,sm['door'])
        for detail in a.get_editor_property('panel_details'):detail.set_material(0,sm['door'])
        for key in ['handle_front','handle_back']:a.get_editor_property(key).set_material(0,sm['brass'])
        for side in [-1,1]:
            for edge in [-1,1]:
                p=(c+side*3,o+edge*55,106) if axis=='x' else (o+edge*55,c+side*3,106)
                decor('door casing',p,(4,8,212) if axis=='x' else (8,4,212))
            p=(c+side*4,o,216) if axis=='x' else (o,c+side*4,216)
            decor('door pediment',p,(6,118,12) if axis=='x' else (118,6,12))
    else:
        # Shallow segmented arch moulding over the opening; real collision
        # remains the rectangular 2.6m-high clearance, never ornament.
        for side in [-1,1]:
            for edge in [-1,1]:
                p=(c+side*6,o+edge*(width/2+5),120) if axis=='x' else (o+edge*(width/2+5),c+side*6,120)
                decor('arch pilaster',p,(10,15,240) if axis=='x' else (15,10,240))
                p=(p[0],p[1],245);decor('pilaster capital',p,(18,25,10) if axis=='x' else (25,18,10))
            for j in range(11):
                angle=math.pi*j/10;offset=math.cos(angle)*width/2;z=245+math.sin(angle)*48
                p=(c+side*7,o+offset,z) if axis=='x' else (o+offset,c+side*7,z)
                a=decor('arch voussoir',p,(11,width/10+5,12) if axis=='x' else (width/10+5,11,12))
                a.set_actor_rotation(u.Rotator(pitch=math.degrees(math.atan2(48*math.cos(angle),-width/2*math.sin(angle)))) if axis=='y' else u.Rotator(roll=math.degrees(math.atan2(48*math.cos(angle),-width/2*math.sin(angle)))),False)

x0,y0,x1,y1=layout.BOUNDS
box('Mansion ground',((x0+x1)/2,0,-40),(x1-x0+300,y1-y0+300,60),'Concrete','Masonry')
for index,(name,(a,b,c,d),floor,_) in enumerate(layout.ROOMS):
    mat='Carpet' if floor=='carpet' else 'Floor' if floor=='wood' else 'Trim'
    a_floor=box('Mansion '+name+' floor',((a+c)/2,(b+d)/2,-10),(c-a,d-b,20),mat,{'carpet':'Carpet','wood':'Wood','tile':'Tile'}[floor])
    if floor=='tile':
        a_floor.static_mesh_component.set_material(0,sm['stone'])
        for x in range(a+50,c,100):
            for y in range(b+50,d,100):
                if ((x-a-50)//100+(y-b-50)//100)%2==0:decor('aged chequer stone',(x,y,.1),(98,98,.2),'darktile')
    ceiling=box('Mansion '+name+' ceiling',((a+c)/2,(b+d)/2,341),(c-a,d-b,22),'Ceiling','Masonry')
    ceiling.set_editor_property('tags',[u.Name('CaptureCeiling')])
for i,spec in enumerate(layout.PARTITIONS):partition(i,spec)
for x in [x0-10,x1+10]:box('Mansion exterior',(x,0,165),(20,y1-y0+40,330),'Wall','Masonry');trim('x',x,y0,y1)
for y in [y0-10,y1+10]:box('Mansion exterior',((x0+x1)/2,y,165),(x1-x0+40,20,330),'Wall','Masonry');trim('y',y,x0,x1)

def window(x,y,facing,w=140,h=170):
    dx,dy=facing;yaw=math.degrees(math.atan2(dy,dx));z=185
    def part(name,depth,zz,size,mat):return decor('sash '+name,(x+dx*depth,y+dy*depth,zz),size,mat,yaw)
    part('casing',3,z,(6,w+16,h+16),'trim');part('moon glass',7,z,(1,w,h),'window')
    for offset in [-w/2,0,w/2]:decor('sash mullion',(x+dx*9-dy*offset,y+dy*9+dx*offset,z),(2,3,h),'trim',yaw)
    for zz in [z-h/2,z,z+h/2]:part('rail',9,zz,(2,w,4),'trim')
    part('sill',14,z-h/2-9,(30,w+24,6),'trim')
    for offset in [-w/2-22,w/2+22]:decor('heavy faded curtain',(x+dx*14-dy*offset,y+dy*14+dx*offset,z-7),(15,30,h+35),'velvet',yaw)
    light('cool window spill',(x+dx*40,y+dy*40,z),80,460,6400)
for x in [-400,420,950,1550]:window(x,1198,(0,-1))
for x in [-400,1450]:window(x,-1198,(0,1))
for y in [-300,300]:window(1798,y,(-1,0),150,200)
for y in [-950,950]:window(-1198,y,(1,0))

def chandelier(x,y,diameter=170):
    decor('chandelier ceiling rose',(x,y,325),(48,48,5),'brass',mesh=u.load_asset('/Engine/BasicShapes/Cylinder'))
    decor('chandelier stem',(x,y,295),(3,3,55),'brass')
    for angle in range(0,360,45):
        rad=math.radians(angle);xx=x+math.cos(rad)*diameter/2;yy=y+math.sin(rad)*diameter/2
        decor('chandelier arm',((x+xx)/2,(y+yy)/2,274),(diameter/2,3,3),'brass',angle)
        decor('chandelier cup',(xx,yy,277),(14,14,4),'brass',mesh=u.load_asset('/Engine/BasicShapes/Cylinder'))
        decor('chandelier candle',(xx,yy,287),(4,4,17),'glass')
        decor('chandelier crystal',(xx,yy,262),(5,5,14),'mirror',mesh=u.load_asset('/Engine/BasicShapes/Sphere'))
    light('chandelier pool',(x,y,255),650,780,3100,True)
chandelier(1320,0,180);chandelier(800,920,155);chandelier(-280,850,135);chandelier(-300,-900,125)
for axis,c,a,b,o,w,kind in layout.PARTITIONS:
    for p in [a+80,b-80]:
        if abs(p-o)<w/2+70:continue
        for side in [-1,1]:
            # One pair on longer wall runs, fewer lights in the private wing.
            if (b-a)<450 and side<0:continue
            if axis=='x':sconce(c+side*3,p,(side,0),85)
            else:sconce(p,c+side*3,(0,side),85)
for y in [-900,-400,300,900]:sconce(-1197,y,(1,0),100)
for x in [250,850,1550]:sconce(x,647,(0,-1),65);sconce(x,-647,(0,1),65)

# Furniture and landmarks are deliberately beside, rather than across, routes.
furnish('loungeSofa',(-590,225),90,90);furnish('tableCoffee',(-485,240),40,90)
furnish('loungeChair',(-585,360),85,180);furnish('lampRoundFloor',(-640,375),165)
furnish('desk',(-180,370),76);furnish('chairDesk',(-150,320),95,180)
furnish('bookcaseOpen',(115,385),185);furnish('books',(115,385),15,z=90)
furnish('loungeChair',(610,350),90,180);furnish('cabinetBed',(580,400),55)
furnish('table',(-200,-330),75,90);furnish('chair',(-230,-425),90,180)
furnish('bedDouble',(330,-380),48,90);furnish('cabinetBed',(630,-180),60)
furnish('pottedPlant',(-650,-400),140)
for y in [620,800,1120]:furnish('bookcaseOpen',(-620,y),225,90);furnish('books',(-615,y),18,90,z=120)
furnish('desk',(-430,1140),83);furnish('chairDesk',(-430,1100),100,180)
furnish('loungeChair',(100,1100),96,180);furnish('loungeChair',(-450,600),96,90)
for x,y,yaw in [(1730,275,180),(1730,-300,180),(480,1090,90),(1450,1100,-90),(-1100,500,90),(-1100,-500,90)]:furnish('loungeSofa',(x,y),90,yaw)
for x,y in [(1650,290),(1630,-290),(1400,1080)]:furnish('tableCoffee',(x,y),44)
for x,y in [(1760,380),(1760,-380),(1740,1070),(-1130,-1100)]:furnish('pottedPlant',(x,y),155)
for x,y in [(360,-1100),(580,-1100),(800,-1100),(360,-1010),(800,-1010)]:furnish('loungeChair',(x,y),100,0)
decor('silent silver screen',(630,-1192,170),(330,4,150),'stone')
decor('screen velvet valance',(630,-1188,260),(390,12,22),'velvet')
for x in [425,835]:decor('screen drape',(x,-1180,155),(45,18,230),'velvet')
furnish('desk',(1660,-1090),82);furnish('chairDesk',(1650,-1020),94)
decor('clouded dressing mirror',(1650,-1190,175),(170,5,120),'brass')
decor('mirror glass',(1650,-1186,175),(150,2,100),'mirror')
furnish('cabinetBed',(1210,-1100),85)

# Hero grand piano: closed walnut lid, brass rim and real keyboard silhouette.
px,py=-480,-1110
decor('piano body',(px,py,86),(260,130,34),'wood')
decor('piano lid',(px,py,107),(265,135,5),'frame')
decor('piano rim',(px,py-66,104),(265,3,6),'brass')
for dx,dy in [(-105,-45),(105,-45),(75,45)]:decor('piano carved leg',(px+dx,py+dy,40),(15,15,80),'wood')
decor('piano keybed',(px,py+73,85),(240,22,9),'frame')
for i in range(32):
    xx=px-116+i*7.3;decor('ivory piano key',(xx,py+73,91),(7,21,3),'trim')
    if i%7 in [0,1,3,4,5]:decor('ebony piano key',(xx+3.5,py+68,94),(3.5,12,4),'frame')
furnish('chair',(px,py+90),65,180)
def fireplace(x,y,facing):
    dx,dy=facing;angle=math.degrees(math.atan2(dy,dx))
    def part(name,d,z,size,mat):decor('fireplace '+name,(x+dx*d,y+dy*d,z),size,mat,angle)
    part('cold hearth',12,50,(24,130,90),'frame')
    for offset in [-78,78]:decor('fireplace pilaster',(x+dx*25-dy*offset,y+dy*25+dx*offset,70),(42,22,140),'stone',angle)
    part('mantel',22,148,(55,205,18),'stone');part('tarnished inset',10,175,(6,185,20),'brass')
    painting(x+dx*5,y+dy*5,240,165,100,facing)
fireplace(-690,750,(1,0));fireplace(1795,930,(-1,0))
for p in [(-1168,0,185,115,90,(1,0)),(1200,1190,190,165,110,(0,-1)),(1150,-640,180,95,75,(0,1)),(1720,435,180,115,85,(0,-1)),(-295,310,170,70,60,(1,0))]:painting(*p)

# Scuffed rugs, coffered ceilings and a marble arrival medallion give scale
# without adding confusing movable cover or collision over the routes.
for x,y,w,h in [(-250,850,540,420),(-300,-820,520,360),(1000,900,980,270),(-500,220,220,200)]:decor('worn rug',(x,y,.3),(w,h,.5),'velvet')
for x in [830,1150,1470,1780]:decor('foyer ceiling coffer',(x,0,325),(10,850,9),'wood')
for y in [-395,0,395]:decor('foyer ceiling coffer',(1250,y,325),(1090,10,9),'wood')
decor('foyer medallion',(1300,0,.3),(220,220,.5),'brass',mesh=u.load_asset('/Engine/BasicShapes/Cylinder'))
decor('foyer medallion inset',(1300,0,.6),(206,206,.3),'darktile',mesh=u.load_asset('/Engine/BasicShapes/Cylinder'))
for a in list(actors.get_all_level_actors()):
    if isinstance(a,u.PostProcessVolume):actors.destroy_actor(a)
post=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector());post.set_actor_label('Mansion faded atmosphere');post.set_editor_property('unbound',True)
settings=post.get_editor_property('settings')
for key,value in {'override_auto_exposure_method':True,'auto_exposure_method':u.AutoExposureMethod.AEM_MANUAL,'override_auto_exposure_apply_physical_camera_exposure':True,'auto_exposure_apply_physical_camera_exposure':False,'override_color_saturation':True,'color_saturation':u.Vector4(.74,.77,.71,1),'override_vignette_intensity':True,'vignette_intensity':.24,'override_bloom_intensity':True,'bloom_intensity':.22,'override_film_grain_intensity':True,'film_grain_intensity':.04}.items():settings.set_editor_property(key,value)
post.set_editor_property('settings',settings)
actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-430,-240,90),u.Rotator(yaw=0))
levels.save_current_level();u.EditorAssetLibrary.save_directory('/Game/Materials',only_if_is_dirty=True,recursive=True)
u.log('SPYCHO_MANSION_COMPLETE: 30 x 24 metres, 15 rooms/areas, 23 doors, 4 open arches, faded Hollywood luxury')
