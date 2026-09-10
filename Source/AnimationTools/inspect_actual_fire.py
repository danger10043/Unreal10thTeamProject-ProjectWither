import unreal,json,os
a=unreal.load_asset('/Game/Main/Animations/MainPlayerAnim/GunAnimation/AS_GunFire_Player')
out={'duration':a.get_play_length(),'poses':[]}
ref=a.get_editor_property('skeleton').get_reference_pose()
for n in ['Bip001-Pelvis','Bip001-Spine2','Bip001-L-Clavicle','Bip001-R-Clavicle']:
 print('TORSO_REF',n,ref.get_bone_pose(n,unreal.AnimPoseSpaces.WORLD))
for t in [0,a.get_play_length()]:
 p=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,t,unreal.AnimPoseEvaluationOptions())
 for n in ['Bip001-Pelvis','Bip001-Spine2','Bip001-L-Clavicle','Bip001-R-Clavicle']:
  print('TORSO_ACT',n,p.get_bone_pose(n,unreal.AnimPoseSpaces.WORLD))
 out['poses'].append({str(n):p.get_bone_pose(n,unreal.AnimPoseSpaces.WORLD).to_tuple() for n in p.get_bone_names() if str(n) in ['Root','Bip001-Pelvis','Bip001-R-UpperArm','Bip001-L-UpperArm','Bip001-R-Hand','Bip001-L-Hand','Bip001-R-Foot','Bip001-L-Foot']})
with open(os.path.join(os.path.dirname(__file__),'actual_fire.json'),'w') as f: json.dump(out,f,default=lambda x:x.to_tuple(),indent=2)
