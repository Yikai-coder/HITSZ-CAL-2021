#! /bin/bash
test_program=(gcc astar zeusmp tonto)
pin_program=cacheModel.so
output_program=result_cmp_2.txt
make
# ../pin -t obj-intel64/cacheModel.so -- runspec --size=test --noreportable gcc
# ../pin -t obj-intel64/cacheModel.so -- runspec --size=test --noreportable astar
# ../pin -t obj-intel64/cacheModel.so -- runspec --size=test --noreportable zeusmp
# ../pin -t obj-intel64/cacheModel.so -- runspec --size=test --noreportable tonto
for program in ${test_program[*]}:
do
    echo ${program}: | tee -a ${output_program}
    ../pin -t obj-intel64/${pin_program} -- runspec --size=test --noreportable --nobuild --iteration=1 ${program} | tee -a ${output_program}
done