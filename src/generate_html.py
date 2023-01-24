from pathlib import Path

if __name__ == '__main__':
    with open('../include/precompile.h.in') as fin:
        with open('index.html') as fhtml:
            Path("../include/gen").mkdir(exist_ok=True)
            with open('../include/gen/precompile.h', 'w') as fout:
                fout.write(fin.read().replace('$1', fhtml.read()))