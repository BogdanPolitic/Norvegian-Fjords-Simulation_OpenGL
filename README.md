# Norvegian-Fjords-Simulation_OpenGL

<h3> General description </h3>

An <span style="font-weight:bold">OpenGL 3D</span> simulation of a typical <h6>norvegian fjord</h6>. The rendered scene contains a part of a river network confluence along which a boat is sailing back and forth taking random paths. Some high, steep hills nearby are also included into the render scene.

![Demo](https://github.com/BogdanPolitic/Demos/blob/main/Fjords_short_0.gif?raw=true)

<h3> Implementation </h3>

The general point I was focused on while building the project is making use of the graphics <h6>geometry shader</h6> based on which the river and the hills are rendered. The river is a network of connected vertices uniformly split along bezier curves. Thus, each river "branch" is defined by a bezier curve. Each chain of hills is built using the same bezier curves, with the vertices offsetted further and elevated on the height axis. The number of samples of a bezier curve can be changed on 3-key and 4-key, resulting in smoother or sharper edges.
The geometry shader takes a matrix transform input and builds its own network of vertices and triangles. The matrix transforms the resulted network of triangles, moving it to the right place on the scene. The same textures are applied to each pair of triangles, the cause of the apparently homogenous objects.
Because on the older GPUs there is a cap of 256 vertices to be rendered by a single geometry shader instance, there's a different instance for each river and hill branch, each one given their specific bezier curve.

The boat is imported from an .obj file from a Blender export. The animation applied to it is given by the successive transform effects which translate and rotate the boat according to the flow of the bezier curves. Each time the confluence is reached, a random river branch is chosen to go on.
