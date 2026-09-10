import unreal,json,os,math
base='/Game/Main/Animations/MainPlayerAnim/GunAnimation/'
a=unreal.load_asset(base+'AS_GunReload_Front_Player')
fire=unreal.load_asset(base+'AS_GunFire_Player')
assert a.get_editor_property('skeleton')==fire.get_editor_property('skeleton')
assert abs(a.get_play_length()-2.8)<1e-5
options=unreal.AnimPoseEvaluationOptions()
aim=unreal.AnimPoseExtensions.get_anim_pose_at_time(fire,0,options)
start=unreal.AnimPoseExtensions.get_anim_pose_at_time(fire,fire.get_play_length(),options)
names=a.get_editor_property('skeleton').get_reference_pose().get_bone_names()
def dist(a,b): return math.sqrt((a.x-b.x)**2+(a.y-b.y)**2+(a.z-b.z)**2)
max_pos=0; min_qdot=1; max_step=0; previous={}; samples=[]
previous_rot={}; max_angle=0; max_angle_bone=''
with open(os.path.join(os.path.dirname(__file__),'front_reload_paths.json')) as info: torso=json.load(info)
min_front=1000; min_elbow_front=1000; frame_samples=[]
for f in range(169):
    p=unreal.AnimPoseExtensions.get_anim_pose_at_time(a,f/60,options)
    joints={}
    for bn in ['Bip001-R-Hand','Bip001-R-Forearm','Bip001-L-Hand','Bip001-L-Forearm','Bip001-R-UpperArm','Bip001-L-UpperArm','Bip001-Spine2','Bip001-Pelvis']:
        tr=p.get_bone_pose(bn,unreal.AnimPoseSpaces.WORLD).translation
        xyz=(tr.x,tr.y,tr.z); joints[bn]=xyz
        depth=sum((xyz[i]-torso['center'][i])*torso['forward'][i] for i in range(3))
        if bn=='Bip001-R-Hand': min_front=min(min_front,depth)
        if bn=='Bip001-R-Forearm': min_elbow_front=min(min_elbow_front,depth)
    frame_samples.append({'frame':f,'joints':joints})
    for name in names:
        tr=p.get_bone_pose(name,unreal.AnimPoseSpaces.WORLD)
        qr=tr.rotation
        if str(name) in previous_rot:
            qa=previous_rot[str(name)]
            angle=math.degrees(2*math.acos(min(1,abs(qr.x*qa.x+qr.y*qa.y+qr.z*qa.z+qr.w*qa.w))))
            if angle>max_angle: max_angle=angle; max_angle_bone=str(name)+' frame '+str(f)
        previous_rot[str(name)]=qr
        assert all(math.isfinite(x) for x in [tr.translation.x,tr.translation.y,tr.translation.z,tr.rotation.x,tr.rotation.y,tr.rotation.z,tr.rotation.w])
        ref=(start if f==0 else aim).get_bone_pose(name,unreal.AnimPoseSpaces.WORLD)
        if str(name) in ['Root','Bip001-L-Foot','Bip001-R-Foot']:
            assert dist(tr.translation,ref.translation)<.02,(f,str(name),'drift')
        if f in [0,168]:
            max_pos=max(max_pos,dist(tr.translation,ref.translation))
            qa,qb=tr.rotation,ref.rotation
            min_qdot=min(min_qdot,abs(qa.x*qb.x+qa.y*qb.y+qa.z*qb.z+qa.w*qb.w))
        if str(name) in ['Bip001-R-Hand','Bip001-L-Hand']:
            if str(name) in previous: max_step=max(max_step,dist(tr.translation,previous[str(name)]))
            previous[str(name)]=tr.translation
    if f in [0,36,51,89,112,144]:
        samples.append({'frame':f,'right_hand':p.get_bone_pose('Bip001-R-Hand',unreal.AnimPoseSpaces.WORLD).translation.to_tuple()})
assert max_pos<.1, max_pos
assert min_qdot>.999, min_qdot
assert max_step<8,max_step
assert max_angle<30,(max_angle,max_angle_bone)
assert min_front>0,('right hand behind torso',min_front)
assert min_elbow_front>0,('right elbow behind torso',min_elbow_front)

report={'duration':a.get_play_length(),'bones_checked':len(names),'frames_checked':169,'max_endpoint_position_error_cm':max_pos,'min_endpoint_quaternion_dot':min_qdot,'max_hand_frame_step_cm':max_step,'max_bone_frame_angle_degrees':max_angle,'max_angle_bone':max_angle_bone,'samples':samples}
with open(os.path.join(os.path.dirname(__file__),'front_reload_verified.json'),'w') as f: json.dump(report,f,indent=2)
with open(os.path.join(os.path.dirname(__file__),'front_reload_evaluated.json'),'w') as f:
    json.dump({'min_right_hand_front_cm':min_front,'min_right_elbow_front_cm':min_elbow_front,'samples':frame_samples},f,indent=2)
print('RELOAD_VERIFIED',report)



