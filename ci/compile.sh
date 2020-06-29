#!/bin/bash

set -eo pipefail

rm -rf /kristall

git clone https://github.com/MasterQ32/kristall /kristall

cd /kristall

make

cp kristall /artifacts
