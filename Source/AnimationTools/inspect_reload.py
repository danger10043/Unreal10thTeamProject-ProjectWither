import unreal,json,os
mesh=unreal.load_asset('/Game/Main/Blueprint/Player/Meshes/Player/SK_Body_MainPlayer')
sk=mesh.get_editor_property('skeleton')
p=sk.get_reference_pose()
data={'mesh':mesh.get_path_name(),'skeleton':sk.get_path_name(),'bones':{str(n):{'parent':str(mesh.get_bone_parent(n)),'pos':p.get_ref_bone_pose(n,unreal.AnimPoseSpaces.WORLD).translation.to_tuple()} for n in p.get_bone_names()}}
a=unreal.load_asset('/Game/Main/Animations/MainPlayerAnim/GunAnimation/AS_GunFire_Player')
data['fire_skeleton']=a.get_editor_property('skeleton').get_path_name()
data['duration']=a.get_play_length()
with open(os.path.join(os.path.dirname(__file__),'reload_info.json'),'w') as f: json.dump(data,f,indent=2)
print('RELOAD_INFO',data['mesh'],data['skeleton'],len(data['bones']))
