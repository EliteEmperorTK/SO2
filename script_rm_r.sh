clear
make clean
make
echo -e "\x1B[38;2;17;245;120m######################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkfs disco 100000\x1b[0m"
./mi_mkfs disco 100000
echo
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo
echo -e "\x1B[38;2;17;245;120m######################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/\x1b[0m"  
./mi_mkdir disco 6 /dir1/   
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/dir11/\x1b[0m"
./mi_mkdir disco 6 /dir1/dir11/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_mkdir disco 6 /dir1/dir11/dir12/\x1b[0m"
./mi_mkdir disco 6 /dir1/dir11/dir12/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/dir11/dir12/fic121\x1b[0m"
./mi_touch disco 6 /dir1/dir11/dir12/fic121 
echo 
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/dir11/dir12/fic122\x1b[0m"
./mi_touch disco 6 /dir1/dir11/dir12/fic122  
echo  
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/dir11/fic111\x1b[0m"
./mi_touch disco 6 /dir1/dir11/fic111  
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_touch disco 6 /dir1/fic11\x1b[0m"
./mi_touch disco 6 /dir1/fic11  
echo
echo -e "\x1B[38;2;17;245;120m"
echo "                         /                  "
echo "                         |                  "
echo "                       dir1                 "                  
echo "                       /    \               "           
echo "                  fic11      \              "               
echo "                             dir11 "           
echo "                  fic111____/      |        "
echo "                                 dir12      "
echo "                                /      \    "
echo "                            fic121    fic122  "
echo -e "\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco / \x1b[0m"
./mi_ls disco /
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco /dir1/ \x1b[0m"
./mi_ls disco /dir1/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco /dir1/dir11/ \x1b[0m"
./mi_ls disco /dir1/dir11/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco /dir1/dir11/dir12/ \x1b[0m"
./mi_ls disco /dir1/dir11/dir12/
echo
echo -e "\x1B[38;2;17;245;120m######################################################################\x1b[0m"
echo -e "\x1B[38;2;17;245;120m$ ./mi_rm_r disco /dir1/ \x1b[0m"
./mi_rm_r disco /dir1/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco /dir1/ \x1b[0m"
echo -e "\x1B[38;2;17;245;120m#Error: El directorio /dir1/ no existe\x1b[0m"
./mi_ls disco /dir1/
echo
echo -e "\x1B[38;2;17;245;120m$ ./mi_ls disco / \x1b[0m"
./mi_ls disco /
echo
make clean