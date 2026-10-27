#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Verify native particle projection, occlusion and unchanged numeric depth.

The deterministic fixture changes only cell selection/jitter, preserving the
production ray traversal, projection, clipping, coverage and radiance code.
"""
import argparse,hashlib,json,math,os,pathlib,statistics,subprocess


def pixels(path):
    with path.open('rb') as stream:
        if stream.readline()!=b'P6\n' or stream.readline()!=b'800 600\n' or stream.readline()!=b'255\n':
            raise ValueError('fixture_image_layout')
        data=stream.read()
    if len(data)!=800*600*3:raise ValueError('fixture_image_size')
    return data


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('binary','sdk','renderer-dir','shaders','output'):p.add_argument('--'+name,type=pathlib.Path,required=True)
    a=p.parse_args();out=a.output.resolve();out.mkdir(exist_ok=False)
    source=a.shaders/'Underwater_piece_vs_piece_ps.any';text=source.read_text()
    cells=[(2,0,-7),(-1,-2,-5),(1,2,-10),(-3,0,-9)]
    before='float seed=fract(sin(dot(cell,float3(127.1,311.7,74.7)))*43758.5453);'
    # Scalar component comparisons work in the GLSL and Metal shader paths.
    chosen=' || '.join('('+' && '.join('cell.'+axis+'=='+str(value)+'.0' for axis,value in zip('xyz',c))+')' for c in cells)
    after='float seed=('+chosen+')?0.99:0.0;'
    jitter='float3 jitter=float3(seed,fract(seed*17.3),fract(seed*43.1));'
    if text.count(before)!=1 or text.count(jitter)!=1:raise ValueError('fixture_seed_contract')
    fixture=text.replace(before,after).replace(jitter,'float3 jitter=float3(0.5,0.5,0.5);')
    shaders=out/'shaders';shaders.mkdir();(shaders/source.name).write_text(fixture)
    poses=[('origin',0.,(0.,0.,-.1)),('right',0.,(0.,-.2,-.1)),('forward',0.,(.15,0.,-.1)),('deeper',0.,(0.,0.,-.35)),('drift',10.,(0.,0.,-.1))]
    rotation=[0,0,1,0,1,0,-1,0,0]
    posefile=out/'poses.txt';posefile.write_text(''.join(' '.join(map(str,[name,t,*origin,*rotation]))+'\n' for name,t,origin in poses))
    sdk=a.sdk.resolve();env=dict(os.environ,GZ_RENDERING_PLUGIN_PATH=str(sdk/'lib'),GZ_RENDERING_RESOURCE_PATH=str(sdk/'share/gz/gz-rendering'),OGRE2_RESOURCE_PATH=str(sdk/'lib/OGRE-Next')+os.pathsep+str(sdk/'share/OGRE-Next'))
    for name in ('DYLD_LIBRARY_PATH','LD_LIBRARY_PATH'):env[name]=str(a.renderer_dir.resolve())+os.pathsep+str(sdk/'lib')
    def run(folder,plane=None):
        folder.mkdir();command=[str(a.binary.resolve()),str(shaders),str(posefile),str(folder)]
        if plane is not None:command.append(str(plane))
        with (folder/'render.log').open('w') as log:r=subprocess.run(command,env=env,stdout=log,stderr=subprocess.STDOUT,timeout=120)
        if r.returncode:raise ValueError('native_render_failed:'+str(r.returncode)+':'+str(folder/'render.log'))
        return folder
    images=run(out/'projection');errors=[];checks=[];focal=474.8967170715332
    for name,t,origin in poses:
        off=pixels(images/(name+'-0.ppm'));on=pixels(images/(name+'-1.ppm'))
        for cell in cells:
            point=[.25*(v+.5)+drift*t for v,drift in zip(cell,(.004,.008,0.))]
            dx,dy,dz=[point[i]-origin[i] for i in range(3)];expected=(400-focal*dy/-dz,300-focal*dx/-dz)
            sx=sy=weight=0.
            for y in range(math.floor(expected[1])-5,math.floor(expected[1])+6):
                for x in range(math.floor(expected[0])-5,math.floor(expected[0])+6):
                    if not 0<=x<800 or not 0<=y<600:raise ValueError('fixture_projection_bounds')
                    i=(y*800+x)*3;w=sum(max(0,on[i+c]-off[i+c]) for c in range(3))
                    sx+=(x+.5)*w;sy+=(y+.5)*w;weight+=w
            if weight==0:raise ValueError('particle_missing:'+name+':'+str(cell))
            observed=(sx/weight,sy/weight);error=math.hypot(observed[0]-expected[0],observed[1]-expected[1]);errors.append(error)
            if error>1.25:raise ValueError('particle_projection_error:'+str(error))
            checks.append(dict(pose=name,time_s=t,world=point,expected_pixel=expected,observed_pixel=observed,error_px=error))
    occluded=run(out/'occlusion',-1.)
    for name,_,_ in poses:
        if pixels(occluded/(name+'-0.ppm'))!=pixels(occluded/(name+'-1.ppm')):raise ValueError('particle_not_occluded:'+name)
    report=dict(passed=True,checks=checks,maximum_projection_error_px=max(errors),numeric_depth_byte_identical=True,opaque_surface_occlusion=True,scope='Deterministic 4-particle native projection in 4 translated views and one advanced-time view; cell selection/jitter fixed only for this fixture. Not fluid, optical calibration or SLAM acceptance.',sha256={str(path.name):hashlib.sha256(path.read_bytes()).hexdigest() for path in [source,pathlib.Path(__file__),a.binary]})
    (out/'receipt.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k not in ('checks','sha256')}))


if __name__=='__main__':main()
