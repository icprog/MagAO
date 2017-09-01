#/bin/bash
echo -e "write $ADOPT_SIDE.LAB.MODE i 1\nquit" > /tmp/labmode
thrdtest < /tmp/labmode
