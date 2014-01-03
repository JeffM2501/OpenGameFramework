g++ = g++ -Wall -ansi
incdir = inc/

sampledir = samples/

simpleDir = simple/
objecWorldDir = objectWorld/

base_compile_options = -I$(incdir) `sdl-config --cflags`
compile_options = $(base_compile_options) -c 
link_options = -lGL -lGLU `sdl-config --libs`
aditional_libs = -lOGF -lIL -lILU -lILUT

simple_compile_options = -I$(incdir) `sdl-config --cflags` -c


include src/Makefile
include $(sampledir)$(simpleDir)Makefile
include $(sampledir)$(objecWorldDir)Makefile

libOGFSources = $(patsubst %.cpp,src/%.cpp,$(OGFSources))
libOGFObjects = $(patsubst %.cpp,%.o,$(libOGFSources))

libOGFSourcesExtended = $(patsubst %.cpp,src/%.cpp,$(OGFSources_extended))
libOGFObjectsExtended = $(patsubst %.cpp,%.o,$(libOGFSourcesExtended))

OGFSample_Simple_SourceFiles = $(patsubst %.cpp,$(sampledir)$(simpleDir)%.cpp,$(OGFSample_Simple_Sources))
OGFSample_ObjectWorld_SourceFiles = $(patsubst %.cpp,$(sampledir)$(objecWorldDir)%.cpp,$(OGFSample_ObjectWorld_Sources))

all:

	@echo Please use \'make linux\' or \'make mac\' for a full version, or for a non GL version use \'make lite\'

clean:

	@echo Cleaning up...
	@rm src/*.o
	@echo Done.

distclean:

	@echo distcleanin up...

	@rm src/*.o
	@rm libOGF.a
	@rm sample/simple
	@rm sample/objectWorld

	@echo Done.

linux:

	@echo
	@echo  		OGF Linux Build    
	@echo
	@echo		Building library
	@echo		Compiling

	$(g++) $(libOGFSources) $(libOGFSourcesExtended) $(compile_options)

	cp *.o src/
	rm *.o

	@echo				Building static lib
	ar rs libOGF.a $(libOGFObjects) $(libOGFObjectsExtended)
	@echo      OGF has been build, have fun
	@echo 

	@echo 	   Building simple sample
	@echo 
	$(g++) $(OGFSample_Simple_SourceFiles) -o samples/Simple $(aditional_libs) -L./ $(base_compile_options) -I$(sampledir)$(simpleDir) $(link_options)


	@echo 	   Building objectWorld sample
	@echo 
	$(g++) $(OGFSample_ObjectWorld_SourceFiles) -o samples/ObjectWorld $(aditional_libs) -L./ $(base_compile_options) -I$(sampledir)$(objecWorldDir) $(link_options)

	@echo
	@echo
	@echo 	   Samples have been built in the samples dir

mac:

	@echo
	@echo  		   OGF OSX Build    
	@echo
	@echo				Building library
	@echo				Compiling

	$(g++) $(libOGFSources) $(libOGFSourcesExtended) -framework openGL $(compile_options)

	cp *.o src/
	rm *.o

	@echo				Building static lib
	ar rs libOGF.a $(libOGFObjects) $(libOGFObjectsExtended)

	@echo      OGF has been build, have fun
	@echo 

	@echo 	   Building simple sample
	@echo 
	$(g++) $(OGFSample_Simple_SourceFiles) -o samples/Simple -framework openGL -framework IL $(aditional_libs) -L./ $(base_compile_options) -I$(sampledir)$(simpleDir) $(link_options)

	@echo 	   Building objectWorld sample
	@echo 
	$(g++) $(OGFSample_ObjectWorld_SourceFiles) -o samples/ObjectWorld -framework openGL -framework IL $(aditional_libs) -L./ $(base_compile_options) -I$(sampledir)$(objecWorldDir) $(link_options)

	@echo
	@echo
	@echo 	   Samples have been built in the samples dir

lite:

	@echo
	@echo  		OGF Lite Build    
	@echo
	@echo		Cleaning up...

	@rm src/*.o

	@echo		Building library
	@echo		Compiling


	$(g++) $(libOGFSources) $(simple_compile_options) -DOGF_LITE

	cp *.o src/
	rm *.o

	@echo				Building static lib
	ar rs libOGFLITE.a $(libOGFObjects)
	@echo		OGFLite has been build, have fun
	@echo 
