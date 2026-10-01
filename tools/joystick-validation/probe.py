"""Exercise the installed browser-input protocol against isolated ArduSub SITL.

This is not a browser UI or USB-gamepad test. No hardware connections are used.
"""
import asyncio,json,math,os,signal,subprocess,time,traceback
from pathlib import Path
import rclpy
from rclpy.qos import qos_profile_sensor_data
from sensor_msgs.msg import Joy
from mavros_msgs.msg import ManualControl,State
from nav_msgs.msg import Odometry
import websockets

out=Path('/results');rows=[];phase='startup';state={'connected':False,'armed':False}
result={'scope':'Published ARM64 candidate, headless websocket -> ROS Joy -> MAVROS -> ArduSub SITL; NOT browser UI or physical joystick','checks':{}}
cmd=['ros2','launch','dave_demos','dave_robot.launch.py','z:=-0.5','namespace:=bluerov2','world_name:=dave_ocean_waves','paused:=false','gui:=false','headless:=true','use_teleop:=true','use_web_joystick:=true','open_qgc:=false','open_virtual_joystick:=false']
result['command']=cmd
f=(out/'messages.jsonl').open('w',buffering=1)
def emit(kind,d):
    row={'t':time.monotonic(),'phase':phase,'kind':kind,**d};rows.append(row);f.write(json.dumps(row)+'\n')
def onstate(m):
    state.update(connected=m.connected,armed=m.armed,mode=m.mode);emit('state',dict(state))
def onodom(m):
    p=m.pose.pose.position;v=m.twist.twist.linear
    emit('odom',{'position':[p.x,p.y,p.z],'velocity':[v.x,v.y,v.z]})

async def exercise(node):
    global phase
    async def tick(ws,seconds,axes=None,button=None):
        end=time.monotonic()+seconds
        while time.monotonic()<end:
            if ws is not None:
                buttons=[0]*17
                if button is not None:buttons[button]=1
                await ws.send(json.dumps({'id':'posim_validation','axes':axes or [0]*6,'buttons':buttons}))
            for _ in range(12):rclpy.spin_once(node,timeout_sec=0)
            await asyncio.sleep(.04)
    deadline=time.monotonic()+100
    while time.monotonic()<deadline and not state['connected']:await tick(None,.25)
    result['checks']['mavros_connected']=state['connected']
    if not state['connected']:raise RuntimeError('No MAVROS connection within 100 seconds')
    async with websockets.connect('ws://127.0.0.1:8765') as ws:
        result['checks']['websocket_connected']=True
        try:
            phase='neutral_baseline';await tick(ws,4)
            phase='mode';await tick(ws,.5,button=0);await tick(ws,2)
            phase='arm';await tick(ws,.5,button=9);await tick(ws,5)
            result['checks']['armed']=state['armed']
            if not state['armed']:raise RuntimeError('Normal arm request was not accepted; no force-arming attempted')
            phase='forward';await tick(ws,8,[0,1,0,0,0,0])
            phase='neutral_after';await tick(ws,4)
            phase='reverse';await tick(ws,8,[0,-1,0,0,0,0])
            phase='neutral_end';await tick(ws,4)
        finally:
            phase='disarm';await tick(ws,.5,button=8);await tick(ws,3)
            result['checks']['disarmed']=not state['armed']
    joys=[r for r in rows if r['kind']=='joy']
    controls=[r for r in rows if r['kind']=='manual']
    result['checks']['forward_joy']=any(r['phase']=='forward' and len(r['axes'])>1 and r['axes'][1]>.9 for r in joys)
    result['checks']['reverse_joy']=any(r['phase']=='reverse' and len(r['axes'])>1 and r['axes'][1]<-.9 for r in joys)
    result['checks']['forward_manual']=any(r['phase']=='forward' and r['x']>100 for r in controls)
    result['checks']['reverse_manual']=any(r['phase']=='reverse' and r['x']<-100 for r in controls)
    for stage in ['forward','reverse']:
        obs=[r for r in rows if r['kind']=='odom' and r['phase']==stage]
        displacement=math.dist(obs[0]['position'][:2],obs[-1]['position'][:2]) if len(obs)>1 else 0
        result[stage+'_planar_displacement_m']=displacement
        result['checks'][stage+'_vehicle_motion']=len(obs)>1 and displacement>.05
    end=[r for r in controls if r['phase']=='neutral_end']
    result['checks']['neutral_command']=len(end)>5 and all(abs(r['x'])<1e-5 and abs(r['y'])<1e-5 and abs(r['r'])<1e-5 for r in end[-5:])

p=None;node=None
try:
    rclpy.init();node=rclpy.create_node('posim_joystick_validation')
    subs=[node.create_subscription(State,'/mavros/state',onstate,qos_profile_sensor_data),node.create_subscription(Odometry,'/model/bluerov2/odometry',onodom,qos_profile_sensor_data),node.create_subscription(Joy,'/joy',lambda m:emit('joy',{'axes':list(m.axes),'buttons':list(m.buttons)}),10),node.create_subscription(ManualControl,'/mavros/manual_control/send',lambda m:emit('manual',{'x':m.x,'y':m.y,'z':m.z,'r':m.r}),10)]
    with (out/'launch.log').open('w') as log:
        p=subprocess.Popen(cmd,stdout=log,stderr=subprocess.STDOUT,start_new_session=True)
        try:asyncio.run(exercise(node))
        finally:
            try:os.killpg(p.pid,signal.SIGINT)
            except ProcessLookupError:pass
            try:result['launch_exit']=p.wait(timeout=25)
            except subprocess.TimeoutExpired:
                os.killpg(p.pid,signal.SIGKILL);result['launch_exit']=p.wait();result['forced_shutdown']=True
except Exception as e:
    result['error']=repr(e);(out/'exception.txt').write_text(traceback.format_exc())
finally:
    if node is not None:node.destroy_node()
    if rclpy.ok():rclpy.shutdown()
    f.close()
result['status']='PASS' if result['checks'] and all(result['checks'].values()) and 'error' not in result else 'FAIL'
(out/'result.json').write_text(json.dumps(result,indent=2));print(json.dumps(result),flush=True)
raise SystemExit(0 if result['status']=='PASS' else 1)
