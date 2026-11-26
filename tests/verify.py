"""Run supplied drivers and deterministic differential regex tests."""
import itertools
from pathlib import Path
import random
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
DATA = Path('/workspace/data/053/2684')


def compile_cpp(source, executable):
    subprocess.run(['g++', '-std=c++17', '-O2', '-Wall', '-Wextra',
                    '-pedantic', '-I', str(ROOT), str(source), '-o',
                    str(executable)], check=True)


with tempfile.TemporaryDirectory() as directory:
    temporary = Path(directory)
    for number in range(1, 8):
        source = temporary / 'driver.cpp'
        source.write_text((DATA / f'{number}.cpp').read_text())
        executable = temporary / 'driver'
        compile_cpp(source, executable)
        actual = subprocess.check_output([executable]).decode().split()
        expected = (DATA / f'{number}.ans').read_text().split()
        assert actual == expected, f'Supplied driver {number} failed'
    print('All seven supplied drivers passed.')

    executable = temporary / 'regex'
    compile_cpp(ROOT / 'main.cpp', executable)
    rng = random.Random(2684)
    atoms = [c + q for c in 'ab' for q in ['', '*', '+', '?']]
    patterns = atoms + [''.join(p) for p in itertools.product(atoms, repeat=2)]
    patterns += ['|'.join(''.join(rng.choices(atoms, k=rng.randint(1, 8)))
                          for _ in range(rng.randint(1, 4)))
                 for _ in range(500)]
    strings = [''.join(s) for length in range(1, 7)
               for s in itertools.product('ab', repeat=length)]
    cases = [(pattern, string) for pattern in patterns for string in strings]
    result = subprocess.run([executable], input=''.join(
        f'{pattern} {string}\n' for pattern, string in cases),
        text=True, capture_output=True, check=True)
    actual = result.stdout.split()
    assert len(actual) == len(cases)
    for (pattern, string), answer in zip(cases, actual):
        assert (answer == '1') == (re.fullmatch(pattern, string) is not None), (
            pattern, string, answer)
    print(f'All {len(cases)} differential cases passed.')
