#!/bin/sh

rm -rf dark/actions/*
rm -rf dark/objects/*

cp -r light/actions/*.svg dark/actions
cp -r light/objects/*.svg dark/objects

sed -i 's/d=/fill="#FFFFFF" d=/g' dark/{objects,actions}/*.svg
