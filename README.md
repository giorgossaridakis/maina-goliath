# maina-goliath
Maina the repeater and Goliath the eraser

Maina is a repeater from stdin to file, a tee type that will *not* stop when the output file is not there anymore and will halt in input stream remains empty for a requested amount of seconds.
Goliath is a daemon that watches every requested intervals the size of a file. He will delete the file if it exceeds requested size and will abort if the file is not there.
I wrote these two to help with Unix symon. Piping telnet output to maina instead of tee will keep a log and Goliath running in the background will ensure this log doesn't exceed desired size. Unix tee stopped output when the file was erased after output start.
in Unix try ->
goliath -t30 -s512 symon.log
telnet 127.0.0.1 2100|maina -t10 symon.log
ps -aux|grep -e maina -e goliath
