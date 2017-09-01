#
# This procedure tests the creation of many clients in a short time
# It is intended to be used together with the macro files: testmany.p,
# testone.p and testtwo.p

# Usage:   source testmany.sh

# The purpose of this all is to generate an heavy load to MsgD-RTDB
# The prcedure needs around 2 minutes and 30 secsto complete.

# This shell script launches some 190 clients connecting to MsgD.
# Clients are divided into two groups. Group one clients (names ranging from
# TESTMANY101 to TESTMANY195) execute the procedure testone.p.
# Group two clients (names ranging from TESTMANY201 to TESTMANY295) execute
# the procedure testtwo.p. See procedure files for documentation on commands.
#
# Between the two groups the client PRODC-01 is launched : it simulates a producer
# of data into shared buffer.

# At the end the procedure also launches a client (name TESTMANY00) executing procedure
# testmany.p
#
# In order to evaluate the functioning it is suggested to start msgdrtdb in verbode mode (-v -v)
#
# All clients will register debug output in files /tmp/TESTMANYxxx.log
# 

#VERBOSE="-d 0"   # Set verbosity of all test processes
#MSGD="-m 127.0.0.1"
MSGD=""
DELAY=0.2

./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY101 > /tmp/TESTMANY101.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY102 > /tmp/TESTMANY102.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY103 > /tmp/TESTMANY103.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY104 > /tmp/TESTMANY104.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY105 > /tmp/TESTMANY105.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY106 > /tmp/TESTMANY106.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY107 > /tmp/TESTMANY107.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY108 > /tmp/TESTMANY108.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY109 > /tmp/TESTMANY109.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY110 > /tmp/TESTMANY110.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY111 > /tmp/TESTMANY111.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY112 > /tmp/TESTMANY112.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY113 > /tmp/TESTMANY113.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY114 > /tmp/TESTMANY114.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY115 > /tmp/TESTMANY115.log &
sleep $DELAY
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY116 > /tmp/TESTMANY116.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY117 > /tmp/TESTMANY117.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY118 > /tmp/TESTMANY118.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY119 > /tmp/TESTMANY119.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY120 > /tmp/TESTMANY120.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY121 > /tmp/TESTMANY121.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY122 > /tmp/TESTMANY122.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY123 > /tmp/TESTMANY123.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY124 > /tmp/TESTMANY124.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY125 > /tmp/TESTMANY125.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY126 > /tmp/TESTMANY126.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY127 > /tmp/TESTMANY127.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY128 > /tmp/TESTMANY128.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY129 > /tmp/TESTMANY129.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY130 > /tmp/TESTMANY130.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY131 > /tmp/TESTMANY131.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY132 > /tmp/TESTMANY132.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY133 > /tmp/TESTMANY133.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY134 > /tmp/TESTMANY134.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY135 > /tmp/TESTMANY135.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY136 > /tmp/TESTMANY136.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY137 > /tmp/TESTMANY137.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY138 > /tmp/TESTMANY138.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY139 > /tmp/TESTMANY139.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY140 > /tmp/TESTMANY140.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY141 > /tmp/TESTMANY141.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY142 > /tmp/TESTMANY142.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY143 > /tmp/TESTMANY143.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY144 > /tmp/TESTMANY144.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY145 > /tmp/TESTMANY145.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY146 > /tmp/TESTMANY146.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY147 > /tmp/TESTMANY147.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY148 > /tmp/TESTMANY148.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY149 > /tmp/TESTMANY149.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY150 > /tmp/TESTMANY150.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY151 > /tmp/TESTMANY151.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY152 > /tmp/TESTMANY152.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY153 > /tmp/TESTMANY153.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY154 > /tmp/TESTMANY154.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY155 > /tmp/TESTMANY155.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY156 > /tmp/TESTMANY156.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY157 > /tmp/TESTMANY157.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY158 > /tmp/TESTMANY158.log &
sleep $DELAY
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY159 > /tmp/TESTMANY159.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY160 > /tmp/TESTMANY160.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY161 > /tmp/TESTMANY161.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY162 > /tmp/TESTMANY162.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY163 > /tmp/TESTMANY163.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY164 > /tmp/TESTMANY164.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY165 > /tmp/TESTMANY165.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY166 > /tmp/TESTMANY166.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY167 > /tmp/TESTMANY167.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY168 > /tmp/TESTMANY168.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY169 > /tmp/TESTMANY169.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY170 > /tmp/TESTMANY170.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY171 > /tmp/TESTMANY171.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY172 > /tmp/TESTMANY172.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY173 > /tmp/TESTMANY173.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY174 > /tmp/TESTMANY174.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY175 > /tmp/TESTMANY175.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY176 > /tmp/TESTMANY176.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY177 > /tmp/TESTMANY177.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY178 > /tmp/TESTMANY178.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY179 > /tmp/TESTMANY179.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY180 > /tmp/TESTMANY180.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY181 > /tmp/TESTMANY181.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY182 > /tmp/TESTMANY182.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY183 > /tmp/TESTMANY183.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY184 > /tmp/TESTMANY184.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY185 > /tmp/TESTMANY185.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY186 > /tmp/TESTMANY186.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY187 > /tmp/TESTMANY187.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY188 > /tmp/TESTMANY188.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY189 > /tmp/TESTMANY189.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY190 > /tmp/TESTMANY190.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY191 > /tmp/TESTMANY191.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY192 > /tmp/TESTMANY192.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY193 > /tmp/TESTMANY193.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY194 > /tmp/TESTMANY194.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testone TESTMANY195 > /tmp/TESTMANY195.log &
sleep $DELAY

./producer -v -c 100 -d 1 1 pluto > /tmp/PRODC-01.log &

sleep $DELAY

./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY201 > /tmp/TESTMANY201.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY202 > /tmp/TESTMANY202.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY203 > /tmp/TESTMANY203.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY204 > /tmp/TESTMANY204.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY205 > /tmp/TESTMANY205.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY206 > /tmp/TESTMANY206.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY207 > /tmp/TESTMANY207.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY208 > /tmp/TESTMANY208.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY209 > /tmp/TESTMANY209.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY210 > /tmp/TESTMANY210.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY211 > /tmp/TESTMANY211.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY212 > /tmp/TESTMANY212.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY213 > /tmp/TESTMANY213.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY214 > /tmp/TESTMANY214.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY215 > /tmp/TESTMANY215.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY216 > /tmp/TESTMANY216.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY217 > /tmp/TESTMANY217.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY218 > /tmp/TESTMANY218.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY219 > /tmp/TESTMANY219.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY220 > /tmp/TESTMANY220.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY221 > /tmp/TESTMANY221.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY222 > /tmp/TESTMANY222.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY223 > /tmp/TESTMANY223.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY224 > /tmp/TESTMANY224.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY225 > /tmp/TESTMANY225.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY226 > /tmp/TESTMANY226.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY227 > /tmp/TESTMANY227.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY228 > /tmp/TESTMANY228.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY229 > /tmp/TESTMANY229.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY230 > /tmp/TESTMANY230.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY231 > /tmp/TESTMANY231.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY232 > /tmp/TESTMANY232.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY233 > /tmp/TESTMANY233.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY234 > /tmp/TESTMANY234.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY235 > /tmp/TESTMANY235.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY236 > /tmp/TESTMANY236.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY237 > /tmp/TESTMANY237.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY238 > /tmp/TESTMANY238.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY239 > /tmp/TESTMANY239.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY240 > /tmp/TESTMANY240.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY241 > /tmp/TESTMANY241.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY242 > /tmp/TESTMANY242.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY243 > /tmp/TESTMANY243.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY244 > /tmp/TESTMANY244.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY245 > /tmp/TESTMANY245.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY246 > /tmp/TESTMANY246.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY247 > /tmp/TESTMANY247.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY248 > /tmp/TESTMANY248.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY249 > /tmp/TESTMANY249.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY250 > /tmp/TESTMANY250.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY251 > /tmp/TESTMANY251.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY252 > /tmp/TESTMANY252.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY253 > /tmp/TESTMANY253.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY254 > /tmp/TESTMANY254.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY255 > /tmp/TESTMANY255.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY256 > /tmp/TESTMANY256.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY257 > /tmp/TESTMANY257.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY258 > /tmp/TESTMANY258.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY259 > /tmp/TESTMANY259.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY260 > /tmp/TESTMANY260.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY261 > /tmp/TESTMANY261.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY262 > /tmp/TESTMANY262.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY263 > /tmp/TESTMANY263.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY264 > /tmp/TESTMANY264.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY265 > /tmp/TESTMANY265.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY266 > /tmp/TESTMANY266.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY267 > /tmp/TESTMANY267.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY268 > /tmp/TESTMANY268.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY269 > /tmp/TESTMANY269.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY270 > /tmp/TESTMANY270.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY271 > /tmp/TESTMANY271.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY272 > /tmp/TESTMANY272.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY273 > /tmp/TESTMANY273.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY274 > /tmp/TESTMANY274.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY275 > /tmp/TESTMANY275.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY276 > /tmp/TESTMANY276.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY277 > /tmp/TESTMANY277.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY278 > /tmp/TESTMANY278.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY279 > /tmp/TESTMANY279.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY280 > /tmp/TESTMANY280.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY281 > /tmp/TESTMANY281.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY282 > /tmp/TESTMANY282.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY283 > /tmp/TESTMANY283.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY284 > /tmp/TESTMANY284.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY285 > /tmp/TESTMANY285.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY286 > /tmp/TESTMANY286.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY287 > /tmp/TESTMANY287.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY288 > /tmp/TESTMANY288.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY289 > /tmp/TESTMANY289.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY290 > /tmp/TESTMANY290.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY291 > /tmp/TESTMANY291.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY292 > /tmp/TESTMANY292.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY293 > /tmp/TESTMANY293.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY294 > /tmp/TESTMANY294.log &
sleep $DELAY
./thrdtest -s $VERBOSE $MSGD -e testtwo TESTMANY295 > /tmp/TESTMANY295.log &
sleep $DELAY

sleep 2

./thrdtest $MSGD -e testmany TESTMANY00
grep established /tmp/TESTMANY*.log
