CPP = g++
C_FLAGS = -fPIC -g -c -DGL_GLEXT_PROTOTYPES

LINK = g++ -fPIC

OBJS = ShaderIF.o

libglsl.so: $(OBJS)
	$(LINK) -shared $(GL_LIB_LOC) -o libglsl.so $(OBJS)
	cp libglsl.so ../lib/

ShaderIF.o: ShaderIF.h ShaderIF.c++
	$(CPP) $(C_FLAGS) ShaderIF.c++
