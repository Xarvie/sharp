import subprocess, os, sys

sharpc = r'cmake-build-debug\sharpc.exe'
env = os.environ.copy()
env['LIB'] = r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\lib\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\um\x64'

tests = [
    'test_func_pointer',
    'test_const_func_ptr',
    'test_feat6_wchar',
]

for name in tests:
    sp = 'tests/' + name + '.sp'
    c = name + '.c'
    r1 = subprocess.run([sharpc, sp, '-no-link', '-o', c], capture_output=True, env=env)
    sys.stdout.write('=== {} ===\n'.format(name))
    sys.stdout.write('sharpc exit={}\n'.format(r1.returncode))
    if r1.stderr:
        sys.stdout.write('sharpc stderr={}\n'.format(r1.stderr[:300].decode('utf-8', errors='replace')))
    if os.path.exists(c):
        with open(c) as f:
            content = f.read()
            sys.stdout.write(content[:800] + '\n')
        r2 = subprocess.run(['clang', '-std=c11', '-w', '-fsyntax-only', c], capture_output=True, text=True)
        if r2.returncode != 0:
            sys.stdout.write('clang: {}\n'.format(r2.stderr[:400]))
        else:
            sys.stdout.write('clang OK\n')
    else:
        sys.stdout.write('NO OUTPUT\n')
    sys.stdout.write('\n')
    try: os.remove(c)
    except: pass
