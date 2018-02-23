# -*- coding: utf-8 -*-

import abc
import os
import re
import statistics
import subprocess

from perftest import NotFoundError, ParseError, ArgumentError
from perftest import logger, result, runtools, stencils, utils


class Runtime(metaclass=abc.ABCMeta):
    """Base class of all runtimes.

    A runtime class represents a software for running stencils, currently
    STELLA or Gridtools.
    """
    def __init__(self, config, grid, precision, backend):
        if grid not in self.grids:
            sup = ', '.join(f'"{g}"' for g in self.grids)
            raise ArgumentError(
                    f'Invalid grid "{grid}", supported are {sup}')
        if precision not in self.precisions:
            sup = ', '.join(f'"{p}"' for p in self.precisions)
            raise ArgumentError(
                    f'Invalid precision "{precision}", supported are {sup}')
        if backend not in self.backends:
            sup = ', '.join(f'"{b}"' for b in self.backends)
            raise ArgumentError(
                    f'Invalid backend "{backend}", supported are {sup}')
        self.config = config
        self.grid = grid
        self.precision = precision
        self.backend = backend
        self.stencils = stencils.load(grid)

    def run(self, domain, runs):
        """Method to run all stencils on the given `domain` size.

        Computes mean and stdev of the run times for all stencils for the
        given number of `runs`.

        Args:
            domain: Domain size as a tuple or list.
            runs: Number of runs to do.

        Returns:
            A `perftest.result.Result` object with the collected times.
        """
        commands = [self.command(s, domain) for s in self.stencils]

        allcommands = [c for c in commands for _ in range(runs)]
        logger.info('Running stencils')
        alloutputs = runtools.run(allcommands, self.config)
        logger.info('Running stencils finished')

        alltimes = [self._parse_time(o) for o in alloutputs]

        times = [alltimes[i:i+runs] for i in range(0, len(alltimes), runs)]

        meantimes = [statistics.mean(t) for t in times]
        stdevtimes = [statistics.stdev(t) if len(t) > 1 else 0 for t in times]

        return result.Result(runtime=self,
                             domain=domain,
                             meantimes=meantimes,
                             stdevtimes=stdevtimes)

    @staticmethod
    def _parse_time(output):
        """Parses the ouput of a STELLA or Gridtools run for the run time.

        Args:
            output: STELLA or Gridtools stdout output.

        Returns:
            Run time in seconds.
        """
        p = re.compile(r'.*\[s\]\s*([0-9.]+).*', re.MULTILINE | re.DOTALL)
        m = p.match(output)
        if not m:
            raise ParseError(f'Could not parse time in output:\n{output}')
        return float(m.group(1))

    def __str__(self):
        return (f'Runtime(name={self.name}, version={self.version}, '
                f'datetime={self.datetime}, path={self.path})')

    @property
    def name(self):
        """Name, class name converted to lower case and 'Runtime' stripped."""
        return type(self).__name__.lower().rstrip('runtime')

    @abc.abstractproperty
    def version(self):
        """Version number or hash."""
        pass

    @abc.abstractproperty
    def datetime(self):
        """(Build or commit) date of the software as a string."""
        pass

    @abc.abstractmethod
    def path(self):
        """Base path of all binaries."""
        pass

    @abc.abstractmethod
    def binary(self, stencil):
        """Stencil-dependent path to the binary."""
        pass

    @abc.abstractmethod
    def command(self, stencil, domain):
        """Full command to run the given stencil on the given dommain."""
        pass


class StellaRuntimeBase(Runtime):
    """Base class for all STELLA runtimes."""
    grids = ['strgrid']
    precisions = ['float', 'double']
    backends = ['cuda', 'host']

    def binary(self, stencil):
        """STELLA binary path."""
        suffix = 'CUDA' if self.backend == 'cuda' else ''
        binary = os.path.join(self.path, f'StandaloneStencils{suffix}')
        if not os.path.isfile(binary):
            raise NotFoundError(f'Could not find STELLA binary at {binary}')
        return binary

    def command(self, stencil, domain):
        """STELLA run command."""
        binary = self.binary(stencil)
        ni, nj, nk = domain
        filt = stencil.stella_filter
        return f'{binary} --ie {ni} --je {nj} --ke {nk} --gtest_filter={filt}'


class GridtoolsRuntimeBase(Runtime):
    """Base class for all Gridtools runtimes."""
    grids = ['strgrid', 'icgrid']
    precisions = ['float', 'double']
    backends = ['cuda', 'host']

    @property
    def version(self):
        """Gridtools git commit hash."""
        return subprocess.check_output(['git', 'rev-parse', 'HEAD'],
                                       cwd=self.path).decode().strip()

    @property
    def datetime(self):
        """Gridtools git commit date and time."""
        commit = self.version
        posixtime = subprocess.check_output(['git', 'show', '-s',
                                             '--format=%ct', commit],
                                            cwd=self.path).decode().strip()
        return utils.timestr_from_posix(posixtime)

    def binary(self, stencil):
        """Stencil-dependent Gridtools binary path."""
        binary = getattr(stencil, 'gridtools_' + self.backend.lower())
        binary = os.path.join(self.path, binary)
        if not os.path.isfile(binary):
            raise NotFoundError(f'Could not find GridTools binary at {binary}')
        return binary

    def command(self, stencil, domain):
        """Gridtools run command."""
        binary = self.binary(stencil)
        ni, nj, nk = domain
        halo = stencil.halo
        ni, nj, nk = ni + 2 * halo, nj + 2 * halo, nk + 2 * halo
        return f'{binary} {ni} {nj} {nk} 10'
