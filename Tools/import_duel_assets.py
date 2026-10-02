"""Import selected CC0 furnishings/textures and inspect bundled Unreal assets."""
import unreal as u
from pathlib import Path
src=Path(u.Paths.project_dir())/'Content/ThirdParty/Source'
tools=u.AssetToolsHelpers.get_asset_tools()
for p in src.glob('*'):
    if p.suffix.lower() not in ['.jpg','.png','.fbx']:continue
    target='/Game/ThirdParty/Furniture' if p.suffix.lower()=='.fbx' else '/Game/ThirdParty/Textures'
    if u.EditorAssetLibrary.does_asset_exist(target+'/'+p.stem):continue
    task=u.AssetImportTask();task.set_editor_property('filename',str(p));task.set_editor_property('destination_path',target)
    task.set_editor_property('automated',True);task.set_editor_property('save',True)
    if p.suffix.lower()=='.fbx':
        options=u.FbxImportUI();options.set_editor_property('import_mesh',True);options.set_editor_property('import_as_skeletal',False)
        options.set_editor_property('mesh_type_to_import',u.FBXImportType.FBXIT_STATIC_MESH)
        options.set_editor_property('automated_import_should_detect_type',False)
        options.set_editor_property('import_materials',True);options.set_editor_property('import_textures',True)
        options.static_mesh_import_data.set_editor_property('combine_meshes',True)
        options.static_mesh_import_data.set_editor_property('auto_generate_collision',True)
        task.set_editor_property('options',options)
    tools.import_asset_tasks([task])
    u.log('DUEL_IMPORT '+p.name+' '+str(task.get_editor_property('imported_object_paths')))
for path in ['/Game/Weapons/Pistol/Meshes/SM_Pistol','/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple','/Game/Characters/Mannequins/Anims/Pistol/MF_Pistol_Idle_ADS']:
    a=u.load_asset(path);assert a,path
    u.log('DUEL_REFERENCE '+path+' '+str(a))
    if isinstance(a,u.StaticMesh):u.log('DUEL_BOUNDS '+str(a.get_bounds()))
for a in u.EditorAssetLibrary.list_assets('/Game/ThirdParty/Furniture',recursive=False):
    m=u.load_asset(a)
    if isinstance(m,u.StaticMesh):u.log('DUEL_FURNITURE '+a+' '+str(m.get_bounds()))
u.EditorAssetLibrary.save_directory('/Game/ThirdParty',only_if_is_dirty=False,recursive=True)
u.log('SPYCHO_DUEL_IMPORT_COMPLETE')
