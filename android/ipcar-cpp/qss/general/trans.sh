for src in $* ;
do
	tmp=$$


	if (test -a $arg); then 
		cp $src $src.bak
		convert $src -fill none -draw 'color 0,0 replace' $tmp
		convert -transparent "#000000" $tmp $src
		rm $tmp
	else
		echo "$arg does not exists";
	fi

	#convert image-off.1.png -fill none -fuzz 10% -draw 'color 0,0 replace' image-off.2.png
	#convert -transparent "#000000" image-off.2.png image-off.png
done
