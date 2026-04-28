import subprocess, os

sharpc = r'cmake-build-debug\sharpc.exe'
env = os.environ.copy()
env['LIB'] = r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\lib\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\um\x64'

sp = 'tests/test_feat6_wchar.sp'
c = 'test_feat6_wchar.c'
subprocess.run([sharpc, sp, '-no-link', '-o', c], capture_output=True, env=env)
with open(c) as f: print(f.read())
r = subprocess.run(['clang', '-std=c11', '-w', '-fsyntax-only', c], capture_output=True, text=True)
if r.returncode != 0:
    print('clang: {}'.format(r.stderr[:400]))
else:
    print('clang OK')
try: os.remove(c)
except: pass
