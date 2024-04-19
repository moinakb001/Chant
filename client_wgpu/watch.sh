
while :
do
	(inotifywait -r . -e modify)&
	make
	wait
done
