#!/bin/bash

grep -n -w $1 $2 | awk -F ':' '{print $1}' | xargs -I {} sh -c 'echo "{} $(tail -n +{} /path/to/source/file.cpp | grep -n -m 1 -w "}")"  | awk '{print $1,$3}' | xargs -L 1 sed -n
