<h1>Physics Demo<h1>

<p>This is a demo project for the first assignment in the class Tools and Middleware at MSc Computer Games and Entertainment at Goldsmiths University of London. The project had to present examples of using hinge constraints, spring constraints and collision detection by means of the Bullet Physics Library included with the Octet framework.</p>

<h2>Hinge Constraints</h2>
<p>In the scene you can see the hinge constraints (btHingeConstraint) in the representation of a plank bridge. There are 2 static cubes at each end of the bridge and between them there is a sequence of dynamic rigid bodies which are connected to the ends and between themselves through hinge constraints. This limits the movement of the dynamic objects giving the impression of a bridge where the planks you step on would be connected to the ropes attached to the end pillars.</p>

<h2>Spring Constraints</h2>
<p>For the spring constraints I have used btGeneric6DofSpringConstraint which is attached to 2 cubes in the scene. The first cube is static while the second is dynamic, by making use of the shooting capabilities implemented on the player object one can shoot small spheres at the dynamic object and observe the effects of the spring constraint.
Collision Detection
In order to detect collisions between specific objects in Bullet Physics, we first have to know the assigned user index to each of our object’s rigid bodies. Once we have these values from the discrete dynamics world we get the dispatcher and iterate through all the dispatcher’s manifolds. Each manifold holds the user indexes of the rigid bodies which are colliding, so if the indexes of the two bodies correspond to the assigned indexes we identified earlier then we know there is a collision between the two objects at that particular time. In order to demonstrate this, in the project a sound is played each time the player rigid body collides with the jukebox rigid body placed in the scene.</p>
