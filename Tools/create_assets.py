"""Run in UnrealEditor with -ExecutePythonScript. Rebuilds the original graybox."""
import unreal as u
from pathlib import Path

assets=u.AssetToolsHelpers.get_asset_tools()
def material(name,color):
    path='/Game/Materials/'+name
    existing=u.load_asset(path)
    if existing: return existing
    m=assets.create_asset(name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
    c=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionConstant3Vector)
    c.set_editor_property('constant',u.LinearColor(*color,1))
    u.MaterialEditingLibrary.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
    rough=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionConstant)
    rough.set_editor_property('r',.85)
    u.MaterialEditingLibrary.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS)
    u.MaterialEditingLibrary.recompile_material(m)
    u.EditorAssetLibrary.save_loaded_asset(m)
    return m

colors={'Drywall':(.33,.32,.27),'Wood':(.16,.105,.06),'Carpet':(.08,.105,.10),'Tile':(.24,.25,.24),'Masonry':(.19,.20,.21),'Dark':(.022,.024,.026),'BulletHole':(.005,.003,.002),'Trim':(.30,.26,.19)}
mats={n:material(n,c) for n,c in colors.items()}
surfaces={}
for n,typ,res,entry,thick,penetrable,gain in [('Drywall',1,.45,3,60,True,1),('Wood',2,1.8,7,80,True,1),('Carpet',3,2,4,25,True,.6),('Tile',4,5,15,25,True,1.3),('Masonry',5,20,100,200,False,1),('Furniture',2,2.2,9,80,True,1)]:
    s=u.load_asset('/Game/Surfaces/'+n)
    if not s: s=assets.create_asset(n,'/Game/Surfaces',u.SpychoSurface,u.PhysicalMaterialFactoryNew())
    for key,val in [('resistance_per_cm',res),('entry_cost',entry),('max_thickness',thick),('penetrable',penetrable),('footstep_gain',gain),('surface_type',getattr(u.PhysicalSurface,'SURFACE_TYPE'+str(typ)))]:
        s.set_editor_property(key,val)
    u.EditorAssetLibrary.save_loaded_asset(s); surfaces[n]=s

root=Path(u.Paths.project_dir())/'Content/Audio/Source'
for source in root.glob('*.wav'):
    task=u.AssetImportTask(); task.set_editor_property('filename',str(source));task.set_editor_property('destination_path','/Game/Audio');task.set_editor_property('automated',True);task.set_editor_property('replace_existing',True);task.set_editor_property('save',True)
    assets.import_asset_tasks([task])

levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
levels.new_level('/Game/Maps/House')
cube=u.load_asset('/Engine/BasicShapes/Cube')
def box(name,pos,size,mat='Drywall',surface=None,rot=0):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*pos),u.Rotator(0,rot,0))
    a.set_actor_label(name)
    c=a.static_mesh_component;c.set_static_mesh(cube);c.set_material(0,mats[mat]);c.set_phys_material_override(surfaces[surface or (mat if mat in surfaces else 'Furniture')]);c.set_collision_profile_name('BlockAll')
    a.set_actor_scale3d(u.Vector(*(v/100 for v in size)))
    return a
def door(name,pos,yaw=0):
    a=actors.spawn_actor_from_class(u.SpychoDoor,u.Vector(*pos),u.Rotator(0,yaw,0));a.set_actor_label(name)
    panel=a.get_editor_property('panel');panel.set_material(0,mats['Wood']);panel.set_phys_material_override(surfaces['Wood'])
    return a
def xwall(name,x,lo,hi,opening,mat='Drywall',thickness=12):
    if opening is None:box(name,(x,(lo+hi)/2,135),(thickness,hi-lo,270),mat);return
    d=opening
    box(name+' south',(x,(lo+d-50)/2,135),(thickness,d-50-lo,270),mat)
    box(name+' north',(x,(d+50+hi)/2,135),(thickness,hi-d-50,270),mat)
    box(name+' lintel',(x,d,239),(thickness,100,62),mat)
    door(name+' door',(x,d-45,0))
def ywall(name,y,lo,hi,opening,mat='Drywall',thickness=12):
    if opening is None:box(name,((lo+hi)/2,y,135),(hi-lo,thickness,270),mat);return
    d=opening
    box(name+' west',((lo+d-50)/2,y,135),(d-50-lo,thickness,270),mat)
    box(name+' east',((d+50+hi)/2,y,135),(hi-d-50,thickness,270),mat)
    box(name+' lintel',(d,y,239),(100,thickness,62),mat)
    door(name+' door',(d+45,y,0),90)

# A 12 x 14 m house. 2.4 m corridor, ordinary 1 m doorways.
box('Outside ground',(0,0,-32),(2600,3000,30),'Masonry')
box('Hall wood',(0,0,-10),(240,1400,20),'Wood')
for side in [-1,1]:
    for i,(lo,hi) in enumerate([(-700,-250),(-250,200),(200,700)]):
        finish=['Wood','Tile','Carpet'][(i+(side==1))%3]
        box('Room floor', (side*360,(lo+hi)/2,-10),(480,hi-lo,20),finish)
        xwall('Hall partition',side*120,lo,hi,[-475,-25,450][i])
    for y in [-250,200]: ywall('Connecting partition',y,(-600 if side==-1 else 120),(-120 if side==-1 else 600),side*360)
xwall('West exterior',-612,-712,712,None,'Masonry',24)
xwall('East exterior',612,-712,712,None,'Masonry',24)
ywall('North exterior',712,-624,624,None,'Masonry',24)
ywall('Entrance',-712,-624,624,0,'Masonry',24)
box('Ceiling',(0,0,285),(1248,1448,30),'Drywall','Masonry')
# Furniture keeps the room boundaries readable without creating an arena.
for n,pos,size,mat in [('Living sofa',(-465,-510,45),(90,190,90),'Carpet'),('Living table',(-310,-510,32),(80,100,64),'Wood'),('Office desk',(490,-560,40),(150,65,80),'Wood'),('Kitchen counter',(-545,-30,45),(65,240,90),'Tile'),('Bathroom cabinet',(535,-100,40),(60,90,80),'Wood'),('Bedroom bed',(-470,530,27),(150,190,54),'Carpet'),('Study shelf',(548,470,90),(45,240,180),'Wood')]:
    box(n,pos,size,mat,'Furniture')
for x in [-580,580]:box('Baseboard',(x,0,6),(5,1400,12),'Trim','Wood')
for x,y in [(0,-800),(0,-540),(0,200),(-360,-470),(360,-470),(-360,0),(360,0),(-360,450),(360,450)]:
    light=actors.spawn_actor_from_class(u.PointLight,u.Vector(x,y,238))
    lc=light.point_light_component
    lc.set_editor_property('mobility',u.ComponentMobility.MOVABLE)
    lc.set_editor_property('intensity_units',u.LightUnits.LUMENS)
    lc.set_editor_property('intensity',1500 if x else 1000)
    lc.set_editor_property('attenuation_radius',600)
    lc.set_editor_property('light_color',u.Color(255,222,178,255))
    lc.set_editor_property('source_radius',8)
    box('Ceiling lamp',(x,y,265),(25,25,8),'Trim','Wood')
moon=actors.spawn_actor_from_class(u.DirectionalLight,u.Vector(0,0,900),u.Rotator(-45,-30,0))
moon_component=moon.get_component_by_class(u.DirectionalLightComponent)
moon_component.set_editor_property('mobility',u.ComponentMobility.MOVABLE)
moon_component.set_editor_property('intensity',.5)
moon_component.set_editor_property('light_color',u.Color(155,179,220,255))
start=actors.spawn_actor_from_class(u.PlayerStart,u.Vector(0,-820,90),u.Rotator(0,90,0))
levels.save_current_level()
u.EditorAssetLibrary.save_directory('/Game',only_if_is_dirty=False,recursive=True)
u.log('SPYCHO_ASSETS_COMPLETE: House and all original material/audio assets saved')
