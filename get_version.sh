#! /bin/sh
./get_svn_revision.sh
echo "23.6.0.`cat svn_revision`" > version
#echo "23.6.0" > version
