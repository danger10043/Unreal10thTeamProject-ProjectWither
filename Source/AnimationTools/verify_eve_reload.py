import unreal,json,os,math
base='/Game/ExternalAssets/Test_Pc_Eve/Eve_Body/Animations/'
a=unreal.load_asset(base+'AS_Eve_Pistol_Reload')
fire=unreal.load_asset(base+'AS_Eve_Pistol_Fire')
assert a.get_editor_property('skeleton')==fire.get_editor_property('skeleton')
assert abs(a.get_play_length()-2.4)<1e-5
options=unreal.AnimPoseEvaluationOptions()
aim=unreal.AnimPoseExtensions.get_anim_pose_at_time(fire,0,options)
names=a.get_editor_property('skeleton').get_reference_pose().get_bone_names()
def dist(a,b): return math.sqrt((a.x-b.x)**2+(a.y-b.y)**2+(a.z-b.z)**2)
max_pos=0; min_qdot=1; max_step=0; previous={}; samples=[]
for f in range(145):
    p=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,f/60,options)
    for name in names:
        tr=p.get_bone_pose(name,unreal.AnimPoseSpaces.WORLD)
        assert all(math.isfinite(x) for x in [tr.translation.x,tr.translation.y,tr.translation.z,tr.rotation.x,tr.rotation.y,tr.rotation.z,tr.rotation.w])
        ref=aim.get_bone_pose(name,unreal.AnimPoseSpaces.WORLD)
        if str(name) in ['Root','Bip001-L-Foot','Bip001-R-Foot']:
            assert dist(tr.translation,ref.translation)<.02,(f,str(name),'drift')
        if f in [0,144]:
            max_pos=max(max_pos,dist(tr.translation,ref.translation))
            qa,qb=tr.rotation,ref.rotation
            min_qdot=min(min_qdot,abs(qa.x*qb.x+qa.y*qb.y+qa.z*qb.z+qa.w*qb.w))
        if str(name) in ['Bip001-R-Hand','Bip001-L-Hand']:
            if str(name) in previous: max_step=max(max_step,dist(tr.translation,previous[str(name)]))
            previous[str(name)]=tr.translation
    if f in [0,36,51,89,112,144]:
        samples.append({'frame':f,'left_hand':p.get_bone_pose('Bip001-L-Hand',unreal.AnimPoseSpaces.WORLD).translation.to_tuple()})
assert max_pos<.1, max_pos
assert min_qdot>.999, min_qdot
assert max_step<8,max_step
assert samples[2]['left_hand'][2]<110,'Left hand did not reach magazine pickup position'
report={'duration':a.get_play_length(),'bones_checked':len(names),'frames_checked':145,'max_endpoint_position_error_cm':max_pos,'min_endpoint_quaternion_dot':min_qdot,'max_hand_frame_step_cm':max_step,'samples':samples}
with open(os.path.join(os.path.dirname(__file__),'reload_verified.json'),'w') as f: json.dump(report,f,indent=2)
print('RELOAD_VERIFIED',report)
