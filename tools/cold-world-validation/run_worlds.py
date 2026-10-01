import json,os,subprocess,time
from pathlib import Path
base=Path(__file__).resolve().parent
image=os.environ.get('POSIM_TEST_IMAGE','ioeslab/posim@sha256:72179187c96a801184a15ab9cdaf3ca9714d3717ce04e97cd2ec60f34d83edb8')
source='source /opt/ros/lyrical/setup.bash; source /home/docker/dave_ws/install/setup.bash; '
common=['docker','run','--rm','--init','--platform','linux/arm64','--shm-size=1g','--entrypoint','bash','-e','ROS_DOMAIN_ID=133','-e','GZ_IP=127.0.0.1','-e','LIBGL_ALWAYS_SOFTWARE=1','-e','QT_QPA_PLATFORM=offscreen','-v',str(base)+':/results']
raw=subprocess.check_output(common+[image,'-lc',source+"find $(ros2 pkg prefix --share dave_worlds)/worlds -maxdepth 1 -name '*.world' -printf '%f\\n'"],text=True)
inventory=sorted(x[:-6] for x in raw.splitlines() if x.endswith('.world'))
names=['dave_integrated','dave_ocean_models']
assert set(names)<=set(inventory), 'Diagnostic worlds missing from installed image'
(base/'world_inventory.json').write_text(json.dumps(names,indent=2))
for i,name in enumerate(names):
    print(f'[{i+1}/{len(names)}] {name}',flush=True)
    cname=f'posim-cold-world-{os.getpid()}'
    try:
        r=subprocess.run(common+['--name',cname,'-e',f'GZ_PARTITION={cname}-{name}',image,'-lc',source+f'cd /tmp; python3 /results/world_probe.py {name}'],text=True,capture_output=True,timeout=330)
        (base/f'runner-{name}.log').write_text(r.stdout+'\n'+r.stderr)
        p=base/'worlds'/name/'result.json'
        print(json.loads(p.read_text())['status'] if p.exists() else f'NO_RESULT exit={r.returncode}',flush=True)
    except subprocess.TimeoutExpired:
        subprocess.run(['docker','stop','-t','5',cname],capture_output=True)
        print('RUNNER_TIMEOUT',flush=True)
rows=[json.loads(p.read_text()) for p in sorted((base/'worlds').glob('*/result.json'))]
(base/'world_summary.json').write_text(json.dumps(rows,indent=2))
print('SUMMARY',[(x['world_name'],x['status']) for x in rows],flush=True)

raise SystemExit(1 if len(rows)!=len(names) or any(x["status"]=="FAIL" for x in rows) else 0)
