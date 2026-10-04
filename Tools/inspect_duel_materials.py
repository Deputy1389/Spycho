import unreal as u
for name in ['PaperPlaster','Parquet','PaintedTrim','WovenCarpet','CeilingPaint','LampShade']:
    m=u.load_asset('/Game/Materials/'+name)
    u.log('DUEL_MAT '+name+' nodes '+str(u.MaterialEditingLibrary.get_num_material_expressions(m))+' base '+str(u.MaterialEditingLibrary.get_material_property_input_node(m,u.MaterialProperty.MP_BASE_COLOR)))
for path in ['/Game/Weapons/Pistol/Materials/MI_Weapon_Pistol','/Game/Characters/Mannequins/Materials/Manny/MI_Manny_01_New']:
    m=u.load_asset(path)
    u.log('DUEL_PARAMS '+path+' '+str(u.MaterialEditingLibrary.get_vector_parameter_names(m)))
    for name in u.MaterialEditingLibrary.get_vector_parameter_names(m):u.log('DUEL_VALUE '+str(name)+' '+str(u.MaterialEditingLibrary.get_material_instance_vector_parameter_value(m,name)))
levels=u.get_editor_subsystem(u.LevelEditorSubsystem);levels.load_level('/Game/Maps/House')
for a in u.get_editor_subsystem(u.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label() in ['Hall floor','Lounge / hall','Ceiling']:
        u.log('DUEL_ASSIGNED '+a.get_actor_label()+' '+str(a.static_mesh_component.get_material(0)))
