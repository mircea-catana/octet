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

  public:
    /// this is called when we construct the class before everything is initialised.
    TM_Assign1(int argc, char **argv) : app(argc, argv) {
    }

    /// this is called once OpenGL is initialized
    void app_init() {
		app_scene =  new visual_scene();
		app_scene->create_default_camera_and_lights();
		dynamics_world = app_scene->get_bt_world();

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
		create_springs();
    }

    void create_bridge() {
        mat4t mtw;
        mtw.loadIdentity();
        mtw.translate(vec3(0, 0.5f, 0));
        mesh_instance *b1 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);

        mtw.loadIdentity();
        mtw.translate(vec3(1.6f, 1.25f, 0.0f));
        mesh_instance *p1 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 0, 1)), true);

        mtw.loadIdentity();
        mtw.translate(vec3(2.7f, 1.25f, 0.0f));
        mesh_instance *p2 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 1, 1)), true);

        mtw.loadIdentity();
        mtw.translate(vec3(3.8f, 1.25f, 0.0f));
        mesh_instance *p3 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 0, 1)), true);

        mtw.loadIdentity();
        mtw.translate(vec3(4.9f, 1.25f, 0.0f));
        mesh_instance *p4 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 1, 1)), true);

        mtw.loadIdentity();
        mtw.translate(vec3(6.5f, 0.5f, 0.0f));
        mesh_instance *b2 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);

        // hinges

        btHingeConstraint *c1 = new btHingeConstraint(*(b1->get_node()->get_rigid_body()), *(p1->get_node()->get_rigid_body()),
            btVector3(0.5f, 0.5f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
            btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        c1->setLimit(-PI * 0.1f, PI* 0.1f);
        dynamics_world->addConstraint(c1);

        btHingeConstraint *c2 = new btHingeConstraint(*(p1->get_node()->get_rigid_body()), *(p2->get_node()->get_rigid_body()),
            btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
            btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        c2->setLimit(-PI * 0.1f, PI* 0.1f);
        dynamics_world->addConstraint(c2);

        btHingeConstraint *c3 = new btHingeConstraint(*(p2->get_node()->get_rigid_body()), *(p3->get_node()->get_rigid_body()),
            btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
            btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        c3->setLimit(-PI * 0.1f, PI* 0.1f);
        dynamics_world->addConstraint(c3);

        btHingeConstraint *c4 = new btHingeConstraint(*(p3->get_node()->get_rigid_body()), *(p4->get_node()->get_rigid_body()),
            btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
            btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        c4->setLimit(-PI * 0.1f, PI* 0.1f);
        dynamics_world->addConstraint(c4);

        btHingeConstraint *c5 = new btHingeConstraint(*(p4->get_node()->get_rigid_body()), *(b2->get_node()->get_rigid_body()),
            btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.5f, 0.0f),
            btVector3(0, 0, 1), btVector3(0, 0, 1), false);
        c5->setLimit(-PI * 0.1f, PI* 0.1f);
        dynamics_world->addConstraint(c5);
    }

    void create_springs() {
        mat4t mtw;
        mtw.translate(-3, 10, 0);
        btRigidBody *rb1 = NULL;
        mesh_instance *mi1 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);
        rb1 = mi1->get_node()->get_rigid_body();

        mtw.loadIdentity();
        mtw.translate(-3, 8, 0);
        btRigidBody *rb2 = NULL;
        mesh_instance *mi2 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(0, 1, 0, 1)), true, 1.0f);
        rb2 = mi2->get_node()->get_rigid_body();

        btTransform frameInA, frameInB;
        frameInA = btTransform::getIdentity();
        frameInA.setOrigin(btVector3(btScalar(0.0f), btScalar(-0.5f), btScalar(0.0f)));
        frameInB = btTransform::getIdentity();
        frameInB.setOrigin(btVector3(btScalar(0.0f), btScalar(0.5f), btScalar(0.0f)));

        btGeneric6DofSpringConstraint *c1 = new btGeneric6DofSpringConstraint(*rb2, *rb1, frameInA, frameInB, true);
        c1->setLinearUpperLimit(btVector3(0.0f, 5.0f, 0.0f));
        c1->setLinearLowerLimit(btVector3(0.0f, -5.0f, 0.0f));

        c1->setAngularLowerLimit(btVector3(0.f, 0.0f, -1.5f));
        c1->setAngularUpperLimit(btVector3(0.f, 0.0f, 1.5f));

        dynamics_world->addConstraint(c1, true);

        c1->setDbgDrawSize(btScalar(5.f));
        c1->enableSpring(0, true);
        c1->setStiffness(0, 10.0f);
        c1->setDamping(0, 0.5f);
        //c1->setEquilibriumPoint();
    }

	plank add_plank(vec3 position, vec3 size, material *mat, btScalar mass, bool is_dynamic) {
		plank p;
		mat4t mtw;

		mtw.loadIdentity();	
		mtw.translate(position);
		p.init(mtw, size, mat, mass);

		dynamics_world->addRigidBody(p.get_rigidbody());
        app_scene->add_shape(mtw, p.get_mesh(), p.get_material(), is_dynamic);
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

		// draw the scene
		app_scene->render((float)vx / vy);
    }
  };
}
