#!/bin/bash

# DO NOT CHANGE these paths unless you're sure of what you're doing.
# The paths below should be coherent with the ones specified into 
# M9DSE Eplorer source code and matlab sources.

BASE_DIR=$HOME
WORKSPACE=$BASE_DIR/matlab-workspace
matlab_ROOT=$BASE_DIR/matlab
M9_SOURCE=$BASE_DIR/M9

# checking matlab installation ###############################
echo ""
echo "** Checking for matlab installation files..."

if [ -e $matlab_ROOT/scripts ]; then
   echo "  ** Ok, found a matlab 4 installation, copying new tcc script..."
   cp $M9_SOURCE/MISC_FILES/tcc $matlab_ROOT/scripts
else
   echo "  > Missing $matlab_ROOT/scripts directory !"
   echo "    check your matlab installation!"
   exit 1
fi


# checking M9DSE workspace ######################################
echo "** Checking for necessary files in $WORKSPACE..."

if [ -e $WORKSPACE/M9-explorer ]; then
   echo "  ** WARNING: found a previous $WORKSPACE/M9-explorer directory"
   echo "It's strongly suggested to move/rename it in order to avoid conflicts with "
   echo "newer versions."
   exit -1
else # missing workspace
   echo " > Creating $WORKSPACE/M9-explorer directory"
    mkdir -p $WORKSPACE/M9-explorer
fi

echo " > Copying SUBSPACES directory..."
cp -Rf $M9_SOURCE/SUBSPACES $WORKSPACE/M9-explorer

echo " > Copying m5elements configuration files..."
cp -Rf $M9_SOURCE/MISC_FILES/m5elements $WORKSPACE/M9-explorer/

echo " > Creating $WORKSPACE/M9-explorer/machines directory..."
cp -Rf $M9_SOURCE/MISC_FILES/machines $WORKSPACE/M9-explorer/

echo " > Creating $WORKSPACE/M9-explorer/step_by_step directory..."
mkdir -p $WORKSPACE/M9-explorer/step_by_step

cp -Rf $M9_SOURCE/MISC_FILES/machines $WORKSPACE/M9-explorer/step_by_step
cp -Rf $M9_SOURCE/MISC_FILES/m5elements $WORKSPACE/M9-explorer/step_by_step

echo " > Creating a new M9_default.conf"
cp -f $M9_SOURCE/MISC_FILES/M9_default.conf $WORKSPACE/M9-explorer

echo " ** Done. **"
echo " Just type ./M9DSE to start M9DSE explorer!"
echo ""

