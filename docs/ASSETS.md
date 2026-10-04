# Asset sources and licenses

## Kenney Furniture Kit — CC0

Source: https://kenney.nl/assets/furniture-kit

17 selected models furnish the lounge, study, den, dining room and bedroom: sofas/chairs, desks/tables, bed, shelves, lamps, plants and computer props. Raw FBX files and the supplied license are retained in `Content/ThirdParty/Source`. Imported Unreal assets are in `Content/ThirdParty/Furniture`.

Downloaded archive SHA-256: `e67652d0932cee41683f74711c03d3e192a2af9979ef8e6b237711f5482d46b0`.

## ambientCG — CC0

- Plaster001: https://ambientcg.com/view?id=Plaster001
- WoodFloor051: https://ambientcg.com/view?id=WoodFloor051
- License and raw redistribution terms: https://docs.ambientcg.com/license/

Selected 1K JPG color/roughness/normal source maps are retained in `Content/ThirdParty/Source`; Unreal textures in `Content/ThirdParty/Textures`. The current wall/floor materials use color, roughness and normal relief with world-space texture mapping. The dark floor uses a lower roughness range for varnished reflections.

Archive SHA-256: Plaster001 `944b4831016e42ace4a89422e4e7190912ca2f7fed6f561354c48ec7bb54d3a4`; WoodFloor051 `3f493484eab1ec5e1c466b90e515003b286fc6d7f84ff8ff6485900bfc26cef5`.

## Epic Games Unreal template Examples

Pistol mesh/material/texture assets and Manny mesh, physics, materials and selected pistol animation assets come from the locally installed Unreal Engine 5.8 First Person template. They are in `Content/Weapons` and `Content/Characters/Mannequins`. These assets are licensed under the Unreal Engine EULA, rather than CC0. Its Examples provision permits distribution in source or object form: https://www.unrealengine.com/eula/unreal (section 5(b)).

## Project-authored assets

House layout, surface/room materials, physical materials, game code and asset/audio tooling were authored for this project. Screenshots are actual rendered game captures. One original image-generated aged landscape painting is included as wall art. Source: Content/Art/Source/HallLandscape.png; imported texture: /Game/Art/HallLandscape. Created with the built-in image-generation tool for this project; no third-party painting or artist signature is used. Tools/style_reference_house.py builds its material and layered frame geometry. ARTWORK.md in the deliverables records the full prompt. All sconces, trim, sash-window details and raised door panels are project-authored geometry.

## Recorded audio — CC0

- Kenney Impact Sounds: https://kenney.nl/assets/impact-sounds — selected wood/carpet/concrete footsteps (four each), wood/plank and metal impacts. Used for footsteps, door/latch, impacts, the coin landing and subtle building foley.
- “Gunshots” by kurt: https://opengameart.org/content/gunshots — one transient trimmed from the supplied 22 Pistol.wav recording.
- “Gun reload sounds” by SpringySpringo: https://opengameart.org/content/gun-reload-sounds — airsoft magazine/slide handling fitted to the 2.1-second reload.

These source pages label the recordings CC0. Original selected recordings are in Content/Audio/Recordings; processed mono 44.1 kHz 16-bit WAVs in Content/Audio/Source. Tools/prepare_recorded_audio.py documents trimming, normalization, layering and duration fitting. Gunfire remains distinct from quiet foley, with a playback multiplier that keeps the sampled transient below full scale.
