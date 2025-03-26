Week 1:
Started repo. Spent 6-8 hours studying material on EnvironmentMapping, started learning about opengl and looking for what libraries I can be using to make the implementation.


Week 2+3:
started to implement the basic things, as The initial step that i did was establishing the environment itself.
i started With the cube map approach which is achieved by drawing a large cube (skybox) around the scene - i uploaded images files for skybox environment.
Each side of the cube contains a texture that represents the corresponding view of the world.

Week 3+4:

Started to implement the environment mapping algorithm by loading
6 images as GL_TEXTURE_CUBE_MAP texture target - 2 hours.
 
Rendering environment reflections on objects is done in shader.frag
by constructing a ray from camera position towards surface position,
then reflecting this ray using surface normal, and finally sampling
the environment color from the cubemap texture - 1 hour.
 
The reflections were there, but cube sides appeared in the wrong order.
To better see the reflections, added a reference sphere using a call to
gui::Renderer::I->drawSphere(). After several unsuccessful attempts,
the proper order was found by looking up cubemap constants in glad.h
(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X, etc.).
Finally, all sides of the cube match each other at the edges
and reflections on objects correspond to the skybox - 5 hours.
 
Solved various issues like properly controlling cubemap application
on different objects, unbinding cubemap texture after loading to avoid
OpenGL errors, removing shading from the skybox, etc. - 3 hours.

Week 5+6:
 
Started to combine environment color and fragment color.
Researched various sources - there is no established approach.
Experimented with different ways to combine colors.
Implemented options:
1) colors are added - the results are too bright.
2) colors are multiplied - the results are too dark.
3) colors averaged - the resulting contrast is low.
4) colors are mixed based on the fragment's brightness - looks best.
8 hours total.
 
Studied imgui_demo.cpp for examples of radio buttons, indentation, etc.
Added interface elements to the program's menu to control graphics settings.
Reorganized uniform flags to correctly work for all user choices.
4 hours total.
