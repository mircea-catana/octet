////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "level_generator.h"

namespace octet {

	class sprite {
		// where is our sprite (overkill for a 2D game!)
		mat4t modelToWorld;

		float spriteRotation = 0.0f;

		// half the width of the sprite
		float halfWidth;

		// half the height of the sprite
		float halfHeight;

		// what texture is on our sprite
		int texture;

		// true if this sprite is enabled.
		bool enabled;

	public:

		enum sprite_type {
			character_sprite,
			enemy_sprite,
			wall_sprite,
			pickup_sprite,
			missile_sprite,
			other_sprite
		} type;

		sprite() {
			texture = 0;
			enabled = true;
		}

		void init(int _texture, float x, float y, float w, float h, sprite_type t = other_sprite) {
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			halfWidth = w * 0.5f;
			halfHeight = h * 0.5f;
			texture = _texture;
			enabled = true;
			type = t;
		}

		void render(texture_shader &shader, mat4t &cameraToWorld) {
			// invisible sprite... used for gameplay.
			if (!texture) return;

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// use "old skool" rendering
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			shader.render(modelToProjection, 0);

			// this is an array of the positions of the corners of the sprite in 3D
			// a straight "float" here means this array is being generated here at runtime.
			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth, halfHeight, 0,
				-halfWidth, halfHeight, 0,
			};

			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			// this is an array of the positions of the corners of the texture in 2D
			static const float uvs[] = {
				0, 0,
				1, 0,
				1, 1,
				0, 1,
			};

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			// finally, draw the sprite (4 vertices)
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}

		// move the object
		void translate(float x, float y) {
			modelToWorld.translate(x, y, 0);
		}

		// rotate the object
		void rotate(float angle) {
			modelToWorld.rotate(angle, 0, 0, 1);
		}

		// rotate the sprite to point towards the given position
		void look_at(float x, float y) {

			float ag = spriteRotation;

			float dx = x;
			float dy = y;
			float angle = atan2(dy, dx) * -180.0f / 3.14159265f;
			float finalAngle = angle - ag;

			if (finalAngle < 1 && finalAngle > -1)
				return;

			modelToWorld.rotateZ(finalAngle-90);
			spriteRotation = angle-90;
		}

		// position the object relative to another.
		void set_relative(sprite &rhs, float x, float y) {
			modelToWorld = rhs.modelToWorld;
			modelToWorld.translate(x, y, 0);
		}

		// return true if this sprite collides with another.
		// note the "const"s which say we do not modify either sprite
		bool collides_with(const sprite &rhs) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
			float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

			// both distances have to be under the sum of the halfwidths
			// for a collision
			return
				(fabsf(dx) < halfWidth + rhs.halfWidth) &&
				(fabsf(dy) < halfHeight + rhs.halfHeight)
				;
		}

		bool is_above(const sprite &rhs, float margin) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

			return
				(fabsf(dx) < halfWidth + margin)
				;
		}

		bool &is_enabled() {
			return enabled;
		}

		vec2 get_position() {
			return vec2(modelToWorld[3][0], modelToWorld[3][1]);
		}

		sprite_type get_type() {
			return type;
		}
	};

  /// Scene containing a box with octet.
  class IP_Assign1 : public app {
	  // Matrix to transform points in our camera space to the world.
	  // This lets us move our camera
	  mat4t cameraToWorld;

	  // shader to draw a textured triangle
	  texture_shader texture_shader_;

	  enum {
		  num_sound_sources = 8,
		  num_rows = 5,
		  num_cols = 10,
		  num_missiles = 2,

		  // sprite definitions
		  ship_sprite = 0,
		  game_over_sprite,

		  first_missile_sprite,
		  last_missile_sprite = first_missile_sprite + num_missiles - 1,

		  num_sprites,
	  };

	  // timers for missiles and bombs
	  int missiles_disabled;

	  // accounting for bad guys
	  int num_lives;

	  // game state
	  bool game_over;
	  int score;

	  // big array of sprites
	  sprite sprites[num_sprites];
	  dynarray<sprite> walls;

	  // random number generator
	  class random randomizer;

	  // a texture for our text
	  GLuint font_texture;

	  // information for our text
	  bitmap_font font;

	  const int map_width = 20;
	  const int map_height = 20;

	  // called when we are hit
	  void on_hit_ship() {
		  if (--num_lives == 0) {
			  game_over = true;
			  sprites[game_over_sprite].translate(-20, 0);
		  }
	  }

	  // use the keyboard to move the ship
	  void move_ship() {
		  sprites[ship_sprite].look_at(mouseWorldPos.x(), mouseWorldPos.y());
		  
		  const float ship_speed = 0.05f;

		  if (is_key_down(key_a)) {
			  sprites[ship_sprite].translate(-ship_speed, 0);
		  } else if (is_key_down(key_d)) {
			  sprites[ship_sprite].translate(ship_speed, 0);
		  } else if (is_key_down(key_w)) {
			  sprites[ship_sprite].translate(0, ship_speed);
		  } else if (is_key_down(key_s)) {
			  sprites[ship_sprite].translate(0, -ship_speed);
		  }

		  for (unsigned i = 0; i < walls.size(); ++i) {
			  if (sprites[ship_sprite].collides_with(walls[i])) {
				  if (is_key_down(key_a)) {
					  sprites[ship_sprite].translate(ship_speed, 0);
				  } else if (is_key_down(key_d)) {
					  sprites[ship_sprite].translate(-ship_speed, 0);
					  } else if (is_key_down(key_w)) {
					  sprites[ship_sprite].translate(0, -ship_speed);
				  } else if (is_key_down(key_s)) {
					  sprites[ship_sprite].translate(0, ship_speed);
				  }
			  }
		  }

		  // move camera with player
		  float dx = sprites[ship_sprite].get_position()[0] - cameraToWorld[3][0];
		  float dy = sprites[ship_sprite].get_position()[1] - cameraToWorld[3][1];
		  cameraToWorld.translate(vec3(dx, dy, 0));
	  }

	  // fire button (space)
	  void fire_missiles() {
		  if (missiles_disabled) {
			  --missiles_disabled;
		  } else if (is_key_going_down(' ') || is_key_going_down(key_lmb)) {
			  // find a missile
			  for (int i = 0; i != num_missiles; ++i) {
				  if (!sprites[first_missile_sprite + i].is_enabled()) {
					  sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0, 0.5f);
					  sprites[first_missile_sprite + i].is_enabled() = true;
					  missiles_disabled = 5;
					  break;
				  }
			  }
		  }
	  }

	  // animate the missiles
	  void move_missiles() {
		  const float missile_speed = 0.3f;
		  for (int i = 0; i != num_missiles; ++i) {
			  sprite &missile = sprites[first_missile_sprite + i];
			  if (missile.is_enabled()) {
				  missile.translate(0, missile_speed);
				  for (unsigned j = 0; j < num_sprites; ++j) {
					  if (missile.collides_with(sprites[j])) {
						  if (sprites[j].get_type() == sprite::sprite_type::enemy_sprite) {
							  // TODO: implement missile damage
						  }
					  }
				  }
				  for (unsigned j = 0; j < walls.size(); ++j) {
					  if (missile.collides_with(walls[j])) {
						  missile.is_enabled() = false;
						  missile.translate(20, 0);
					  }
				  }
			  }
		  next_missile:;
		  }
	  }

	  void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
		  mat4t modelToWorld;
		  modelToWorld.loadIdentity();
		  modelToWorld.translate(x, y, 0);
		  modelToWorld.scale(scale, scale, 1);
		  mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

		  /*mat4t tmp;
		  glLoadIdentity();
		  glTranslatef(x, y, 0);
		  glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
		  glScalef(scale, scale, 1);
		  glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

		  enum { max_quads = 32 };
		  bitmap_font::vertex vertices[max_quads * 4];
		  uint32_t indices[max_quads * 6];
		  aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

		  unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
		  glActiveTexture(GL_TEXTURE0);
		  glBindTexture(GL_TEXTURE_2D, font_texture);

		  shader.render(modelToProjection, 0);

		  glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
		  glEnableVertexAttribArray(attribute_pos);
		  glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
		  glEnableVertexAttribArray(attribute_uv);

		  glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
	  }

  public:

	  // this is called when we construct the class
	  IP_Assign1(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
	  }

	  // this is called once OpenGL is initialized
	  void app_init() {
		  enable_cursor();
		  
		  // set up the shader
		  texture_shader_.init();

		  // set up the matrices with a camera 5 units from the origin
		  cameraToWorld.loadIdentity();
		  cameraToWorld.translate(0, 0, 3);

		  // generate random cave
		  GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
		  level_generator *generator = new level_generator();
		  int startX = 0;
		  int startY = 0;
		  bool found = false;
		  int **map = generator->generate_level(map_width, map_height);
		  for (unsigned i = 0; i < map_height; ++i) {
			  for (unsigned j = 0; j < map_width; ++j) {
				  if (map[i][j] == 1) {
					  sprite s;
					  s.init(white, j, map_height - i, 1, 1, sprite::sprite_type::wall_sprite);
					  walls.push_back(s);
				  }
				  if (!found) {
					  if (map[i][j] == 0) {
						  startX = j;
						  startY = map_height - i;
						  found = true;
					  }
				  }
			  }
		  }
		  delete generator;

		  font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

		  GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/shooter/character.gif");
		  sprites[ship_sprite].init(ship, startX, startY, 0.25f, 0.25f, sprite::sprite_type::character_sprite);

		  GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
		  sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1.5f, sprite::sprite_type::other_sprite);

		  // use the missile texture
		  GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
		  for (int i = 0; i != num_missiles; ++i) {
			  // create missiles off-screen
			  sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f, sprite::sprite_type::missile_sprite);
			  sprites[first_missile_sprite + i].is_enabled() = false;
		  }

		  // sundry counters and game state.
		  missiles_disabled = 0;
		  num_lives = 3;
		  game_over = false;
		  score = 0;
	  }

	  // called every frame to move things
	  void simulate() {
		  if (game_over) {
			  return;
		  }

		  move_ship();

		  fire_missiles();

		  move_missiles();
	  }

	  vec3 mouseWorldPos;

	  void compute_mouse_pos(int w, int h) {
		  int mouseX, mouseY;
		  get_mouse_pos(mouseX, mouseY);
		  float dx = -w / 2 + mouseX;
		  float dy = -h / 2 + mouseY;
		  mouseWorldPos = vec3(dx / ((float)w / 6.0f), dy / ((float)h / 6.0f), 0);
	  }

	  // this is called to draw the world
	  void draw_world(int x, int y, int w, int h) {
		  // compute mouse position in the world
		  compute_mouse_pos(w, h);

		  simulate();
		  // set a viewport - includes whole window area
		  glViewport(x, y, w, h);

		  // clear the background to black
		  glClearColor(0, 0, 0, 1);
		  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		  // don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
		  glDisable(GL_DEPTH_TEST);

		  // allow alpha blend (transparency when alpha channel is 0)
		  glEnable(GL_BLEND);
		  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		  // draw all the sprites
		  for (int i = 0; i != num_sprites; ++i) {
			  sprites[i].render(texture_shader_, cameraToWorld);
		  }

		  // draw walls
		  for (unsigned i = 0; i != walls.size(); ++i) {
			  walls[i].render(texture_shader_, cameraToWorld);
		  }

		  char score_text[32];
		  sprintf(score_text, "score: %d   lives: %d\n", score, num_lives);
		  draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, score_text);

		  // move the listener with the camera
		  vec4 &cpos = cameraToWorld.w();
		  alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
	  }
  };
}
