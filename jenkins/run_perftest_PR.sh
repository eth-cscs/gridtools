#!/bin/bash

source $(dirname "$0")/setup.sh

grid=structured

export GTCMAKE_GT_ENABLE_BACKEND_NAIVE=OFF

# build binaries for performance tests
./pyutils/driver.py -v -l $logfile build -b release -p $real_type -g $grid -o build -e $envfile -t perftests || { echo 'Build failed'; rm -rf $tmpdir; exit 1; }

for domain in 128 256; do
  # result directory, create if it does not exist yet
  resultdir=./build/pyutils/perftest/results/${label}_$env
  mkdir -p $resultdir
  result=$resultdir/$domain.json

  # run performance tests
  ./build/pyutils/driver.py -v -l $logfile perftest run -s $domain $domain 80 -o $result || { echo 'Running failed'; rm -rf $tmpdir; exit 1; }

  # find references for same configuration
  reference=./pyutils/perftest/references/${label}_$env/$domain.json
  # plot comparison of current result with references
  ./build/pyutils/driver.py -v -l $logfile perftest plot compare -i $reference $result -o compare-$domain.html || { echo 'Plotting failed'; rm -rf $tmpdir; exit 1; }
done
