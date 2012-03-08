#!/bin/bash

function conv3htm {
	echo $1 | sed -re 's/^(.*\.[0-9]+)$/\1.html/'
}

for manPage in `find . -type f`; do
	if [ "$manPage" == "$0" ] || [ "$manPage" == "./.gitignore" ]; then
		continue;
	fi
	htmPage=`conv3htm $manPage`
	cat $manPage | man2html -rp > $htmPage
done

for manLink in `find . -type l`; do
	if [ "$manLink" == "$0" ] || [ "$manPage" == "./.gitignore" ]; then
		continue;
	fi
	htmTarget=`readlink $manLink`
	htmTarget=`conv3htm $htmTarget`
	htmLink=`conv3htm $manLink`
	ln -fs $htmTarget $htmLink
done
