// shader to draw a star field
// inspiration from https://www.shadertoy.com/view/Md2SR3
// and noise from http://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

namespace octet {
    namespace shaders {
        class star_shader : public shader {
            GLuint modelToProjectionIndex_;
            GLuint resolutionIndex_;
            GLuint offsetIndex_;

        public:
            void init() {
           
                // vertex shader
                const char vertex_shader[] = SHADER_STR(
                    attribute vec4 pos;
                    uniform mat4 modelToProjection;

                    void main() { 
                        gl_Position = modelToProjection * pos; 
                    }
                );

                // fragment shader
                const char fragment_shader[] = SHADER_STR(

                    uniform vec2 resolution;
                    uniform vec2 offset;
                    
                    float rand(vec2 co) {
                        return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
                    }

                    void main() { 

                        vec2 p = (gl_FragCoord.xy + floor(offset)) / resolution.xy;
                        vec3 vColor = vec3(0.1, 0.2, 0.4) * p.y;

                        float threshold = 0.97;
                        float startVal = rand(p.xy);
                        
                        if (startVal >= threshold) {
                            startVal = pow((startVal - threshold) / (1.0 - threshold), 6.0);
                            vColor += vec3(startVal);
                        }

                        gl_FragColor = vec4(vColor, 1.0);
                    }
                );

                shader::init(vertex_shader, fragment_shader);

                modelToProjectionIndex_ = glGetUniformLocation(program(), "modelToProjection");
                resolutionIndex_ = glGetUniformLocation(program(), "resolution");
                offsetIndex_ = glGetUniformLocation(program(), "offset");
            }

            void render(const mat4t &modelToProjection, const vec2 &resolution, const vec2 &offset) {
                shader::render();

                glUniform2fv(offsetIndex_, 1, offset.get());
                glUniform2fv(resolutionIndex_, 1, resolution.get());
                glUniformMatrix4fv(modelToProjectionIndex_, 1, GL_FALSE, modelToProjection.get());
            }
        };
    }
}
