Welcome to Handmade Hero!

If you are new to the series, the royal we strongly recommend watching
at least the first episode of Handmade Hero before trying to build the
source code, just so that you are familiar with the build
methodology.  You can find it here:

https://hero.handmade.network/episode/code/day001

If you'd rather go commando, you can try it without the video:

1) First, unpack a handmade_hero_day_???_source.zip AND the
handmade_hero_day_???_art.zip into the same directory.  We
recommend w:\handmade.  The art is updated less frequently
than the code, so you just need the most recent art ZIP that
has a _lesser or equal_ day number than the code ZIP.

Once you've unzipped both zips, the directory should contain at least:

w:\handmade\code - the source code
w:\handmade\sources - the source assets (pngs, wavs, etc.)
w:\handmade\tags - text listings of how to import and pack assets
w:\handmade\misc - other stuff

and after you build for the first time, these directories will
be created for the executables and packed assets:

w:\handmade\data - where the packed, processed assets go
w:\build - where the build results go

2) To build the game, you have to have Visual Studio installed.
You can get it for free from Microsoft here:

https://www.visualstudio.com/en-us/products/visual-studio-community-vs.aspx

Once you have installed it, you will need to start a command shell and
run "vcvarsall.bat x64" to set up the paths and environment variables
for building C programs from the command line.

You can then build Handmade Hero at any time by going to
w:\handmade\code and running the build.bat there.  This will build
the game into w:\build.

3) To run the game, you must be in the w:\handmade directory, and
you should run the w:\build\win32_handmade.exe executable.  If you
would like to run it from the Visual Studio debugger, you must load
win32_handmade.exe in the debugger and then use the Solution Explorer
to set the working directory to w:\handmade.

When the game first runs, it will process and pack all the art assets,
which takes a long time.  After it has done this once, the game will
only have to process art assets that have changed, which should be
quick.

4) If you have problems or questions, head over to the forums and
post them:

https://hero.handmade.network/forums

Happy coding!
- Casey
