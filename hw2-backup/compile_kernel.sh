#!/bin/bash

cd /usr/src/linux-2.4.18-14custom
dos2unix *.c Makefile
make bzImage 2>compile_error.log
#echo '---------------------------------' 2>> compile_error.log
#make modules 2>>compile_error.log
#echo '---------------------------------' 2>> compile_error.log
#make modules_install 2>>compile_error.log
clear
echo 'finished compiling. you may view possible errors.do you want to see them?(you can quit after with q)'
read continue_var
if [ $continue_var = 'y' ];then
less compile_error.log
fi


echo 'copy image to boot?'
read continue_var
if [ $continue_var = 'y' ];then


echo 'copies kernel image from arch/i386/boot to boot'
\cp /usr/src/linux-2.4.18-14custom/arch/i386/boot/bzImage /boot/vmlinuz-2.4.18-14custom
cd /boot
rm -f 2.4.18-14custom.img
mkinitrd 2.4.18-14custom.img 2.4.18-14custom

echo 'the new image is ready in boot'
fi
