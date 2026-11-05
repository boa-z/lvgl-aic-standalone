import os
import re

from building import *

Import('RTT_ROOT')

cwd = GetCurrentDir()
objs = []

lvgl_dir = os.environ.get('LVGL_DIR')
if not lvgl_dir:
    lvgl_dir = os.path.normpath(os.path.join(cwd, '..', 'lvgl'))
lvgl_dir = os.path.abspath(lvgl_dir)

if not os.path.isfile(os.path.join(lvgl_dir, 'lvgl.h')):
    raise RuntimeError(
        'LVGL source not found at %s; set LVGL_DIR to an official '
        'LVGL v9.6.0 tree' % lvgl_dir)

version_h = os.path.join(lvgl_dir, 'include', 'lvgl', 'lv_version.h')
if not os.path.isfile(version_h):
    version_h = os.path.join(lvgl_dir, 'lv_version.h')
if not os.path.isfile(version_h):
    raise RuntimeError('LVGL version header not found under %s' % lvgl_dir)

with open(version_h, 'r') as f:
    version_text = f.read()

def version_number(name):
    match = re.search(r'#define\s+%s\s+(\d+)' % name, version_text)
    return int(match.group(1)) if match else -1

major = version_number('LVGL_VERSION_MAJOR')
minor = version_number('LVGL_VERSION_MINOR')
patch = version_number('LVGL_VERSION_PATCH')
if (major, minor) != (9, 6):
    raise RuntimeError(
        'LVGL %d.%d.%d is not supported; lvgl-aic-standalone targets 9.6.x'
        % (major, minor, patch))
print('LVGL: %d.%d.%d from %s' % (major, minor, patch, lvgl_dir))

use_d21x = GetDepend('LVGL_AIC_USE_D21X_BACKEND')
use_touch = GetDepend('LVGL_AIC_USE_TOUCH')
build_demo = GetDepend('LVGL_AIC_DEMO_SMOKE')
bsp_root = os.environ.get('LVGL_AIC_BSP_DIR')

inc = [os.path.join(cwd, 'include'), os.path.join(cwd, 'src'),
       os.path.join(cwd, 'src', 'port', 'display'),
       os.path.join(cwd, 'src', 'port', 'input'),
       os.path.join(cwd, 'src', 'utils'),
       os.path.join(cwd, 'config'),
       os.path.join(RTT_ROOT, 'include'),
       os.path.join(RTT_ROOT, 'components', 'finsh'),
       os.path.join(RTT_ROOT, 'components', 'drivers', 'include'),
       lvgl_dir, os.path.join(lvgl_dir, 'include'),
       os.path.join(lvgl_dir, 'env_support', 'rt-thread'),
       os.path.join(lvgl_dir, 'src')]

if use_d21x:
    if not bsp_root:
        raise RuntimeError(
            'LVGL_AIC_USE_D21X_BACKEND requires LVGL_AIC_BSP_DIR')
    bsp_root = os.path.abspath(bsp_root)
    inc += [bsp_root, os.path.join(bsp_root, 'drivers'),
            os.path.join(bsp_root, 'board'),
            os.path.join(bsp_root, 'vendor', 'include')]

# Build unmodified upstream LVGL and component objects under the application's
# build directory.  The external source trees are never polluted.
build_root = os.path.join(Dir('#').abspath, 'build', 'lvgl_aic')
ext_root = os.path.join(build_root, 'lvgl_ext')
comp_root = os.path.join(build_root, 'component')
lv_env = Env.Clone()
lv_env.Append(CPPPATH=inc)
lv_env.Append(CFLAGS=['-std=gnu11', '-DCACHE_LINE_SIZE=64'])
exclude_root = os.path.join(lvgl_dir, 'src', 'libs', 'thorvg', 'rapidjson',
                            'msinttypes')

for root, dirs, files in os.walk(os.path.join(lvgl_dir, 'src')):
    if os.path.commonpath([exclude_root, root]) == exclude_root:
        continue
    for name in files:
        if not name.endswith('.c'):
            continue
        src = os.path.join(root, name)
        rel = os.path.relpath(src, lvgl_dir)
        target = os.path.join(ext_root, rel + '.o')
        target_dir = os.path.dirname(target)
        if not os.path.isdir(target_dir):
            os.makedirs(target_dir)
        objs += lv_env.Object(target, src)

src = [
    os.path.join(cwd, 'src', 'runtime', 'lvgl_aic_runtime.c'),
    os.path.join(cwd, 'src', 'port', 'display', 'lvgl_aic_display_port.c'),
    os.path.join(cwd, 'src', 'utils', 'lvgl_aic_display_state.c'),
]
if use_touch:
    src += [
        os.path.join(cwd, 'src', 'port', 'input', 'lvgl_aic_touch_port.c'),
        os.path.join(cwd, 'src', 'utils', 'lvgl_aic_touch_state.c'),
    ]
if use_d21x:
    src += [os.path.join(cwd, 'src', 'backend', 'd21x',
                         'lvgl_aic_d21x_display.c')]
else:
    src += [os.path.join(cwd, 'src', 'backend', 'rtthread',
                         'lvgl_aic_rtthread_display.c')]
if build_demo:
    src += [os.path.join(cwd, 'examples', 'd213-smoke',
                         'lvgl_aic_smoke.c')]

for source in src:
    rel = os.path.relpath(source, cwd)
    target = os.path.join(comp_root, rel + '.o')
    target_dir = os.path.dirname(target)
    if not os.path.isdir(target_dir):
        os.makedirs(target_dir)
    objs += lv_env.Object(target, source)

Return('objs')
