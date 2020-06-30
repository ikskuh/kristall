#!/bin/bash

set -eo pipefail

# First, do the docker build
docker-compose up

scp -i /home/ci/secret/kristall_ci output/Kristall-nightly-x86_64.AppImage kristall-ci@kristall.random-projects.net:.
