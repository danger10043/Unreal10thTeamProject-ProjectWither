"""Generate an in-place pistol reload on the Eve skeleton, using the existing shot's aim pose.
Run in Unreal 5.8 Python commandlet. Does not overwrite existing assets.
Magazine and slide are hand gestures; weapon animation/gameplay must be wired separately.
"""
import unreal, math, json, os

DEST='/Game/ExternalAssets/Test_Pc_Eve/Eve_Body/Animations'
NAME='AS_Eve_Pistol_Reload'
mesh=unreal.load_asset('/Game/Main/Blueprint/Player/Meshes/Player/SK_Body_MainPlayer')
source=unreal.load_asset(DEST+'/AS_Eve_Pistol_Fire')
sk=mesh.get_editor_property('skeleton')
assert source.get_editor_property('skeleton')==sk
pose=unreal.AnimPoseExtensions.get_anim_pose_at_time(source,0,unreal.AnimPoseEvaluationOptions())
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
right=[(0,rp),(.16,rp),(.42,(28,9,132)),(1.48,(28,9,132)),(1.72,(29,8,134)),(1.90,(30,8,134)),(2.08,(30,8,135)),(2.4,rp)]
left=[(0,lp),(.16,lp),(.42,(28,5,125)),(.60,(28,5,117)),(.85,(7,-20,100)),(1.04,(7,-20,100)),(1.30,(28,5,119)),(1.48,(28,5,127)),(1.58,(28,5,127)),(1.72,(35,5,142)),(1.86,(28,5,142)),(1.98,(25,-6,137)),(2.16,(32,-7,135)),(2.4,lp)]
weights=[(0,(0,)),(.16,(0,)),(.42,(1,)),(1.98,(1,)),(2.4,(0,))]
assert not unreal.EditorAssetLibrary.does_asset_exist(DEST+'/'+NAME),'Asset exists; refusing overwrite'
factory=unreal.AnimSequenceFactory()
factory.set_editor_property('target_skeleton',sk)
factory.set_editor_property('preview_skeletal_mesh',mesh)
anim=unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME,DEST,unreal.AnimSequence,factory)
c=anim.get_editor_property('controller')
c.open_bracket('Eve pistol reload',False)
c.set_frame_rate(unreal.FrameRate(60,1),False)
c.set_number_of_frames(unreal.FrameNumber(144),False)
tracks={n:[[],[],[]] for n in names}
samples=[]
for frame in range(145):
    t=frame/60; w=curve(weights,t)[0]
    rotations={}; positions={}; elbows={}; targets={'R':curve(right,t),'L':curve(left,t)}
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
                d=length(sub(target,shoulder)); assert abs(a-b)+.01<d<a+b-.01,(frame,side,d,a+b)
                direction=norm(sub(target,shoulder)); along=(a*a-b*b+d*d)/(2*d)
                base_dir=norm(sub(v(world[prefix+'Hand'].translation),v(world[n].translation)))
                base_elbow=sub(v(world[prefix+'Forearm'].translation),v(world[n].translation))
                base_pole=norm(sub(base_elbow,mul(base_dir,dot(base_elbow,base_dir))))
                pole=norm(mix(base_pole,(0,.65 if side=='R' else -.65,-1),w))
                bend=norm(sub(pole,mul(direction,dot(pole,direction))))
                elbow=add(shoulder,add(mul(direction,along),mul(bend,math.sqrt(max(0,a*a-along*along)))))
                elbows[side]=elbow
                delta=between(sub(v(world[prefix+'Forearm'].translation),v(world[n].translation)),sub(elbow,shoulder))
            elif n.endswith('-Forearm'):
                delta=between(sub(v(world[prefix+'Hand'].translation),v(world[n].translation)),sub(target,positions[n]))
            else:
                # Cant the pistol inward; support hand keeps a loose grasp for the magazine.
                delta=qm(axis((1,0,0),(-22 if side=='R' else 15)*w),axis((0,1,0),-12*w))
            rotations[n]=qm(delta,q(world[n]))
        lq=norm(qm(inv(pq),rotations[n]))
        tracks[n][0].append(local[n].translation)
        tracks[n][1].append(unreal.Quat(*lq))
        tracks[n][2].append(local[n].scale3d)
    if frame in [0,25,36,51,78,89,103,112,144]:
        samples.append({'frame':frame,'hands':{side:positions['Bip001-'+side+'-Hand'] for side in ['R','L']}})
for n,(p,r,s) in tracks.items():
    assert c.add_bone_curve(n,False),n
    assert c.set_bone_track_keys(n,p,r,s,False),n
c.close_bracket(False)
unreal.AnimationLibrary.add_animation_notify_track(anim,'ReloadTiming')
for name,time in [('MagazineOut',.60),('MagazineIn',1.48),('SlidePull',1.86),('ReloadComplete',2.16)]:
    unreal.AnimationLibrary.add_animation_sync_marker(anim,name,time,'ReloadTiming')
unreal.EditorAssetLibrary.set_metadata_tag(anim,'Description','2.4s/60fps two-handed pistol reload. Timing sync markers only; no gameplay callbacks, weapon or magazine animation. Starts and ends in AS_Eve_Pistol_Fire aim pose.')
assert unreal.EditorAssetLibrary.save_loaded_asset(anim)
with open(os.path.join(os.path.dirname(__file__),'reload_created.json'),'w') as f:
    json.dump({'asset':anim.get_path_name(),'duration':anim.get_play_length(),'tracks':len(tracks),'samples':samples},f,indent=2)
print('RELOAD_CREATED',anim.get_path_name(),anim.get_play_length(),len(tracks))
