////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

namespace octet {
  /// Scene containing a box with octet.
 class TM_Assign1 : public app {
   private:
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

		app_scene->add_shape(
			mat,
			new mesh_terrain(vec3(100.0f, 0.5f, 100.0f), ivec3(100, 1, 100), terrain_source),
			new material(new image("assets/grass.jpg")),
			false, 0
			);

		float player_height = 1.83f;
		float player_radius = 0.25f;
		float player_mass = 90.0f;

		mat.loadIdentity();
		mat.translate(0, player_height*6.0f, 50);

		mesh_instance *mi = app_scene->add_shape(
			mat,
			new mesh_sphere(vec3(0), player_radius),
			new material(vec4(1, 0, 0, 1)),
			true, player_mass,
			new btCapsuleShape(0.25f, player_height)
			);
		player_node = mi->get_node();

		mat.loadIdentity();
		mat.translate(0, 0.5f, 0);
		app_scene->add_shape(mat, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);

		mat.loadIdentity();
		mat.translate(1.5, 1.25, 0);
		app_scene->add_shape(mat, new mesh_box(vec3(.5f, .25f, 1)), new material(vec4(0, 1, 0, 1)), true);
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
		app_scene->update(1.0f/30);

		// draw the scene
		app_scene->render((float)vx / vy);
    }
  };
}
