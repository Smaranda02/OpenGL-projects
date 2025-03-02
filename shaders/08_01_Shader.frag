//
// ================================================
// | Grafica pe calculator                        |


#version 330

float winWidth = 1400; // Latimea ferestrei

//	Variabile de intrare (dinspre Shader.vert);
in vec4 gl_FragCoord; // Variabila care indica pozitia fragmentului (prin raportare la fereastra de vizualizare)
in vec4 ex_Color; 
in vec3 FragPos;  
in vec3 Normal; 
in vec3 inLightPos;
in vec3 inViewPos;
in vec2 ex_TexCoord;


//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;



//  Variabile uniforme;
uniform vec3 lightColor;
uniform vec3 objectColor;
uniform int codCol;
// Uniform for the texture
uniform sampler2D moonTexture;
uniform sampler2D jupiterTexture;
uniform sampler2D uranusTexture;
uniform sampler2D earthTexture;



void main(void)
{
    vec3 objectColor=vec3(ex_Color);

    //  Ambient;
    float ambientStrength = 0.2f;
    vec3 ambient_light = ambientStrength * lightColor;  // ambient_light=ambientStrength*lightColor;
    vec3 ambient_term= ambient_light * objectColor;     // ambient_material=objectColor;
      
    //  Diffuse; 
    vec3 norm = normalize(Normal); // vectorul s 
    vec3 lightDir = normalize(inLightPos - FragPos);            // vectorul L;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse_light = lightColor;                            // diffuse_light=lightColor;
    vec3 diffuse_term = diff * diffuse_light * objectColor;     // diffuse_material=objectColor
    
    //  Specular;
    float specularStrength = 0.7f;
    float shininess = 100.0f;
    vec3 viewDir = normalize(inViewPos - FragPos);              // versorul catre observator;
    vec3 reflectDir = normalize(reflect(-lightDir, norm));      // versorul vectorului R;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess); 
    vec3 specular_light = specularStrength  * lightColor;       // specular_light=specularStrength  * lightColor;
    vec3 specular_term = spec * specular_light * objectColor;   // specular_material=objectColor;
       
    //  Culoarea finala; 
    //vec3 emission=vec3(0.0, 0.0, 0.0);
    vec3 emission=vec3(1.0,0.8,0.4);
    vec3 result = emission +  (ambient_term + diffuse_term + specular_term);
    out_Color = vec4(result, 1.0f);


    switch (codCol)
    {
        case 1: 
            out_Color = texture(uranusTexture, ex_TexCoord);
            break;
        case 2:
            out_Color = texture(earthTexture, ex_TexCoord);
            break;
        case 3:
            out_Color = texture(moonTexture, ex_TexCoord);
            break;
        case 4:
            out_Color=vec4(1.0, 0.8, 0.8, 1.0); // Culoarea este stabilita in functie de pozitia fragmentului in fereastra 
            break;
        case 5:
            out_Color = texture(jupiterTexture, ex_TexCoord);
            break;
        case 6:
            out_Color =   vec4 (1.0, 0.855, 0.725, 1.0);
            break;
      
        default: 
            out_Color=ex_Color;
    }
}