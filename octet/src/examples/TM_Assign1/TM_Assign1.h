////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <iostream>

namespace octet {
  /// Scene containing a box with octet.
  class TM_Assign1 : public app {
    // scene for drawing box
    ref<visual_scene> app_scene;

	//physics members
	btBroadphaseInterface *broadphase;
	btDefaultCollisionConfiguration *collisionConfiguration;
	btCollisionDispatcher *collisionDispatcher;
	btSequentialImpulseConstraintSolver *collisionSolver;
	btDiscreteDynamicsWorld *dynamicsWorld;

	btCollisionShape *pBoxShape;
	btCollisionShape *pGroundShape;
	btRigidBody *boxRigidBody;
	btRigidBody *groundRigidBody;

  public:
    /// this is called when we construct the class before everything is initialised.
    TM_Assign1(int argc, char **argv) : app(argc, argv) {

		//initialize bullet physics
		broadphase = new btDbvtBroadphase();
		collisionConfiguration = new btDefaultCollisionConfiguration();
		collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
		collisionSolver = new btSequentialImpulseConstraintSolver();

		dynamicsWorld = new btDiscreteDynamicsWorld(collisionDispatcher, broadphase, collisionSolver, collisionConfiguration);
		btVector3 gravity = btVector3(0, -9.8f, 0);
		dynamicsWorld->setGravity(gravity);

		//define collision shapes for objects
		pGroundShape = new btStaticPlaneShape(btVector3(0, 1, 0), 1);
		pBoxShape = new btBoxShape(btVector3(1, 1, 1));

		//create rigidbodies
		btDefaultMotionState *groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, -1, 0)));
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(0, groundMotionState, pGroundShape, btVector3(0, 0, 0));
		groundRigidBody = new btRigidBody(groundRigidBodyCI);
		dynamicsWorld->addRigidBody(groundRigidBody);

		btDefaultMotionState *boxMotionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 50, 0)));
		btScalar boxMass = 1;
		btVector3 inertia(0, 0, 0);
		pBoxShape->calculateLocalInertia(boxMass, inertia);
		btRigidBody::btRigidBodyConstructionInfo boxRigidBodyCI(boxMass, boxMotionState, pBoxShape, inertia);
		boxRigidBody = new btRigidBody(boxRigidBodyCI);
		dynamicsWorld->addRigidBody(boxRigidBody);
    }

    /// this is called once OpenGL is initialized
    void app_init() {
      app_scene =  new visual_scene();
      app_scene->create_default_camera_and_lights();

      material *red = new material(vec4(1, 0, 0, 1));
      mesh_box *box = new mesh_box(vec3(1));
      scene_node *node = new scene_node();
      app_scene->add_child(node);
      app_scene->add_mesh_instance(new mesh_instance(node, box, red));
    }

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

		//compute physics. assume 30 fps.
		dynamicsWorld->stepSimulation(1.0f / 30, 10);

		//TODO: remove this and move box according to transform
		btTransform boxTransform;
		boxRigidBody->getMotionState()->getWorldTransform(boxTransform);
		float yPos = boxTransform.getOrigin().getY();
		printf("Box Y Pos: %f\n", yPos);


      int vx = 0, vy = 0;
      get_viewport_size(vx, vy);
      app_scene->begin_render(vx, vy);

      // update matrices. assume 30 fps.
      app_scene->update(1.0f/30);

      // draw the scene
      app_scene->render((float)vx / vy);
    }

	~TM_Assign1() {
		//physics cleanup
		dynamicsWorld->removeRigidBody(boxRigidBody);
		delete boxRigidBody->getMotionState();
		delete boxRigidBody;

		dynamicsWorld->removeRigidBody(groundRigidBody);
		delete groundRigidBody->getMotionState();
		delete groundRigidBody;

		delete pBoxShape;
		delete pGroundShape;

		delete dynamicsWorld;
		delete collisionSolver;
		delete collisionDispatcher;
		delete collisionConfiguration;
		delete broadphase;
	}
  };
}
