
while :
do
	(inotifywait -r . -e modify)&
	make
	status=$?
	[ $status -eq 0 ] || $(wait && (inotifywait -r . -e modify)&)
	wait
done
