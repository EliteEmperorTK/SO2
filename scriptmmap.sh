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
echo -e "\x1B[38;2;17;245;120m$ ./modificarSb disco\x1b[0m"
./modificarSb disco
echo
echo -e "\x1B[38;2;17;245;120m$ ./leer_sf disco\x1b[0m"
./leer_sf disco
echo

make clean