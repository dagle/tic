tic - Simple twitch monitor
===========================
tic is a simple twitch monitor, made to live in your systemtray.
tic is written in C and is intended to be portable, have a low footprint and few dependencies.
runnable on \*nix, OS X and Windows. It uses gtk as a crossplattform gui and hopefully 
shouldn't create to much problems for a small program like tic.

tic has the following dependencies:
	gtk
	curl 
	json-c

To install on linux, type make and then make install (with the correct permissions).

To configure, either use the gui or locate the config file (tic.conf) and edit it.

Since all development is done on linux, that is what the git-version is most likely tested on.

Faq:

1. Why isn't there any binaries for my plattform?
Because I haven't updated the buildsystem for your plattform
and I can't currently build it. The reason for that is that
tic it's not ready for usage or not ready for that pattform.

Since a screenshot can say more than 100 words (look in the opper courner):
![picture of tic in action](http://alephnull.se/software/tic-scrot.png)
