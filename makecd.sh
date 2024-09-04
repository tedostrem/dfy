mkpsxiso -y CDLAYOUT.xml
sudo mount /dev/sdb1 /mnt/usb
sudo mkdir -p /mnt/usb/Code/$1
sudo cp dfy.cue dfy.iso /mnt/usb/Code/$1
sudo umount /mnt/usb
