#version 400

    in vec4 position;
    uniform sampler2D terrain;
    uniform float height_scale;
    uniform float height_offset;
    
    void main(void){
        vec2 texcoord = position.xy;
        float height = texture(terrain, texcoord).a * height_scale + height_offset;
        vec4 displaced = vec4(position.x, position.y, height, 1.0);
        gl_Position = displaced;
    }
