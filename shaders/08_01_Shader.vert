

#version 330

//  Variabile de intrare (dinspre programul principal);
layout (location = 0) in vec4 in_Position;     //  Se preia din buffer de pe prima pozitie (0) atributul care contine coordonatele;
layout (location = 1) in vec3 in_Normal;       //  Se preia din buffer de pe a doua pozitie (1) atributul care contine normalele;
layout (location = 2) in vec4 in_Color;        //  Se preia din buffer de pe a treia pozitie (2) atributul care contine culoarea;
layout (location = 3) in vec2 in_TexCoord;  // Texture coordinates (new)

//  Variabile de iesire;
//out vec4 gl_Position;   //  Transmite pozitia actualizata spre programul principal;
out vec4 ex_Color;      //  Transmite culoarea (de modificat in Shader.frag); 
out vec3 FragPos;
out vec3 Normal;
out vec3 inLightPos;
out vec3 inViewPos;
out vec2 ex_TexCoord;    // Pass texture coordinates to fragment shader (new)


//  Variabile uniforme;
uniform mat4 viewModel;
uniform mat4 projection;
uniform int nrVertices;
uniform mat4 myMatrix;
uniform mat4 view;
uniform vec3 lightPos; 
uniform vec3 viewPos; 

void main(void)
{
    gl_Position = projection * viewModel * view * in_Position;
    ex_Color = in_Color;
    FragPos = vec3(gl_Position);
    Normal=vec3(projection*view*vec4(in_Normal,0.0));
    inLightPos= vec3(projection*view* vec4(lightPos, 1.0f));
    inViewPos=vec3(projection*view*vec4(viewPos, 1.0f));
    // Pass texture coordinates to fragment shader
    ex_TexCoord = in_TexCoord;
} 
 