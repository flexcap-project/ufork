#! /bin/bash
find . -type f -regex '.*/.*\.\(c\)$' -exec bash -c '
    for pathname do

        python ~/.unikraft/apps/micropython/build/libmicropython/origin/micropython-201951645afb7cd0990c95144e204e1f536761db/py/makeqstrdefs.py split qstr $pathname qstr t.t

    done' bash {} +
