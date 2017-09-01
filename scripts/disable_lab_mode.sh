#/bin/bash
echo -e "write L.LAB.MODE i 0\nquit" > /tmp/labmode
thrdtest < /tmp/labmode
