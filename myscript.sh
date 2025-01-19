echo hello
pwd
cd extras
sort *.txt | grep ra
ls test*file.txt
cd subextras
sort *.txt | grep la
sort file1.txt | grep ra < file2.txt
pwd
cd fakedir
cd ../..
false
then echo should not print
true
then echo should print
false
else echo should print
true
else echo should not print
sort mywords.txt > result.txt
grep hi < mywords.txt
sort mywords.txt | grep ra
ls extras/test*file.txt
ls *.c *.txt
then exit whoops leaving now
