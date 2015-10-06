////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "plank.h"

namespace octet {
  /// Scene containing a box with octet.
 class TM_Assign1 : public app {
   private:
	//constants
	const float PI = 3.14159;

	btDefaultCollisionConfiguration *configuration;
	btCollisionDispatcher *dispatcher;
	btDbvtBroadphase *broadphase;
	btSequentialImpulseConstraintSolver *solver;
	btDiscreteDynamicsWorld *dynamics_world;

    // scene for drawing box
    ref<visual_scene> app_scene;

	//camera & fps members
	mouse_look mouse_look_helper;
	ref<camera_instance> main_camera;

	helper_fps_controller fps_helper;
	ref<scene_node> player_node;

	//terrain 
	struct terrain_mesh_source : mesh_terrain::geometry_source {
		mesh::vertex vertex(vec3_in bb_min, vec3_in uv_min, vec3_in uv_delta, vec3_in pos) {
			vec3 p = bb_min + pos;
			vec3 uv = uv_min + vec3((float)pos.x(), (float)pos.z(), 0) * uv_delta;
			return mesh::vertex(p, vec3(0, 1, 0), uv);
		}
	};
	terrain_mesh_source terrain_source;

	//scene objects
	dynarray<scene_node*> nodes;
	dynarray<btRigidBody*> rigidbodies;

  public:
    /// this is called when we construct the class before everything is initialised.
    TM_Assign1(int argc, char **argv) : app(argc, argv) {
		configuration = new btDefaultCollisionConfiguration();
		dispatcher = new btCollisionDispatcher(configuration);
		broadphase = new btDbvtBroadphase();
		solver = new btSequentialImpulseConstraintSolver();
		dynamics_world = new btDiscreteDynamicsWorld(dispatcher, broadphase, solver, configuration);
    }

	~TM_Assign1() {
		delete dynamics_world;
		delete solver;
		delete broadphase;
		delete dispatcher;
		delete configuration;
	}

    /// this is called once OpenGL is initialized
    void app_init() {
		app_scene =  new visual_scene();
		app_scene->create_default_camera_and_lights();

		mouse_look_helper.init(this, 200.0f / 360, false);
		fps_helper.init(this);

		main_camera = app_scene->get_camera_instance(0);
		main_camera->get_node()->translate(vec3(0, 4, 0));
		main_camera->set_far_plane(10000);

		mat4t mat;
		mat.loadIdentity();

		mesh_instance *mi = app_scene->add_shape(
			mat,
			new mesh_terrain(vec3(100.0f, 0.5f, 100.0f), ivec3(100, 1, 100), terrain_source),
			new material(new image("assets/grass.jpg")),
			false, 0
			);
		btRigidBody *rb = mi->get_node()->get_rigid_body();
		dynamics_world->addRigidBody(rb);
		rigidbodies.push_back(rb);
		nodes.push_back(mi->get_node());

		float player_height = 1.8f;
		float player_radius = 0.25f;
		float player_mass = 90.0f;

		mat.loadIdentity();
		mat.translate(0.0f, player_height*6.0f, 50.0f);

		mesh_instance *mi2 = app_scene->add_shape(
			mat,
			new mesh_sphere(vec3(0), player_radius),
			new material(vec4(1, 0, 0, 1)),
			true, player_mass,
			new btCapsuleShape(0.25f, player_height)
			);
		player_node = mi2->get_node();

		create_bridge();
    }

	void create_bridge() {
		plank pred = add_plank(vec3(0), vec3(1), new material(vec4(1, 0, 0, 1)), 0.0f);
		plank pgreen = add_plank(vec3(1.5f, 2.0f, 0.0f), vec3(.5f, .25f, 1.0f), new material(vec4(0, 1, 0, 1)), 1.0f);
		btHingeConstraint *constraint = new btHingeConstraint((*pred.get_rigidbody()), (*pgreen.get_rigidbody()),
			btVector3(1.0f, 1.0f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
			btVector3(0, 0, 1), btVector3(0, 0, 1), false);
		constraint->setLimit(-PI * 0.1f, PI * 0.1f);
		dynamics_world->addConstraint(constraint);

		plank pgreen2 = add_plank(vec3(2.0f, 2.0f, 0.0f), vec3(.5f, .25f, 1.0f), new material(vec4(0, 1, 0, 1)), 1.0f);
		btHingeConstraint *constraint2 = new btHingeConstraint((*pgreen.get_rigidbody()), (*pgreen2.get_rigidbody()),
			btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
			btVector3(0, 0, 1), btVector3(0, 0, 1), false);
		constraint2->setLimit(-PI * 0.1f, PI * 0.1f);
		dynamics_world->addConstraint(constraint2);

		plank pgreen3 = add_plank(vec3(2.0f, 2.0f, 0.0f), vec3(.5f, .25f, 1.0f), new material(vec4(0, 1, 0, 1)), 1.0f);
		btHingeConstraint *constraint3 = new btHingeConstraint((*pgreen2.get_rigidbody()), (*pgreen3.get_rigidbody()),
			btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
			btVector3(0, 0, 1), btVector3(0, 0, 1), false);
		constraint3->setLimit(-PI * 0.1f, PI * 0.1f);
		dynamics_world->addConstraint(constraint3);

		plank pgreen4 = add_plank(vec3(2.0f, 2.0f, 0.0f), vec3(.5f, .25f, 1.0f), new material(vec4(0, 1, 0, 1)), 1.0f);
		btHingeConstraint *constraint4 = new btHingeConstraint((*pgreen3.get_rigidbody()), (*pgreen4.get_rigidbody()),
			btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
			btVector3(0, 0, 1), btVector3(0, 0, 1), false);
		constraint4->setLimit(-PI * 0.1f, PI * 0.1f);
		dynamics_world->addConstraint(constraint4);

		plank pred2 = add_plank(vec3(6, 0, 0), vec3(1), new material(vec4(1, 0, 0, 1)), 0.0f);
		btHingeConstraint *constraint5 = new btHingeConstraint((*pgreen4.get_rigidbody()), (*pred2.get_rigidbody()),
			btVector3(0.5f, 0.25f, 0.0f), btVector3(-1.0f, 1.0f, 0.0f),
			btVector3(0, 0, 1), btVector3(0, 0, 1), false);
		constraint5->setLimit(-PI * 0.1f, PI * 0.1f);
		dynamics_world->addConstraint(constraint5);
	}

	plank add_plank(vec3 position, vec3 size, material *mat, btScalar mass) {
		plank p;
		mat4t mtw;

		mtw.loadIdentity();
		mtw.translate(position);
		p.init(mtw, size, mat, mass);

		dynamics_world->addRigidBody(p.get_rigidbody());
		rigidbodies.push_back(p.get_rigidbody());
		nodes.push_back(p.get_scene_node());
		app_scene->add_mesh_instance(new mesh_instance(p.get_scene_node(), p.get_mesh(), p.get_material()));
		app_scene->add_child(p.get_scene_node());
		return p;
	}

    /// this is called to draw the world
    void draw_world(int x, int y, int w, int h) {

		int vx = 0, vy = 0;
		get_viewport_size(vx, vy);
		app_scene->begin_render(vx, vy);

		//update camera
		scene_node *camera_node = main_camera->get_node();
		mat4t &camera_to_world = camera_node->access_nodeToParent();
		mouse_look_helper.update(camera_to_world);

		fps_helper.update(player_node, camera_node);

		// update matrices. assume 30 fps.
		app_scene->update(1.0f / 30);

		// physics step
		dynamics_world->stepSimulation(1.0f / 30);

		// update mesh positions to physics
		for (int i = 0; i != rigidbodies.size(); ++i) {
			btRigidBody *rigidbody = rigidbodies[i];
			btQuaternion btq = rigidbody->getOrientation();
			btVector3 pos = rigidbody->getCenterOfMassPosition();
			quat q(btq[0], btq[1], btq[2], btq[3]);
			mat4t mtw = q;
			mtw[3] = vec4(pos[0], pos[1], pos[2], 1);
			nodes[i]->access_nodeToParent() = mtw;
		}

		// draw the scene
		app_scene->render((float)vx / vy);
    }
  };
}
