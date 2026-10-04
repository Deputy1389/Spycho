import unreal as u
edit=u.MaterialEditingLibrary
palette={'wood':(.32,.19,.095),'woodDark':(.095,.055,.028),'carpet':(.19,.25,.24),'carpetDarker':(.08,.12,.13),'carpetWhite':(.65,.60,.49),'metal':(.24,.26,.27),'metalDark':(.025,.03,.035),'metalMedium':(.10,.12,.14),'plant':(.09,.22,.07),'lamp':(.78,.67,.48)}
materials={}
for name,color in palette.items():
    path='/Game/Materials/Furniture_'+name
    m=u.load_asset(path)
    if not m:
        m=u.AssetToolsHelpers.get_asset_tools().create_asset('Furniture_'+name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
        c=edit.create_material_expression(m,u.MaterialExpressionConstant3Vector);c.set_editor_property('constant',u.LinearColor(*color,1))
        edit.connect_material_property(c,'',u.MaterialProperty.MP_BASE_COLOR)
        rough=edit.create_material_expression(m,u.MaterialExpressionConstant);rough.set_editor_property('r',.8 if name.startswith('carpet') else .5)
        edit.connect_material_property(rough,'',u.MaterialProperty.MP_ROUGHNESS);edit.recompile_material(m)
        u.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
    materials[name]=m
# Furnishings use the house's checked-in CC0 grain/weave textures rather than
# uniform colored blocks. The low-poly meshes remain intentionally lightweight.
if u.load_asset('/Game/Materials/OakParquet'):
    materials['wood']=u.load_asset('/Game/Materials/OakParquet')
    materials['carpet']=u.load_asset('/Game/Materials/CarpetWeave')
for path in u.EditorAssetLibrary.list_assets('/Game/ThirdParty/Furniture',recursive=False):
    mesh=u.load_asset(path)
    if not isinstance(mesh,u.StaticMesh):continue
    for i,slot in enumerate(mesh.get_editor_property('static_materials')):
        name=str(slot.get_editor_property('material_slot_name'))
        if name in materials and mesh.get_material(i)!=materials[name]:mesh.set_material(i,materials[name])
    u.EditorAssetLibrary.save_loaded_asset(mesh,only_if_is_dirty=True)
weapon=u.load_asset('/Game/Weapons/Pistol/Materials/MI_Weapon_Pistol')
def tint(material,name,color):
    values=list(material.get_editor_property('vector_parameter_values'))
    for value in values:
        if str(value.get_editor_property('parameter_info').get_editor_property('name'))==name:
            value.set_editor_property('parameter_value',color)
            material.set_editor_property('vector_parameter_values',values)
            return
    edit.set_material_instance_vector_parameter_value(material,name,color)
tint(weapon,'Team.WeaponTint',u.LinearColor(.17,.19,.21,1))
u.EditorAssetLibrary.save_loaded_asset(weapon,only_if_is_dirty=False)
for name in ['MI_Manny_01_New','MI_Manny_02_New']:
    m=u.load_asset('/Game/Characters/Mannequins/Materials/Manny/'+name)
    tint(m,'Paint Tint',u.LinearColor(.19,.22,.21,1))
    tint(m,'LogoTint',u.LinearColor(.08,.08,.075,1))
    u.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
levels=u.get_editor_subsystem(u.LevelEditorSubsystem);levels.load_level('/Game/Maps/House')
actors=u.get_editor_subsystem(u.EditorActorSubsystem)
for a in actors.get_all_level_actors():
    if a.get_actor_label()=='Ceiling':a.set_editor_property('tags',[u.Name('CaptureCeiling')])
    if a.get_actor_label().endswith(' sign'):actors.destroy_actor(a)
for text,x,y,yaw in [('STUDY',-70,82,-90),('DEN',470,82,-90),('DINING',-70,-82,90),('BEDROOM',470,-82,90)]:
    a=actors.spawn_actor_from_class(u.TextRenderActor,u.Vector(x,y,216),u.Rotator(yaw=yaw));a.set_actor_label(text+' sign')
    c=a.get_component_by_class(u.TextRenderComponent);c.set_text(u.Text(text));c.set_world_size(12);c.set_horizontal_alignment(u.HorizTextAligment.EHTA_CENTER)
levels.save_current_level();u.log('SPYCHO_DUEL_ART_COMPLETE')
