"""Generate an in-place pistol reload on the Eve skeleton, using the existing shot's aim pose.
Run in Unreal 5.8 Python commandlet. Does not overwrite existing assets.
Magazine and slide are hand gestures; weapon animation/gameplay must be wired separately.
"""
import unreal, math, json, os

DEST='/Game/Main/Animations/MainPlayerAnim/GunAnimation'
NAME='AS_GunReload_Player'
mesh=unreal.load_asset('/Game/Main/Blueprint/Player/Meshes/Player/SK_Body_MainPlayer')
source=unreal.load_asset(DEST+'/AS_GunFire_Player')
sk=mesh.get_editor_property('skeleton')
assert source.get_editor_property('skeleton')==sk
pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(source,source.get_play_length(),unreal.AnimPoseEvaluationOptions())
names=[str(n) for n in sk.get_reference_pose().get_bone_names()]
local={n:pose.get_bone_pose(n) for n in names}
world={n:pose.get_bone_pose(n,unreal.AnimPoseSpaces.WORLD) for n in names}
parents={n:str(mesh.get_bone_parent(n)) for n in names}
def v(x): return (x.x,x.y,x.z)
def add(a,b): return tuple(x+y for x,y in zip(a,b))
def sub(a,b): return tuple(x-y for x,y in zip(a,b))
def mul(a,s): return tuple(x*s for x in a)
def dot(a,b): return sum(x*y for x,y in zip(a,b))
def length(a): return math.sqrt(dot(a,a))
def norm(a): return mul(a,1/length(a))
def cross(a,b): return (a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0])
def q(t): return (t.rotation.x,t.rotation.y,t.rotation.z,t.rotation.w)
def qm(a,b): return (*add(add(mul(b[:3],a[3]),mul(a[:3],b[3])),cross(a[:3],b[:3])),a[3]*b[3]-dot(a[:3],b[:3]))
def inv(a): return (-a[0],-a[1],-a[2],a[3])
def rotate(a,b): return qm(qm(a,(*b,0)),inv(a))[:3]
def axis(a,d): return (*mul(norm(a),math.sin(math.radians(d)/2)),math.cos(math.radians(d)/2))
def between(a,b):
    a,b=norm(a),norm(b)
    r=(*cross(a,b),1+dot(a,b))
    return norm(r)
def mix(a,b,s): return add(mul(a,1-s),mul(b,s))
def smooth(s): return s*s*(3-2*s)
def curve(keys,t):
    for (t0,a),(t1,b) in zip(keys,keys[1:]):
        if t<=t1: return mix(a,b,smooth(max(0,min(1,(t-t0)/(t1-t0)))))
    return keys[-1][1]
R='Bip001-R-Hand'; L='Bip001-L-Hand'
rp=v(world[R].translation); lp=v(world[L].translation)
left=[(0,lp),(.16,lp),(.48,(25,2,132)),(1.48,(25,2,132)),(1.72,(26,2,134)),(1.90,(27,2,134)),(2.08,(28,3,136)),(2.4,lp)]
right=[(0,rp),(.16,rp),(.32,(0,-28,114)),(.48,(23,-3,125)),(.64,(23,-3,117)),(.88,(-18,-26,99)),(1.06,(-18,-26,99)),(1.30,(23,-3,119)),(1.48,(23,-3,127)),(1.58,(23,-3,127)),(1.72,(26,-2,141)),(1.86,(23,-2,141)),(1.98,(16,-12,129)),(2.16,(-10,-24,106)),(2.4,rp)]
weights=[(0,(0,)),(.16,(0,)),(.42,(1,)),(1.98,(1,)),(2.4,(0,))]
# Rebuild this task's generated sequence in place.
factory=unreal.AnimSequenceFactory()
factory.set_editor_property('target_skeleton',sk)
factory.set_editor_property('preview_skeletal_mesh',mesh)
anim=unreal.load_asset(DEST+'/'+NAME) if unreal.EditorAssetLibrary.does_asset_exist(DEST+'/'+NAME) else unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME,DEST,unreal.AnimSequence,factory)
c=anim.get_editor_property('controller')
c.open_bracket('Eve pistol reload',False)
c.set_frame_rate(unreal.FrameRate(60,1),False)
c.set_number_of_frames(unreal.FrameNumber(144),False)
tracks={n:[[],[],[]] for n in names}
samples=[]
for frame in range(145):
    t=frame/60; w=curve(weights,t)[0]
    rotations={}; positions={}; elbows={}; armframes={}; targets={'R':curve(right,t),'L':curve(left,t)}
    for n in names:
        parent=parents[n]; pq=rotations.get(parent,(0,0,0,1)); pp=positions.get(parent,(0,0,0))
        positions[n]=add(pp,rotate(pq,v(local[n].translation)))
        rotations[n]=qm(pq,q(local[n]))
        if n in ['Bip001-Spine1','Bip001-Spine2']:
            rotations[n]=qm(axis((0,0,1),-2*w),rotations[n])
        if n=='Bip001-Head': rotations[n]=qm(axis((0,1,0),8*w),rotations[n])
        if n.endswith(('-UpperArm','-Forearm','-Hand')):
            side='R' if '-R-' in n else 'L'; prefix='Bip001-'+side+'-'
            target=targets[side]
            if n.endswith('-UpperArm'):
                shoulder=positions[n]
                a=length(v(local[prefix+'Forearm'].translation)); b=length(v(local[prefix+'Hand'].translation))
                d=length(sub(target,shoulder)); assert abs(a-b)<d<a+b+.02,(frame,side,d,a+b)
                direction=norm(sub(target,shoulder)); along=(a*a-b*b+d*d)/(2*d)
                base_dir=norm(sub(v(world[prefix+'Hand'].translation),v(world[n].translation)))
                base_elbow=sub(v(world[prefix+'Forearm'].translation),v(world[n].translation))
                base_pole=norm(sub(base_elbow,mul(base_dir,dot(base_elbow,base_dir))))
                pole=norm(mix(base_pole,(0,-.65 if side=='R' else .65,-1),w))
                bend=norm(sub(pole,mul(direction,dot(pole,direction))))
                swing=between(base_dir,direction)
                swung_pole=rotate(swing,base_pole)
                twist=math.degrees(math.atan2(dot(direction,cross(swung_pole,bend)),dot(swung_pole,bend)))
                armframes[side]=qm(axis(direction,twist),swing)
                elbow=add(shoulder,add(mul(direction,along),mul(bend,math.sqrt(max(0,a*a-along*along)))))
                elbows[side]=elbow
                af=armframes[side]
                delta=between(sub(v(world[prefix+'Forearm'].translation),v(world[n].translation)),sub(elbow,shoulder))
            elif n.endswith('-Forearm'):
                af=armframes[side]
                delta=between(sub(v(world[prefix+'Hand'].translation),v(world[n].translation)),sub(target,positions[n]))
            else:
                # Cant the pistol inward; support hand keeps a loose grasp for the magazine.
                delta=qm(axis((1,0,0),(22 if side=='L' else -15)*w),axis((0,1,0),-12*w))
            rotations[n]=qm(delta,q(world[n]))
        lq=norm(qm(inv(pq),rotations[n]))
        tracks[n][0].append(local[n].translation)
        tracks[n][1].append(unreal.Quat(*lq))
        tracks[n][2].append(local[n].scale3d)
    if frame in [0,25,36,51,78,89,103,112,144]:
        samples.append({'frame':frame,'hands':{side:positions['Bip001-'+side+'-Hand'] for side in ['R','L']}})
# Smooth the arm rotations across IK direction changes while preserving endpoints.
for n in names:
    if not n.endswith(('-UpperArm','-Forearm','-Hand')): continue
    keys=[(x.x,x.y,x.z,x.w) for x in tracks[n][1]]
    for iteration in range(12):
        updated=list(keys)
        for i in range(1,144):
            prev=keys[i-1] if dot(keys[i-1],keys[i])>=0 else mul(keys[i-1],-1)
            nxt=keys[i+1] if dot(keys[i+1],keys[i])>=0 else mul(keys[i+1],-1)
            updated[i]=norm(add(mul(keys[i],.5),mul(add(prev,nxt),.25)))
        keys=updated
    tracks[n][1]=[unreal.Quat(*x) for x in keys]
# Exact source endpoints, with a smooth correction over the recovery phase.
start_pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(source,0,unreal.AnimPoseEvaluationOptions())
for n in names:
    end=start_pose.get_bone_pose(n)
    for frame in range(130,145):
        weight=smooth((frame-130)/14)
        old=tracks[n][1][frame]
        delta=qm(inv(q(local[n])),q(end))
        if delta[3]<0: delta=mul(delta,-1)
        correction=norm(mix((0,0,0,1),delta,weight))
        tracks[n][1][frame]=unreal.Quat(*norm(qm((old.x,old.y,old.z,old.w),correction)))
        tracks[n][0][frame]=unreal.Vector(*mix(v(local[n].translation),v(end.translation),weight))
        tracks[n][2][frame]=unreal.Vector(*mix(v(local[n].scale3d),v(end.scale3d),weight))
    for frame,exact in [(0,local[n]),(144,end)]:
        tracks[n][0][frame]=exact.translation
        tracks[n][1][frame]=exact.rotation
        tracks[n][2][frame]=exact.scale3d
for n,(p,r,s) in tracks.items():
    c.add_bone_curve(n,False)
    assert c.set_bone_track_keys(n,p,r,s,False),n
c.close_bracket(False)
unreal.AnimationLibrary.remove_all_animation_sync_markers(anim)
unreal.AnimationLibrary.remove_all_animation_notify_tracks(anim)
unreal.AnimationLibrary.add_animation_notify_track(anim,'ReloadTiming')
for name,time in [('MagazineOut',.64),('MagazineIn',1.48),('SlidePull',1.86),('ReloadComplete',2.16)]:
    unreal.AnimationLibrary.add_animation_sync_marker(anim,name,time,'ReloadTiming')
unreal.EditorAssetLibrary.set_metadata_tag(anim,'Description','2.4s/60fps left-hand pistol, right-hand magazine/slide manipulation. Starts at AS_GunFire_Player final pose, ends at its first pose. Timing sync markers only; no gameplay callbacks or weapon animation.')
assert unreal.EditorAssetLibrary.save_loaded_asset(anim)
with open(os.path.join(os.path.dirname(__file__),'gun_reload_created.json'),'w') as f:
    json.dump({'asset':anim.get_path_name(),'duration':anim.get_play_length(),'tracks':len(tracks),'samples':samples},f,indent=2)
print('RELOAD_CREATED',anim.get_path_name(),anim.get_play_length(),len(tracks))



