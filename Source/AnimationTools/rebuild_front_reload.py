"""Generate an in-place pistol reload on the Eve skeleton, using the existing shot's aim pose.
Run in Unreal 5.8 Python commandlet. Does not overwrite existing assets.
Magazine and slide are hand gestures; weapon animation/gameplay must be wired separately.
"""
import unreal, math, json, os

DEST='/Game/Main/Animations/MainPlayerAnim/GunAnimation'
NAME='AS_GunReload_Front_Player'
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

def frame_quat(direction,normal):
    x=norm(direction); y=norm(cross(normal,x)); z=cross(x,y)
    m=[[x[i],y[i],z[i]] for i in range(3)]
    trace=m[0][0]+m[1][1]+m[2][2]
    if trace>0:
        s=math.sqrt(trace+1)*2
        return norm(((m[2][1]-m[1][2])/s,(m[0][2]-m[2][0])/s,(m[1][0]-m[0][1])/s,s/4))
    i=max(range(3),key=lambda i:m[i][i]); j=(i+1)%3; k=(i+2)%3
    s=math.sqrt(1+m[i][i]-m[j][j]-m[k][k])*2
    r=[0,0,0,(m[k][j]-m[j][k])/s]; r[i]=s/4
    r[j]=(m[j][i]+m[i][j])/s; r[k]=(m[k][i]+m[i][k])/s
    return norm(r)

chest='Bip001-Spine2'
reference=sk.get_reference_pose().get_bone_pose(chest,unreal.AnimPoseSpaces.WORLD)
torso_delta=qm(q(world[chest]),inv(q(reference)))
# Imported Eve body's anterior axis is -X in the reference mesh, verified in preview.
forward=rotate(torso_delta,(-1,0,0)); forward=norm((forward[0],forward[1],0))
right_axis=(forward[1],-forward[0],0); up=(0,0,1)
center=v(world[chest].translation)
def body(depth,lateral,height):
    xy=add(center,add(mul(forward,depth),mul(right_axis,lateral)))
    return (xy[0],xy[1],height)
def coords(p):
    d=sub(p,center); return (dot(d,forward),dot(d,right_axis),p[2])
R='Bip001-R-Hand'; L='Bip001-L-Hand'
rp=v(world[R].translation); lp=v(world[L].translation)
left=[(0,lp),(.10,lp),(.60,body(26,-23,136)),(1.65,body(26,-23,136)),(1.88,body(27,-23,138)),(2.12,body(27,-23,138)),(2.30,body(28,-24,139)),(2.8,lp)]
right=[(0,rp),(.10,rp),(.34,body(24,13,113)),(.60,body(28,-19,129)),(.84,body(29,-17,119)),(1.06,body(20,10,106)),(1.20,body(20,10,106)),(1.48,body(28,-19,121)),(1.65,body(28,-19,130)),(1.78,body(28,-19,130)),(1.94,body(29,-21,142)),(2.10,body(25,-19,142)),(2.28,body(28,-6,129)),(2.52,body(23,12,111)),(2.8,rp)]
weights=[(0,(0,)),(.10,(0,)),(.60,(1,)),(2.30,(1,)),(2.8,(0,))]
tracks={n:[[],[],[]] for n in names}; samples=[]
for f in range(169):
    t=f/60; w=curve(weights,t)[0]
    rot={}; pos={}; solved={}
    for n in names:
        parent=parents[n]; pq=rot.get(parent,(0,0,0,1)); pp=pos.get(parent,(0,0,0))
        pos[n]=add(pp,rotate(pq,v(local[n].translation)))
        rot[n]=qm(pq,q(local[n]))
        if n.endswith(('-UpperArm','-Forearm','-Hand')):
            side='R' if '-R-' in n else 'L'; pre='Bip001-'+side+'-'
            if n.endswith('-UpperArm'):
                shoulder=pos[n]; target=curve(right if side=='R' else left,t)
                a=length(v(local[pre+'Forearm'].translation)); b=length(v(local[pre+'Hand'].translation))
                d=length(sub(target,shoulder)); assert abs(a-b)<d<a+b+.01,(f,side,'reach',d,a+b)
                direction=norm(sub(target,shoulder)); along=(a*a-b*b+d*d)/(2*d)
                original_elbow=v(world[pre+'Forearm'].translation)
                desired_pole=add(shoulder,add(mul(forward,18),add(mul(right_axis,12 if side=='R' else -12),mul(up,-22))))
                pole=sub(mix(original_elbow,desired_pole,w),shoulder)
                bend=norm(sub(pole,mul(direction,dot(pole,direction))))
                elbow=add(shoulder,add(mul(direction,along),mul(bend,math.sqrt(max(0,a*a-along*along)))))
                normal=norm(cross(sub(elbow,shoulder),sub(target,elbow)))
                src_upper=sub(v(world[pre+'Forearm'].translation),v(world[pre+'UpperArm'].translation))
                src_lower=sub(v(world[pre+'Hand'].translation),v(world[pre+'Forearm'].translation))
                src_normal=norm(cross(src_upper,src_lower))
                upper_delta=qm(frame_quat(sub(elbow,shoulder),normal),inv(frame_quat(src_upper,src_normal)))
                lower_delta=qm(frame_quat(sub(target,elbow),normal),inv(frame_quat(src_lower,src_normal)))
                solved[side]=(upper_delta,lower_delta,target,elbow)
                rot[n]=qm(upper_delta,q(world[n]))
            elif n.endswith('-Forearm'):
                rot[n]=qm(solved[side][1],q(world[n]))
            else:
                # Maintain the source gun grip; only a small inward cant during reload.
                delta=axis(forward,8*w if side=='L' else -8*w)
                rot[n]=qm(delta,q(world[n]))
        lq=norm(qm(inv(pq),rot[n]))
        tracks[n][0].append(local[n].translation)
        tracks[n][1].append(unreal.Quat(*lq))
        tracks[n][2].append(local[n].scale3d)
    samples.append({'frame':f,'joints':{n:vect for n,vect in pos.items() if n in ['Bip001-Pelvis','Bip001-Spine2','Bip001-Neck','Bip001-L-UpperArm','Bip001-R-UpperArm','Bip001-L-Forearm','Bip001-R-Forearm',R,L]},'body':{n:coords(pos[n]) for n in ['Bip001-R-Forearm',R,'Bip001-L-Forearm',L]}})
# Filter only arm orientation keys; re-evaluate front-plane clearance after saving.
for n in names:
    if not n.endswith(('-UpperArm','-Forearm','-Hand')): continue
    keys=[(r.x,r.y,r.z,r.w) for r in tracks[n][1]]
    for iteration in range(10):
        result=list(keys)
        for i in range(1,168):
            a=keys[i-1] if dot(keys[i-1],keys[i])>=0 else mul(keys[i-1],-1)
            b=keys[i+1] if dot(keys[i+1],keys[i])>=0 else mul(keys[i+1],-1)
            result[i]=norm(add(mul(keys[i],.5),mul(add(a,b),.25)))
        keys=result
    tracks[n][1]=[unreal.Quat(*r) for r in keys]
# End at the actual firing start pose; preserve the exact firing end at frame zero.
endpose=unreal.AnimPoseExtensions.get_anim_pose_at_time(source,0,unreal.AnimPoseEvaluationOptions())
for n in names:
    end=endpose.get_bone_pose(n)
    for f in range(145,169):
        weight=smooth((f-145)/23); old=tracks[n][1][f]
        delta=qm(inv(q(local[n])),q(end))
        if delta[3]<0: delta=mul(delta,-1)
        corr=norm(mix((0,0,0,1),delta,weight))
        tracks[n][1][f]=unreal.Quat(*norm(qm((old.x,old.y,old.z,old.w),corr)))
        tracks[n][0][f]=unreal.Vector(*mix(v(local[n].translation),v(end.translation),weight))
        tracks[n][2][f]=unreal.Vector(*mix(v(local[n].scale3d),v(end.scale3d),weight))
    for f,tr in [(0,local[n]),(168,end)]:
        tracks[n][0][f]=tr.translation; tracks[n][1][f]=tr.rotation; tracks[n][2][f]=tr.scale3d
factory=unreal.AnimSequenceFactory()
factory.set_editor_property('target_skeleton',sk)
factory.set_editor_property('preview_skeletal_mesh',mesh)
exists=unreal.EditorAssetLibrary.does_asset_exist(DEST+'/'+NAME)
anim=unreal.load_asset(DEST+'/'+NAME) if exists else unreal.AssetToolsHelpers.get_asset_tools().create_asset(NAME,DEST,unreal.AnimSequence,factory)
c=anim.get_editor_property('controller')
c.open_bracket('Torso-front reload paths and elbow planes',False)
c.set_frame_rate(unreal.FrameRate(60,1),False)
c.set_number_of_frames(unreal.FrameNumber(168),False)
for n,(p,r,s) in tracks.items():
    if not exists: c.add_bone_curve(n,False)
    assert c.set_bone_track_keys(n,p,r,s,False),n
c.close_bracket(False)
unreal.AnimationLibrary.remove_all_animation_sync_markers(anim)
if not exists: unreal.AnimationLibrary.add_animation_notify_track(anim,'ReloadTiming')
for name,time in [('MagazineOut',.84),('MagazineIn',1.65),('SlidePull',2.10),('ReloadComplete',2.30)]:
    unreal.AnimationLibrary.add_animation_sync_marker(anim,name,time,'ReloadTiming')
unreal.EditorAssetLibrary.set_metadata_tag(anim,'Description','Torso-space front reload, 2.8s/60fps. Left hand draws pistol inward. Right hand and elbow stay anterior to torso; magazine pickup at front belt. Source AS_GunFire_Player endpoints. Sync markers are timing guides only.')
assert unreal.EditorAssetLibrary.save_loaded_asset(anim)
with open(os.path.join(os.path.dirname(__file__),'front_reload_paths.json'),'w') as f:
    json.dump({'forward':forward,'right':right_axis,'center':center,'samples':samples},f,indent=2)
print('FRONT_RELOAD_SAVED',anim.get_path_name(),forward)
