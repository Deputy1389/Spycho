"""Apply the user's dim residential hallway reference after the base house build.

Decorations have no collision: the paper walls, door seals and duel routes stay
authoritative. Source painting is retained with the other project source art.
"""
from pathlib import Path
import math
import unreal as u

assets=u.AssetToolsHelpers.get_asset_tools()
edit=u.MaterialEditingLibrary
levels=u.get_editor_subsystem(u.LevelEditorSubsystem)
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
assert levels.load_level('/Game/Maps/House')
source=Path(u.Paths.project_dir())/'Content/Art/Source/HallLandscape.png'
assert source.exists(),source
if not u.EditorAssetLibrary.does_asset_exist('/Game/Art/HallLandscape'):
    task=u.AssetImportTask();task.set_editor_property('filename',str(source))
    task.set_editor_property('destination_path','/Game/Art');task.set_editor_property('automated',True)
    task.set_editor_property('save',True);assets.import_asset_tasks([task])

def expr(mat,cls,**values):
    node=edit.create_material_expression(mat,cls)
    for key,value in values.items():node.set_editor_property(key,value)
    return node
def link(a,out,b,slot):assert edit.connect_material_expressions(a,out,b,'' if slot in ['Input','Coordinates'] else slot)
def output(node,pin,prop):assert edit.connect_material_property(node,'' if pin=='RGB' else pin,prop)
def scalar(mat,value):return expr(mat,u.MaterialExpressionConstant,r=value)
def color(mat,value):return expr(mat,u.MaterialExpressionConstant3Vector,constant=u.LinearColor(*value,1))
def material(name,tint,rough=.7,texture=None,size=150,normal_strength=.2,metal=0,emissive=False):
    mat=u.load_asset('/Game/Materials/'+name)
    # Native door construction roots its default material. Leave that simple
    # paint graph intact; the other authored graphs can be rebuilt for tuning.
    if mat and name=='ReferenceDoor':return mat
    if mat:edit.delete_all_material_expressions(mat)
    else:mat=assets.create_asset(name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
    mat.set_editor_property('tangent_space_normal',texture is None)
    base=color(mat,tint)
    if texture:
        def aligned(suffix,normal=False):
            tex=u.load_asset('/Game/ThirdParty/Textures/'+texture+'_1K-JPG_'+suffix);assert tex
            obj=expr(mat,u.MaterialExpressionTextureObject,texture=tex)
            fn=u.load_asset('/Engine/Functions/Engine_MaterialFunctions01/Texturing/'+('WorldAlignedNormal' if normal else 'WorldAlignedTexture'));assert fn
            call=expr(mat,u.MaterialExpressionMaterialFunctionCall,material_function=fn)
            link(obj,'',call,'TextureObject');link(color(mat,(size,size,size)),'',call,'TextureSize')
            return call
        sample=aligned('Color');mult=expr(mat,u.MaterialExpressionMultiply)
        link(sample,'XYZ Texture',mult,'A');link(base,'',mult,'B');base=mult
        roughtex=aligned('Roughness');rmul=expr(mat,u.MaterialExpressionMultiply)
        link(roughtex,'XYZ Texture',rmul,'A');link(scalar(mat,rough),'',rmul,'B')
        add=expr(mat,u.MaterialExpressionAdd);link(rmul,'',add,'A');link(scalar(mat,.16 if 'Floor' in name else .25),'',add,'B')
        output(add,'',u.MaterialProperty.MP_ROUGHNESS)
        normal=aligned('NormalDX',True)
        flat=expr(mat,u.MaterialExpressionVertexNormalWS)
        blend=expr(mat,u.MaterialExpressionLinearInterpolate)
        link(flat,'',blend,'A');link(normal,'XYZ Texture',blend,'B');link(scalar(mat,normal_strength),'',blend,'Alpha')
        norm=expr(mat,u.MaterialExpressionNormalize);link(blend,'',norm,'VectorInput');output(norm,'',u.MaterialProperty.MP_NORMAL)
    else:output(scalar(mat,rough),'',u.MaterialProperty.MP_ROUGHNESS)
    output(base,'',u.MaterialProperty.MP_EMISSIVE_COLOR if emissive else u.MaterialProperty.MP_BASE_COLOR)
    output(scalar(mat,metal),'',u.MaterialProperty.MP_METALLIC)
    edit.recompile_material(mat);u.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=False)
    return mat

mats={
 'plaster':material('ReferencePlaster',(.67,.74,.72),.55,'Plaster001',170,.10),
 'floor':material('ReferenceFloor',(.14,.10,.075),.38,'WoodFloor051',180,.06),
 'trim':material('ReferenceIvory',(.57,.59,.53),.4),
 'ceiling':material('ReferenceCeiling',(.40,.43,.39),.85),
 'door':material('ReferenceDoor',(.29,.31,.27),.36),
 'frame':material('ReferenceFrame',(.045,.028,.015),.30),
 'metal':material('ReferenceBronze',(.09,.075,.055),.27,metal=.75),
 'glass':material('ReferenceLampGlass',(1.8,1.9,1.65),.28,emissive=True),
 'window':material('ReferenceWindow',(.14,.21,.27),.5,emissive=True),
}
# Painting maps onto local Y/Z so frames on either hallway wall keep the scan
# upright. Vertex UV on the stock cube varies by face, so use explicit axes.
picture=u.load_asset('/Game/Materials/ReferenceLandscape')
if picture:edit.delete_all_material_expressions(picture)
else:picture=assets.create_asset('ReferenceLandscape','/Game/Materials',u.Material,u.MaterialFactoryNew())
if picture:
    pos=expr(picture,u.MaterialExpressionWorldPosition)
    local=expr(picture,u.MaterialExpressionTransformPosition,transform_source_type=u.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_WORLD,transform_type=u.MaterialPositionTransformSource.TRANSFORMPOSSOURCE_LOCAL)
    link(pos,'',local,'')
    uv=expr(picture,u.MaterialExpressionComponentMask,r=False,g=True,b=True,a=False);link(local,'',uv,'Input')
    scale=expr(picture,u.MaterialExpressionMultiply);link(uv,'',scale,'A');link(expr(picture,u.MaterialExpressionConstant2Vector,r=.01,g=-.01),'',scale,'B')
    offset=expr(picture,u.MaterialExpressionAdd);link(scale,'',offset,'A');link(expr(picture,u.MaterialExpressionConstant2Vector,r=.5,g=.5),'',offset,'B')
    sample=expr(picture,u.MaterialExpressionTextureSample,texture=u.load_asset('/Game/Art/HallLandscape'))
    link(offset,'',sample,'Coordinates');output(sample,'RGB',u.MaterialProperty.MP_BASE_COLOR)
    output(scalar(picture,.65),'',u.MaterialProperty.MP_ROUGHNESS)
    edit.recompile_material(picture);u.EditorAssetLibrary.save_loaded_asset(picture,only_if_is_dirty=False)
mats['picture']=picture

cube=u.load_asset('/Engine/BasicShapes/Cube');sphere=u.load_asset('/Engine/BasicShapes/Sphere')
def box(name,pos,size,mat,rot=0,mesh=None):
    a=actors.spawn_actor_from_class(u.StaticMeshActor,u.Vector(*pos),u.Rotator(yaw=rot));a.set_actor_label('Reference '+name)
    c=a.static_mesh_component;c.set_static_mesh(mesh or cube);c.set_material(0,mats[mat]);c.set_collision_profile_name('NoCollision')
    if name.startswith('sconce'):c.set_cast_shadow(False)
    a.set_actor_scale3d(u.Vector(*(v/100 for v in size)));return a
def light(name,pos,lumens=140,radius=350,temp=3600):
    a=actors.spawn_actor_from_class(u.PointLight,u.Vector(*pos));a.set_actor_label('Reference '+name)
    c=a.point_light_component;c.set_editor_property('mobility',u.ComponentMobility.MOVABLE)
    c.set_editor_property('intensity_units',u.LightUnits.LUMENS);c.set_editor_property('intensity',lumens)
    c.set_editor_property('attenuation_radius',radius);c.set_editor_property('source_radius',5)
    c.set_editor_property('use_temperature',True);c.set_editor_property('temperature',temp)
    return a
def sconce(x,y,facing,power=190):
    # facing is a unit direction from the wall into the room. A ribbed frosted
    # glass lantern with a dark metal backplate, brackets, cap and four rails.
    dx,dy=facing;angle=math.degrees(math.atan2(dy,dx))
    def part(name,d,z,size,mat):return box('sconce '+name,(x+dx*d,y+dy*d,z),size,mat,angle)
    part('backplate',1,184,(2,13,36),'metal')
    part('arm',8,172,(16,3,3),'metal')
    part('glass',15,189,(10,12,22),'glass')
    for z in [176,202]:part('cap',15,z,(14,17,3),'metal')
    for offset in [-7,7]:
        for depth in [10,20]:
            box('sconce rail',(x+dx*depth-dy*offset,y+dy*depth+dx*offset,189),(1.2,1.2,26),'metal')
    for z in [180,185,190,195,200]:part('glass rib',15,z,(10.5,12.5,.5),'trim')
    light('sconce pool',(x+dx*27,y+dy*27,190),power,260,4500)
def frame(x,y,z,width,height,facing):
    dx,dy=facing;angle=math.degrees(math.atan2(dy,dx))
    box('painting backing',(x,y,z),(3,width,height),'frame',angle)
    box('landscape painting',(x+dx*2,y+dy*2,z),(.4,width-10,height-10),'picture',angle)
    for sign in [-1,1]:
        for depth,edge,thick in [(3,0,4),(4.1,-3.5,1.3),(3.6,-5,1.2)]:
            mat='metal' if thick==1.3 else 'frame'
            box('frame vertical',(x+dx*depth-dy*sign*(width/2+edge),y+dy*depth+dx*sign*(width/2+edge),z),(1.4,thick,height+2*edge),mat,angle)
            box('frame horizontal',(x+dx*depth,y+dy*depth,z+sign*(height/2+edge)),(1.4,width+2*edge,thick),mat,angle)

old=list(actors.get_all_level_actors())
for a in old:
    name=a.get_actor_label()
    if name.startswith('Reference '):actors.destroy_actor(a);continue
    if isinstance(a,(u.PointLight,u.TextRenderActor)) or name.startswith(('Ceiling fixture','Print')):
        actors.destroy_actor(a);continue
    if isinstance(a,u.SpychoDoor):
        a.get_editor_property('panel').set_material(0,mats['door'])
        a.get_editor_property('inset').set_material(0,mats['door'])
        for detail in a.get_editor_property('panel_details'):detail.set_material(0,mats['door'])
        for key in ['handle_front','handle_back']:a.get_editor_property(key).set_material(0,mats['metal'])
        continue
    if not isinstance(a,u.StaticMeshActor):continue
    c=a.static_mesh_component
    if name in ['Switch plate','Switch']:
        p=a.get_actor_location()
        if abs(p.x+100)<1:p.x=-145
        elif abs(p.x-440)<1:p.x=395
        a.set_actor_location(p,False,True)
    if name=='Ceiling':c.set_material(0,mats['ceiling'])
    elif 'floor' in name.lower() and ('Lounge' in name or 'Hall' in name or a.get_actor_location().x<200):c.set_material(0,mats['floor'])
    elif any(key in name.lower() for key in ['skirting','crown','frame','header','mullion','sill','switch plate']):c.set_material(0,mats['trim'])
    elif name=='Night window':c.set_material(0,mats['window'])
    elif name.endswith('accent'):c.set_material(0,mats['plaster'])
    elif name in ['Exterior wall'] or any(key in name for key in [' / ',' lintel']):c.set_material(0,mats['plaster'])

# Layered trim on both sides of every partition, stopped at the real door gap.
def trim_run(axis,coord,lo,hi,opening=None):
    runs=[(lo,hi)] if opening is None else [(lo,opening-56),(opening+56,hi)]
    for a,b in runs:
        for side in [-1,1]:
            for d,z,h,w in [(2.5,8,16,2),(3.4,16.7,1.4,3),(4,1.5,3,4),(3,225,2,3),(4,229,4,4),(5,233,4,5),(6,237,4,7)]:
                if z>=225 and a!=runs[0][0]:continue
                aa,bb=(lo,hi) if z>=225 else (a,b)
                pos=((aa+bb)/2,coord+side*d,z) if axis=='y' else (coord+side*d,(aa+bb)/2,z)
                size=(bb-aa,w,h) if axis=='y' else (w,bb-aa,h)
                box('moulding',pos,size,'trim')
    if opening is not None:
        for side in [-1,1]:
            for direction in [-1,1]:
                p=(opening+direction*56,coord+side*3.2,106) if axis=='y' else (coord+side*3.2,opening+direction*56,106)
                box('door casing',p,(7,3,212) if axis=='y' else (3,7,212),'trim')
            p=(opening,coord+side*3.2,213) if axis=='y' else (coord+side*3.2,opening,213)
            box('door architrave',p,(120,4,8) if axis=='y' else (4,120,8),'trim')
trim_run('x',-300,-450,450,0)
for y in [-85,85]:
    trim_run('y',y,-300,200,-70);trim_run('y',y,200,700,470)
trim_run('x',200,85,450,270);trim_run('x',200,-450,-85,-270)
for y in [-447,447]:trim_run('y',y,-700,700)
for x in [-697,697]:trim_run('x',x,-450,450)

# Alternating wall lamps leave the middle of the hall in shadow. The far-end
# sash window and cool spill supply the bright focal point from the reference.
sconce(-190,82,(0,-1),120);sconce(225,-82,(0,1),55);sconce(630,82,(0,-1),75)
frame(100,79,155,75,58,(0,-1));frame(330,-79,156,65,49,(0,1));frame(-225,-79,156,38,48,(0,1))
frame(590,-79,153,36,45,(0,1))
box('end window outer',(696,0,157),(7,76,126),'trim')
box('end window glass',(691,0,157),(1,66,116),'window')
for y in [-32,0,32]:box('end sash upright',(689,y,157),(2,2,116),'trim')
for z in [101,137,175,213]:box('end sash rail',(689,0,z),(2,66,2.8),'trim')
box('end window sill',(686,0,94),(18,84,5),'trim')
end=actors.spawn_actor_from_class(u.RectLight,u.Vector(675,0,157),u.Rotator(yaw=180));end.set_actor_label('Reference end window spill')
rc=end.get_component_by_class(u.RectLightComponent)
for key,value in {'mobility':u.ComponentMobility.MOVABLE,'intensity_units':u.LightUnits.LUMENS,'intensity':65.0,'attenuation_radius':250.0,'source_width':60.0,'source_height':108.0,'use_temperature':True,'temperature':6200.0}.items():rc.set_editor_property(key,value)
# Furnished rooms use the same restrained old-house lighting language.
for x,y,facing,power in [(-697,-170,(1,0),170),(-697,250,(1,0),180),(60,447,(0,-1),185),(580,447,(0,-1),115),(135,-447,(0,1),185),(600,-447,(0,1),105)]:sconce(x,y,facing,power)
for x,y,z,power in [(-640,375,155,65),(590,400,98,50),(630,-180,104,50)]:light('table lamp',(x,y,z),power,280,3100)
for x,y in [(-80,410),(480,410),(-80,-410),(480,-410)]:light('moon spill',(x,y,170),45,260,6500)
for x,y,z,w,h,facing in [(-697,40,156,82,62,(1,0)),(-215,442,160,68,51,(0,-1)),(330,442,162,55,42,(0,-1)),(-215,-442,160,78,58,(0,1))]:frame(x,y,z,w,h,facing)

for a in list(actors.get_all_level_actors()):
    if isinstance(a,u.PostProcessVolume):actors.destroy_actor(a)
post=actors.spawn_actor_from_class(u.PostProcessVolume,u.Vector(0,0,0));post.set_actor_label('Reference atmosphere')
post.set_editor_property('unbound',True)
settings=post.get_editor_property('settings')
for key,value in {
 'override_auto_exposure_method':True,'auto_exposure_method':u.AutoExposureMethod.AEM_MANUAL,
 'override_auto_exposure_bias':True,'auto_exposure_bias':-.45,
 'override_auto_exposure_apply_physical_camera_exposure':True,'auto_exposure_apply_physical_camera_exposure':False,
 'override_color_saturation':True,'color_saturation':u.Vector4(.82,.82,.82,1),
 'override_vignette_intensity':True,'vignette_intensity':.23,
 'override_bloom_intensity':True,'bloom_intensity':.22,
 'override_film_grain_intensity':True,'film_grain_intensity':.035,
}.items():settings.set_editor_property(key,value)
post.set_editor_property('settings',settings)
levels.save_current_level()
u.log('SPYCHO_REFERENCE_STYLE_COMPLETE: cool plaster, dark reflective wood, layered trim, original paintings, wall sconces')
