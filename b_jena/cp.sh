#!/bin/sh
DIR="`dirname "$0"`"
JENA_HOME=`ls -td $DIR/apache-jena-4* | head -1`
JENA_CP="$JENA_HOME/lib/*"

echo "$DIR:$CLASSPAH:$JENA_CP"
