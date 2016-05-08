CPP = g++
INC = -I../glslutil -I../cryphutil
C_FLAGS = -fPIC -g -c -DGL_GLEXT_PROTOTYPES $(INC)

LINK = g++ -fPIC

OBJS = Controller.o GLFWController.o ModelView.o ModelView3D.o BasicShape.o BasicShapeModelView.o

libmvc.so: $(OBJS)
	$(LINK) -shared -o libmvc.so $(OBJS)
	cp libmvc.so ../lib/

Controller.o: Controller.h Controller.c++
	$(CPP) $(C_FLAGS) Controller.c++

GLFWController.o: GLFWController.h GLFWController.c++
	$(CPP) $(C_FLAGS) -DGLFW_INCLUDE_GLU GLFWController.c++

ModelView.o: ModelView.h ModelView.c++
	$(CPP) $(C_FLAGS) ModelView.c++

ModelView3D.o: ModelView3D.h ModelView3D.c++
	$(CPP) $(C_FLAGS) ModelView3D.c++

BasicShape.o: BasicShape.h BasicShape.c++
	$(CPP) $(C_FLAGS) BasicShape.c++

BasicShapeModelView.o: BasicShapeModelView.h BasicShapeModelView.c++
	$(CPP) $(C_FLAGS) BasicShapeModelView.c++
