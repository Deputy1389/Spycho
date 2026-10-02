"""Five rooms and a short hall, based on the user's overhead layout reference."""
import unreal as u
import math
tools=u.AssetToolsHelpers.get_asset_tools()
edit=u.MaterialEditingLibrary
def solid(name,color,rough=.75,emissive=False):
    path='/Game/Materials/'+name;m=u.load_asset(path)
    if m:return m
    m=tools.create_asset(name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
    c=edit.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.set_editor_property('constant',u.LinearColor(*color,1))
    edit.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR if emissive else u.MaterialProperty.MP_BASE_COLOR)
    r=edit.create_material_expression(m,u.MaterialExpressionConstant);r.set_editor_property('r',rough);edit.connect_material_property(r,'',u.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False);return m
def textured(name,asset,tint,size):
    path='/Game/Materials/'+name;m=u.load_asset(path)
    if m:return m
    m=tools.create_asset(name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
    fn=u.load_asset('/Engine/Functions/Engine_MaterialFunctions01/Texturing/WorldAlignedTexture');assert fn
    def aligned(suffix):
        tex=u.load_asset('/Game/ThirdParty/Textures/'+asset+'_1K-JPG_'+suffix);assert tex,asset+suffix
        obj=edit.create_material_expression(m,u.MaterialExpressionTextureObject);obj.set_editor_property('texture',tex)
        call=edit.create_material_expression(m,u.MaterialExpressionMaterialFunctionCall);call.set_editor_property('material_function',fn)
        tile=edit.create_material_expression(m,u.MaterialExpressionConstant3Vector);tile.set_editor_property('constant',u.LinearColor(size,size,size,1))
        assert edit.connect_material_expressions(obj,'',call,'TextureObject')
        assert edit.connect_material_expressions(tile,'',call,'TextureSize')
        return call
    color=aligned('Color');c=edit.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.set_editor_property('constant',u.LinearColor(*tint,1))
    mult=edit.create_material_expression(m,u.MaterialExpressionMultiply);assert edit.connect_material_expressions(color,'XYZ Texture',mult,'A');assert edit.connect_material_expressions(c,'',mult,'B')
    assert edit.connect_material_property(mult,'',u.MaterialProperty.MP_BASE_COLOR)
    rough=aligned('Roughness');assert edit.connect_material_property(rough,'XYZ Texture',u.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(m);u.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False);return m
mats={
 'Wall':textured('PaperPlaster','Plaster001',(.78,.77,.71),200),
 'Floor':textured('Parquet','WoodFloor051',(.7,.62,.52),180),
 'Trim':solid('PaintedTrim',(.14,.17,.18)),
 'Carpet':solid('WovenCarpet',(.15,.18,.17)),
 'Ceiling':solid('CeilingPaint',(.62,.64,.62)),
 'Lamp':solid('LampShade',(5,4.5,3.7),emissive=True),
 'Dark':solid('Dark',(.022,.024,.026)),
 'Concrete':solid('Concrete',(.19,.20,.21)),
}
surfaces={name:u.load_asset('/Game/Surfaces/'+name) for name in ['Drywall','Wood','Carpet','Tile','Masonry','Furniture']}
paper=surfaces['Drywall'];paper.set_editor_property('resistance_per_cm',.08);paper.set_editor_property('entry_cost',.2);paper.set_editor_property('max_thickness',60)
u.EditorAssetLibrary.save_loaded_asset(paper,only_if_is_dirty=False)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem);actors=u.get_editor_subsystem(u.EditorActorSubsystem)
assert levels.load_level('/Game/Maps/House')
for actor in actors.get_all_level_actors():
    if not isinstance(actor,(u.WorldSettings,u.LevelScriptActor)):
        actors.destroy_actor(actor)
cube=u.load_asset('/Engine/BasicShapes/Cube')
def box(name,pos,size,mat='Wall',surface='Drywall',rot=0,collision=True):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*pos),u.Rotator(yaw=rot));a.set_actor_label(name)
    c=a.static_mesh_component;c.set_static_mesh(cube);c.set_material(0,mats[mat]);c.set_phys_material_override(surfaces[surface]);c.set_collision_profile_name('BlockAll' if collision else 'NoCollision')
    a.set_actor_scale3d(u.Vector(*(v/100 for v in size)));return a
def door(name,pos,yaw,angle):
    a=actors.spawn_actor_from_class(u.SpychoDoor,u.Vector(*pos),u.Rotator(yaw=yaw));a.set_actor_label(name)
    panel=a.get_editor_property('panel');panel.set_material(0,mats['Trim']);panel.set_phys_material_override(surfaces['Drywall']);a.set_editor_property('open_angle',angle)
    return a
def partition(name,axis,coord,lo,hi,opening,angle):
    # 2.5 cm paper-like opaque partitions; ordinary 1 m openings.
    pieces=[(lo,opening-50),(opening+50,hi)]
    for a,b in pieces:
        pos=(coord,(a+b)/2,120) if axis=='x' else ((a+b)/2,coord,120)
        size=(2.5,b-a,240) if axis=='x' else (b-a,2.5,240)
        box(name,pos,size)
        trimsize=(4,b-a,9) if axis=='x' else (b-a,4,9)
        box(name+' skirting',(pos[0],pos[1],4.5),trimsize,'Trim','Drywall')
    pos=(coord,opening,225) if axis=='x' else (opening,coord,225)
    size=(2.5,100,30) if axis=='x' else (100,2.5,30)
    box(name+' lintel',pos,size)
    for sign in [-1,1]:
        trimpos=(coord,opening+sign*52,104) if axis=='x' else (opening+sign*52,coord,104)
        trimsize=(5,4,208) if axis=='x' else (4,5,208)
        box(name+' frame',trimpos,trimsize,'Trim','Drywall')
    header=(coord,opening,210) if axis=='x' else (opening,coord,210)
    box(name+' header',header,(6,108,6) if axis=='x' else (108,6,6),'Trim','Drywall')
    door(name+' door',(coord,opening-50.5,0) if axis=='x' else (opening+50.5,coord,0),0 if axis=='x' else 90,angle)
def furnishing(name,xy,height,yaw=0,z=0):
    mesh=u.load_asset('/Game/ThirdParty/Furniture/'+name);assert mesh,name
    bounds=mesh.get_bounds();scale=height/(2*bounds.box_extent.z)
    theta=math.radians(yaw);ox=bounds.origin.x*scale;oy=bounds.origin.y*scale
    loc=(xy[0]-(ox*math.cos(theta)-oy*math.sin(theta)),xy[1]-(ox*math.sin(theta)+oy*math.cos(theta)),z-(bounds.origin.z-bounds.box_extent.z)*scale)
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*loc),u.Rotator(yaw=yaw));a.set_actor_label(name)
    c=a.static_mesh_component;c.set_static_mesh(mesh);c.set_phys_material_override(surfaces['Furniture']);c.set_collision_profile_name('BlockAll')
    a.set_actor_scale3d(u.Vector(scale,scale,scale));return a
box('Ground',(0,0,-35),(2300,1800,40),'Concrete','Masonry')
box('Lounge floor',(-500,0,-10),(400,900,20),'Floor','Wood')
box('Hall floor',(200,0,-10),(1000,170,20),'Floor','Wood')
for side in [-1,1]:
    for x in [-50,450]:box('Room floor',(x,side*267.5,-10),(500,365,20),'Carpet' if x==450 else 'Floor','Carpet' if x==450 else 'Wood')
partition('Lounge / hall','x',-300,-450,450,0,95)
partition('Study / hall','y',85,-300,200,-70,-95)
partition('Den / hall','y',85,200,700,470,-95)
partition('Dining / hall','y',-85,-300,200,-70,95)
partition('Bedroom / hall','y',-85,200,700,470,95)
partition('Study / den','x',200,85,450,270,-95)
partition('Dining / bedroom','x',200,-450,-85,-270,95)
for x in [-710,710]:box('Exterior wall',(x,0,120),(20,920,240),'Wall','Masonry')
for y in [-460,460]:box('Exterior wall',(0,y,120),(1440,20,240),'Wall','Masonry')
box('Ceiling',(0,0,251),(1440,940,22),'Ceiling','Masonry')
# Furniture stays clear of the route graph and doorway approaches.
furnishing('loungeSofa',(-600,220),90,90);furnishing('tableCoffee',(-485,240),40,90)
furnishing('loungeChair',(-585,360),85,180);furnishing('lampRoundFloor',(-640,375),165)
furnishing('desk',(-170,375),76);furnishing('chairDesk',(-150,310),100,180)
furnishing('computerScreen',(-170,380),43,z=76);furnishing('computerKeyboard',(-165,355),4,z=76)
furnishing('bookcaseOpen',(120,385),185);furnishing('books',(115,382),15,z=90)
furnishing('loungeChair',(620,345),90,180);furnishing('cabinetBed',(590,400),55)
furnishing('lampSquareTable',(590,400),45,z=55);furnishing('pottedPlant',(650,130),140)
furnishing('table',(-150,-380),75)
for x,y,yaw in [(-240,-375,90),(-60,-375,-90),(-150,-425,180)]:furnishing('chair',(x,y),90,yaw)
furnishing('bedDouble',(600,-320),64,90);furnishing('cabinetBed',(630,-180),60)
furnishing('lampSquareTable',(630,-180),45,z=60);furnishing('pottedPlant',(-650,-400),140)
def lamp(x,y,lumens=900):
    a=actors.spawn_actor_from_class(u.PointLight,u.Vector(x,y,216));c=a.point_light_component
    c.set_editor_property('mobility',u.ComponentMobility.MOVABLE);c.set_editor_property('intensity_units',u.LightUnits.LUMENS)
    c.set_editor_property('intensity',lumens);c.set_editor_property('attenuation_radius',520);c.set_editor_property('source_radius',18)
    c.set_editor_property('light_color',u.Color(255,235,213,255))
    box('Ceiling fixture',(x,y,238),(34,34,3),'Lamp','Masonry',collision=False)
for x,y in [(-500,-170),(-500,240),(-80,0),(470,0),(-70,300),(470,300),(-70,-300),(470,-300)]:lamp(x,y,700 if y==0 else 950)
# Window panels add reflected cool daylight; partitions remain opaque.
for x in [-80,480]:
    for y in [-448,448]:
        box('Window frame',(x,y,155),(135,4,95),'Trim','Masonry')
        box('Frosted window',(x,y+(-3 if y>0 else 3),155),(125,2,85),'Lamp','Masonry',collision=False)
start=actors.spawn_actor_from_class(u.PlayerStart,u.Vector(-560,-300,90),u.Rotator(yaw=49))
levels.save_current_level();u.EditorAssetLibrary.save_directory('/Game/Materials',only_if_is_dirty=False,recursive=True)
u.log('SPYCHO_DUEL_HOUSE_COMPLETE: lounge, study, den, dining, bedroom and short hall; 2.5 cm partitions')
