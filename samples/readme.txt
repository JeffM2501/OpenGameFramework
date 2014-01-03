--- samples dir-----

Here are some samples of how to use the Open Game Framework


"simple" is just that a very simple sample that sets up a window with some options,
creates a 3d view, and draws a couple simple things.

"objetWorld" is a bit more complex, allowing first person shooter type movement,
throughout a 3d world made up of objects. This sample uses a STML map file.

The data dir contains all the data for the samples.

All platforms will build the final executables in the samples dir, so it has access
to the data dir when runing. Make sure that the data dir is your working dir when you
run the samples.

Windows users---
you will want to ether put the DEVIL and SDL DLLs in this dir, or in your windows\system32 dir so that the samples can run