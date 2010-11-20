#!/bin/sh

for I in assets/levels/*
do
  tail -n 1 $I | tr -d '\n' > $I 
done
