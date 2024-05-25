#!/bin/bash

if [ $1 == 1 ]
then
./engine ../tests/test_files_phase_1/test_1_1.xml;
./engine ../tests/test_files_phase_1/test_1_2.xml;
./engine ../tests/test_files_phase_1/test_1_3.xml;
./engine ../tests/test_files_phase_1/test_1_4.xml;
./engine ../tests/test_files_phase_1/test_1_5.xml
elif [ $1 == 2 ]
then
./engine ../tests/test_files_phase_2/test_2_1.xml;
./engine ../tests/test_files_phase_2/test_2_2.xml;
./engine ../tests/test_files_phase_2/test_2_3.xml;
./engine ../tests/test_files_phase_2/test_2_4.xml;
./engine ../tests/test_files_phase_2/solar_system.xml
elif [ $1 == 4 ]
then
./engine ../tests/test_files_phase_4/test_4_1.xml;
./engine ../tests/test_files_phase_4/test_4_2.xml;
./engine ../tests/test_files_phase_4/test_4_3.xml;
./engine ../tests/test_files_phase_4/test_4_4.xml;
./engine ../tests/test_files_phase_4/test_4_5.xml;
./engine ../tests/test_files_phase_4/test_4_6.xml;
else
echo "Wrong option (available: 1, 2, 4)"
fi
