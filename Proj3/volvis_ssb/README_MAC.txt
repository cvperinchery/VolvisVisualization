As of Spring 2016, the latest Mac OS (10.11.3) only supports OpenGL 4.1. The
code here uses GL_SHADER_STORAGE_BUFFER (which requires at least OpenGL 4.3)
and glBufferStorage (which requires OpenGL 4.4).

A version using texture buffer objects that only requires OpenGL 4.1 is
in directory volvis_tbo.
