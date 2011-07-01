#! /bin/bash

if [ $# -lt 1 ]
then
    echo "Usage: $0 version [destination_dir]" 
    exit 0
fi

VERSION=$1

tar czf VerbalizerSoftware_$VERSION.tgz arduino docs -C Verbalizer/bin/ Verbalizer.app

if [ $2 ] 
then
    mv VerbalizerSoftware_$VERSION.tgz $2/
fi
