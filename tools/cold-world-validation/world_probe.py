#!/usr/bin/env python3
"""One installed world per disposable container. No source overlays or SDF edits."""
import hashlib,json,os,re,signal,subprocess,sys,time
import xml.etree.ElementTree as ET
from pathlib import Path
from ament_index_python.packages import get_package_share_directory

name=sys.argv[1]
out=Path('/results/worlds')/name
out.mkdir(parents=True,exist_ok=True)
worldfile=Path(get_package_share_directory('dave_worlds'))/'worlds'/f'{name}.world'
root=ET.parse(worldfile).getroot(); world=root.find('world'); worldname=world.get('name')
sonar=any('multibeam_sonar_system' in x.get('filename','') for x in root.iter('plugin'))
cmd=['ros2','launch','dave_demos','dave_world.launch.py',f'world_name:={name}','headless:=true','verbose:=true']
result={'harness':'cold-download diagnostic: 240s asset-ready budget; post-readiness stats; child shutdown audited','startup_budget_seconds':240,'original_run':'36833452768 (50-second baseline retained, not overwritten)','world_file':str(worldfile),'sha256':hashlib.sha256(worldfile.read_bytes()).hexdigest(),'world_name':worldname,'command':cmd,'cuda_world':sonar,'checks':{},'scope':'Diagnostic of two 50s startup timeouts with a 240s cold-asset budget. No source, model, plugin or image changes. Checks readiness, advancing simulation, direct models and shutdown; not every sensor or task'}
def cap(label,args,timeout=10):
    try:
        c=subprocess.run(args,capture_output=True,text=True,timeout=timeout)
        rc,s,e=c.returncode,c.stdout,c.stderr
    except subprocess.TimeoutExpired as x:
        rc=124;s=x.stdout or '';e=x.stderr or ''
        if isinstance(s,bytes):s=s.decode(errors='replace')
        if isinstance(e,bytes):e=e.decode(errors='replace')
    (out/(label+'.txt')).write_text(s);(out/(label+'.stderr')).write_text(e)
    (out/(label+'.command.json')).write_text(json.dumps({'command':args,'returncode':rc}))
    return rc,s
p=None;forced=False
try:
    with (out/'launch.log').open('w') as log:
        p=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        started=time.monotonic();deadline=started+240;ready=False;n=0
        while time.monotonic()<deadline and p.poll() is None:
            n+=1;rc,s=cap(f'services-{n}',['gz','service','-l'],7)
            if f'/world/{worldname}/control' in s.splitlines():ready=True;break
            time.sleep(1)
        result['checks']['world_ready']=ready
        if ready:
            attempts=0
            while True:
                attempts+=1
                rc,s=cap(f'scene-{attempts}',['gz','service','-s',f'/world/{worldname}/scene/info','--reqtype','gz.msgs.Empty','--reptype','gz.msgs.Scene','--timeout','5000','--req',''],8)
                if 'name:' in s or time.monotonic()>=deadline or p.poll() is not None:break
            result['scene_attempts']=attempts
            expected=[x.get('name') for x in world.findall('model')]+[x.findtext('name') for x in world.findall('include') if x.findtext('name')]
            missing=[x for x in expected if not re.search(r'name:\s*"'+re.escape(x)+'"',s)]
            result['expected_direct_models']=expected;result['missing_direct_models']=missing
            result['checks']['scene_response']=rc==0 and 'name:' in s
            result['checks']['direct_models_present']=not missing
            result['scene_observed_seconds']=time.monotonic()-started
            time.sleep(4)
            rc,s=cap('stats',['gz','topic','-e','-t',f'/world/{worldname}/stats','-n','3'],15)
            it=[int(x) for x in re.findall(r'iterations:\s*(\d+)',s)]
            result['iterations']=it
            result['checks']['simulation_advances']=rc==0 and len(it)>=2 and it[-1]>it[0]
            cap('gz_topics',['gz','topic','-l'])
            result['checks']['launch_alive']=p.poll() is None
        try:os.killpg(p.pid,signal.SIGINT)
        except ProcessLookupError:pass
        try:ret=p.wait(timeout=20)
        except subprocess.TimeoutExpired:
            forced=True;os.killpg(p.pid,signal.SIGKILL);ret=p.wait(timeout=8)
        result['shutdown_returncode']=ret
        result['checks']['clean_shutdown']=not forced and ret in [0,130,-2]
except Exception as e:
    result['error']=repr(e)
finally:
    if p is not None and p.poll() is None:
        os.killpg(p.pid,signal.SIGKILL);p.wait()
text=(out/'launch.log').read_text(errors='replace')
pattern=r'Segmentation fault|exit code (?:-11|-6|134|139)\b|terminate called|Traceback \(most recent call last\)|Failed to load|Unable to find uri|Unable to read file|Could not find resource|Could not find file|Error Code [1-9]|error while loading shared libraries'
faults=[x for x in text.splitlines() if re.search(pattern,x,re.I)]
for line in text.splitlines():
    m=re.search(r'process has died .*exit code (-?\d+)\b',line)
    if m and int(m[1]) not in (0,130,-2) and line not in faults:faults.append(line)
    if "escalating to 'SIGTERM'" in line or "escalating to 'SIGKILL'" in line:result['checks']['clean_shutdown']=False
result['faults']=faults;result['checks']['no_fatal_or_resource_error']=not faults
result['status']='PASS' if result['checks'] and all(result['checks'].values()) and 'error' not in result else 'FAIL'
if sonar:
    result['status']='BACKEND_UNAVAILABLE'
    result['note']='No NVIDIA device/CUDA compute available in this Apple ARM64 container. Scene startup is recorded but does not certify sonar. Other faults remain listed.'
(out/'result.json').write_text(json.dumps(result,indent=2))
print(json.dumps(result),flush=True)
