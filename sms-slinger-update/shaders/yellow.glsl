
#version 330 core

uniform sampler2D u_texture;

in vec2 v_texCoord;

out vec4 FragColour;

void main()
{
    vec4 colour = texture(u_texture, v_texCoord);

    //process the colour fragment here...

    FragColour = colour + vec4(1.0, 1.0, 0.0, 1.0);
}
