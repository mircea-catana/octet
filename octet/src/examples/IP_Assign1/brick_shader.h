namespace octet {
    namespace shaders{
        class brick_shader : public shader {
            GLuint modelToProjectionIndex_;
            GLuint resolutionIndex_;

        public:
            void init() {

                // Vertex Shader
                const char vertex_shader[] = SHADER_STR(
                    attribute vec4 pos;
                    uniform mat4 modelToProjection;

                    void main() { gl_Position = modelToProjection * pos; }
                );

                // Fragment Shader
                const char fragment_shader[] = SHADER_STR(
                    uniform vec2 resolution;

                    void main() { 
                        vec2 p = gl_FragCoord.xy / resolution.xy;
                        p.x *= resolution.x / resolution.y;

                        vec3 color = vec3(1.0, 0.0, 0.0);
                        float index = floor(p.y * 15.0);
                        float k = mod(index, 2.0) == 0.0 ? 2.0 : 6.0;
                        if (mod(floor(p.x * 70.0), 8.0) == k || mod(floor(p.y * 60.0), 4.0) == 0.0) {
                            color = vec3(1.0);
                        }

                        gl_FragColor = vec4(color, 1.0f); 
                    }
                );

                shader::init(vertex_shader, fragment_shader);
                modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
                resolutionIndex_ = glGetUniformLocation(program(), "resolution");
            }

            void render(const mat4t &modelToProjection, const vec2 &resolution) {
                shader::render();

                glUniform2fv(resolutionIndex_, 1, resolution.get());
                glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
            }
        };
    }
}
