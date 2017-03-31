### simple implementation of ping 

#### why normal user can use ping, which use the RAW socket ? 

```
ls -al /bin/ping
-rwsr-xr-x 1 root root 44168  5月  8  2014 /bin/ping
```
because of  SUID authority.

#### how to get SUID authority of root, so that normal user can use ping ?

```
mkdir build && cd build
make -j
sudo chown root ./zy_ping
sudo chgrp root ./zy_ping
sudo chmod +s ./zy_ping
```