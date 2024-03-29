#!/bin/bash

if [ $1 == 1 ]
then
./generator plane 2 3 ../3d/plane_2_3.3d &&
        echo "./generator plane 2 3 ../3d/plane_2_3.3d";
./generator box 2 3 ../3d/box_2_3.3d &&
        echo "./generator box 2 3 ../3d/box_2_3.3d";
./generator cone 1 2 4 3 ../3d/cone_1_2_4_3.3d &&
        echo "./generator cone 1 2 4 3 ../3d/cone_1_2_4_3.3d";
./generator sphere 1 10 10 ../3d/sphere_1_10_10.3d &&
        echo "./generator sphere 1 10 10 ../3d/sphere_1_10_10.3d"
elif [ $1 == 2 ]
then
./generator box 2 3 ../3d/box_2_3.3d &&
        echo "./generator box 2 3 ../3d/box_2_3.3d";
./generator cone 1 2 4 3 ../3d/cone_1_2_4_3.3d &&
        echo "./generator cone 1 2 4 3 ../3d/cone_1_2_4_3.3d";
./generator sphere 1 8 8 ../3d/sphere_1_8_8.3d &&
        echo "./generator sphere 1 8 8 ../3d/sphere_1_8_8.3d"
else 
echo "Wrong option (available: 1, 2)"
fi
