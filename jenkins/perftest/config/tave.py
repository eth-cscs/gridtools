# -*- coding: utf-8 -*-

import os
import textwrap

from perftest import runtime


def sbatch(command):
    return textwrap.dedent(f"""\
        #!/bin/bash -l
        #SBATCH --job-name=gridtools_perftest
        #SBATCH --time=00:10:00
        #SBATCH --constraint=flat,quad

        srun numactl -m 1 {command}

        sync
        """)
