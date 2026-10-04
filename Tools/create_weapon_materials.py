"""Small original pistol materials, without rebuilding the player's house."""
import unreal as u
assets=u.AssetToolsHelpers.get_asset_tools()
for name,color in [('WeaponMetal',(.8,.9,1)),('WeaponPolymer',(.25,.28,.32)),('SightPaint',(6,6,5.2))]:
    m=u.load_asset('/Game/Materials/'+name)
    if not m:m=assets.create_asset(name,'/Game/Materials',u.Material,u.MaterialFactoryNew())
    # Restrained unlit values keep the small viewmodel readable at fixed exposure;
    # no colored glow, and its geometry still occludes the sights naturally.
    m.set_editor_property('shading_model',u.MaterialShadingModel.MSM_UNLIT)
    c=u.MaterialEditingLibrary.get_material_property_input_node(m,u.MaterialProperty.MP_EMISSIVE_COLOR)
    if not c:c=u.MaterialEditingLibrary.create_material_expression(m,u.MaterialExpressionConstant3Vector)
    c.set_editor_property('constant',u.LinearColor(*color,1))
    assert u.MaterialEditingLibrary.connect_material_property(c,'',u.MaterialProperty.MP_EMISSIVE_COLOR)
    u.MaterialEditingLibrary.recompile_material(m)
    assert u.EditorAssetLibrary.save_loaded_asset(m,only_if_is_dirty=False)
    u.log(name+' '+str(c.get_editor_property('constant')))
u.log('SPYCHO_WEAPON_MATERIALS_COMPLETE')
