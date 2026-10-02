import unreal as u
from pathlib import Path
for p in (Path(u.Paths.project_dir())/'Content/Audio/Source').glob('*.wav'):
    task=u.AssetImportTask();task.set_editor_property('filename',str(p));task.set_editor_property('destination_path','/Game/Audio')
    task.set_editor_property('replace_existing',True);task.set_editor_property('automated',True);task.set_editor_property('save',True)
    u.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    sound=u.load_asset('/Game/Audio/'+p.stem);assert sound,p
u.log('SPYCHO_RECORDED_AUDIO_COMPLETE')
