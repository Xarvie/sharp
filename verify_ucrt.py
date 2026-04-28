import subprocess, tempfile, os, sys

sharpc = r'cmake-build-debug\sharpc.exe'
env = os.environ.copy()
env['LIB'] = r'C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.43.34808\lib\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\ucrt\x64;C:\Program Files (x86)\Windows Kits\10\lib\10.0.22621.0\um\x64'

headers = [
    'complex.h', 'corecrt.h', 'corecrt_startup.h', 'corecrt_stdio_config.h',
    'corecrt_wconio.h', 'corecrt_wctype.h', 'corecrt_wstdlib.h',
    'crtdbg.h', 'fpieee.h', 'mbctype.h', 'mbstring.h',
    'minmax.h', 'new.h', 'search.h', 'uchar.h',
]

tmp = tempfile.gettempdir()
results = []
for h in headers:
    safe = h.replace('/', '_').replace('.', '_')
    sp = os.path.join(tmp, 'test_' + safe + '.sp')
    c  = os.path.join(tmp, 'test_' + safe + '.c')
    with open(sp, 'w', encoding='ascii') as f:
        f.write('/* auto */\n#include <' + h + '>\nint main() { return 0; }\n')
    # sharpc
    r1 = subprocess.run([sharpc, sp, '-no-link', '-o', c], capture_output=True, text=True, env=env)
    sharpc_ok = (r1.returncode == 0 and os.path.exists(c))
    # clang
    clang_ok = False
    clang_err = ''
    if sharpc_ok:
        r2 = subprocess.run(['clang', '-std=c11', '-w', '-fsyntax-only', c], capture_output=True, text=True, env=env)
        clang_ok = (r2.returncode == 0)
        if not clang_ok:
            clang_err = r2.stderr.strip()[:200]
    status = 'PASS' if (sharpc_ok and clang_ok) else ('FAIL' if not sharpc_ok else 'CLANG_FAIL')
    results.append((h, status, clang_err if status == 'CLANG_FAIL' else ''))
    try: os.remove(sp)
    except: pass
    try: os.remove(c)
    except: pass

print('{:<35} {:<12}'.format('Header', 'Status'))
print('-' * 50)
for h, s, e in results:
    mark = 'OK' if s == 'PASS' else 'XX'
    print('[{}] {:<33} {:<12}'.format(mark, h, s))
    if e:
        print('    ' + e[:120])
passed = sum(1 for _, s, _ in results if s == 'PASS')
print('\n{}/{} headers pass both sharpc + clang'.format(passed, len(results)))
