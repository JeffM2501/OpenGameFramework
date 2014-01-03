OGF or libOGF
Open Game Framework

This library is built on top of the Simple Direct Media library, or libSDL.
You need it... get it at www.libsdl.org.

It also uses the OpenIL DevIL imaging library.
You need it too.. get it at http://www.imagelib.org
it may have other requirements

The thing makes a library and a sample
The library is libOGF on linux and macintosh and SGL.lib for windows.
The sample is ogf_sample on linux and the 'tosh, and sample.exe on windows.
For linux there is a makefile, it's not prety but it works. Just type make linux
For Mac there is a makefile, it's worse, and it may not work, try typing make mac and pray.
For Windows there is a Visual C++ .net 2003 ( vc 7.1 ) workspace that will build the lib, 
and one that will build the lib and the sample. If you have an older version of VC, then
sorry, you'll have to make due.

The lib does a lot of things;

	Create a window bound to an openGL context
	Handle perspective and matrix transofrms
	Provide simple cammera support.
	Manage openGL lights.
	Load up images in a number of formats and bind them as textures,
	  including; png, jpeg, bmp, tga, tif, psd, and many others.
	Provide OS indepenend File IO
	Classes for loading and saving prefrences/config files.
	Classes for managing command line options from any OS
	Wrap up the SDL event manager in a clean C++ game loop class
	Provide 3d math classes for vertex, vector and matrix calculations
	Provide 3d frustum clulling classes that can pull frustums from openGL
	Time classes
	Input management classes
	OpenGL Font rendering from image based fonts, with multiple input sizes, and smart blending.
	Animated mesh manger that can use Quake3, Unreal , or Wavefront OBJ models.
	Pannel manager for simplified menu GUI in openGL