#!/bin/sh

RCFILES=("/etc/init.d/cups restart" \
	 "/etc/init.d/cupsd restart" \
	 "/etc/software/init.d/cups restart" \
	 "/etc/init.d/cupsys restart" \
	 "/etc/init.d/cupsys reload" \
	 "/etc/rc.d/rc.cups restart" )

OK=0
for FILE in "${RCFILES[@]}"
do
	if [ -e `echo $FILE | cut -d ' ' -f 1 -` ]; then
		echo "Trying $FILE..."
		$FILE
		OK=1
	fi
done

if [ $OK == 0 ]; then
	echo "Trying killall -HUP cupsd..."
	killall -HUP cupsd
fi

echo "Waiting 5 seconds..."
sleep 5
