chmod a+x ./debug.sh
make clean
make dep
sudo make
gdb bootimg

