glrender:
	g++ /home/david/ColumbiaCS/Graphics-CS4160/glfw-3.2.1/deps/glad.c initshader.cc hello_triangle_lit.cc -I/home/david/ColumbiaCS/Graphics-CS4160/glfw-3.2.1/include/ -I/home/david/ColumbiaCS/Graphics-CS4160/glfw-3.2.1/deps -L/usr/lib/x86_64-linux-gnu/ -L/home/david/ColumbiaCS/Graphics-CS4160/glfw-3.2.1/lib/ -L/home/david/ColumbiaCS/Graphics-CS4160/glfw-3.2.1/deps -L/home/david/ColumbiaCS/Graphics-CS4160/glew-2.0.0/lib -Wl,-rpath,/home/david/ColumbiaCS/Graphics-CS4160/glew-2.0.0/lib -Wl,--no-as-needed -lglfw3 -lGLU -lGL -lGLEW -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -lm -std=c++11 -o glrender

.PHONY: clean
clean:
	rm -f *.o a.out glrender

.PHONY: all
all: clean glrender 
