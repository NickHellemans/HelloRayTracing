#include "Renderer.h"

#include "Walnut/Random.h"
namespace Utils
{
    //Function to convert vec4 to uint32_t so we can use it in memorybuffer
    static uint32_t ConvertToRGBA(const glm::vec4& color)
    {
        uint8_t r = (uint8_t)(color.r * 255.0f); //Red channel of our color = 8 bits/32 bits
        uint8_t g = (uint8_t)(color.g * 255.0f); //Green channel of our color = 8 bits/32 bits
        uint8_t b = (uint8_t)(color.b * 255.0f); //Blue channel of our color = 8 bits/32 bits
        uint8_t a = (uint8_t)(color.a * 255.0f); //Alpha channel of our color = 8 bits/32 bits

        //Combine color channels into one color and put them in right place: abgr
        uint32_t result = (a << 24) | (b << 16) | (g << 8) | r;
        return result;
    }
}
void Renderer::OnResize(uint32_t width, uint32_t height)
{

    if(m_FinalImage)
    {
        //Exit function when no resize is needed so delete doesnt get called
        if(m_FinalImage->GetWidth() == width && m_FinalImage->GetHeight() == height)
            return;
        //Personal function inside Walnut::Image | Will check if image needs resizing, if so -> release and reallocate memory
        //Better design than to delete image and create new one + the object pointer doesnt change, just the contents of the memory block
        //So everything that holds a reference to this object still has same image because we dont delete object and create new one that will
        //different spot in memory
        m_FinalImage->Resize(width, height);
    }
    else
    {
        //Create image if it doesnt exist only no need to recreate it when width/height dont match current image size
        //this gets handles in resize function
        m_FinalImage = std::make_shared<Walnut::Image>(width, height, Walnut::ImageFormat::RGBA);
    }

    //Delete current image data if exists - its ok to call delete on nullptr(first render) - delete will check for u
    delete[] m_ImageData;
    //Reallocate for new data
    //uint32 is 32 bits -> RGBA 1 byte for each color = 32bits
    m_ImageData = new uint32_t[width * height];
}

void Renderer::Render(const Scene& scene, const Camera& camera)
{
    //const glm::vec3& rayOrigin = camera.GetPosition();
    Ray ray;
    ray.Origin = camera.GetPosition();
    //Render every pixel of viewport
    //Fill image data
    //Iterate through y first = better performance - next uint32 is horizontal - dont want to skip "rows" if
    //first going vertical and then going horizontal
    for (uint32_t y = 0; y < m_FinalImage->GetHeight(); y++)
    {
        for (uint32_t x = 0; x < m_FinalImage->GetWidth(); x++) {
            //get coordinate in space - 
            //glm::vec2 coord = {x/(float) m_FinalImage->GetWidth(), y/(float) m_FinalImage->GetHeight()};
            
            //Remap 0-1 to -1-1 : to get rays in all directions
            //coord = coord * 2.0f - 1.0f;

            //Dont need to get coord anymore -> calculation inside GetRayDirections
            ray.Direction = camera.GetRayDirections()[x+y*m_FinalImage->GetWidth()];
            
            //Get color for pixel
            glm::vec4 color = TraceRay(scene, ray);
            //clamp values between 0 and 1 so we dont get any spill into other channels - 1 = 255 = max
            //GPU will do this for us but we are on CPU
            color = glm::clamp(color,glm::vec4(0.0f), glm::vec4(1.0f));
            //Index = offset x with how big each row is - y * width
            m_ImageData[x + y * m_FinalImage->GetWidth()] = Utils::ConvertToRGBA(color);
        }
    }
    //Upload to gpu for rendering - Inefficient to load data on cpu and transfer to GPU but will change later in series
    m_FinalImage->SetData(m_ImageData);
}

//glm::vec4 Renderer::PerPixel(glm::vec2 coord)
glm::vec4 Renderer::TraceRay(const Scene& scene, const Ray& ray)
{
    //Convert coord in to color channel: x = red, y = green
    //8bits per color channel: rgba
    //uint8_t r = (uint8_t) (coord.x * 255.0f);
    //uint8_t g = (uint8_t) (coord.y * 255.0f);

    /* Fn of circle substituted with values for a and b
     * (bx^2 + by^2)t^2 + (2(axbx + ayby))t + (ax^2 + ay^2 - r^2) = 0
     * where
     * a = origin of ray
     * b = direction
     * r = radius
     * t = hit distance
     */

    //Dont need these anymore(rayOrigin and direction) because we have everything in ray object
    //Set forward axis of our camera shooting rays to -1
    //so it has a z dimension else rays would get casted in 2d space and wont hit anything -> -1 or 1 depends on
    //right or left handed rule
    //glm::vec3 rayDirection = {coord.x, coord.y, -1.0f};
    //rayDirection = glm::normalize(rayDirection);

    //Origin of rays - move camera back along z so it is not inside our circle
    //glm::vec3 rayOrigin(0.0f, 0.0f, 1.0f);

    //Radius of our sphere - we get radius from sphere itself now
    //float radius = 0.5f;

    if (scene.Spheres.empty())
        return {0,0,0,1};

    const Sphere* closestSphere = nullptr;
    float hitDistance = FLT_MAX; //Keep closest so far
    for (const Sphere& sphere : scene.Spheres)
    {
        //Moving sphere by moving the ray origin/camera by sphere pos, effectively adding 0.5 to x component of the ray origin and using this as new origin
        //glm::vec3 origin = ray.Origin - glm::vec3(-0.5f, 0.0f, 0.0f);
        glm::vec3 origin = ray.Origin - sphere.Position;
    
        // Named by convention of quadratic formula: a, b, c -> a^2t + bt + c
        //a = (bx^2 + by^2) -> operation = dot product with self : multiple components and add (bx * bx + by * by)
        float a = glm::dot(ray.Direction, ray.Direction);
        //b = (2(axbx + ayby)) -> dot product: multiple each component from a with b and add
        float b = 2.0f * (glm::dot(origin, ray.Direction) );
        // c = (ax^2 + ay^2 - r^2) -> dot product of a with a - r^2
        float c = glm::dot(origin, origin) - sphere.Radius * sphere.Radius;
        //Fill these in to discriminant and quadratic formula to find solutions
        //Discriminant = b^2 - 4ac
    
        float discriminant = b * b - 4.0f * a * c; //Check if there are solutions or not, no need to go further if there are no solutions/hits
        if (discriminant < 0.0f)
            continue; //Check for other possible spheres
        //return {0,0,0,1};
        //return 0xffff00ff;

        //Part before solutions -> return color based on discriminant
        //return glm::vec4(0,0,0,1);    
        //return 0xff000000; -> this order is abgr but in vector order is rgba (more logical for us )but have to make sure in memory
        //is correct by adjusting the bits to their right place -> 0xff000000
        //return 0xff000000 | (g<<8) | r; // OR red and green (offset by 8 so its after red) channels into color with alpha = 1 , blue = 00
        //m_ImageData[i] = Walnut::Random::UInt(); 
        //m_ImageData[i] |= 0xff000000; 
        // 0xffff00ff : From l->r: ABGR, 1 byte each == 2 letters
        //Or equals to set bits of alpha channel to 1 (most significant) because
        //random uint will give us random alpha values aswell and we want to see image

        //Quadratic formula = (-b +- sqrt(discriminant) / 2a -> find solutions
        float t0 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
        float closestT = (-b - glm::sqrt(discriminant)) / (2.0f * a); //closest to origin (camera) - a will never be negative and we subtract
        //For multiple spheres: Check all hits for a ray and get closest (not thinking about translucent objects yet)
        if (closestT < hitDistance)
        {
            hitDistance = closestT;
            closestSphere = &sphere;
        }
    }
  //if sphere is still nullptr after going through scene, then we didnt hit a single sphere so return with default color  
 if(closestSphere == nullptr)
     return {0.0f, 0.0f, 0.0f, 1.0f};

    //Recalculate origin for closest sphere
    glm::vec3 origin = ray.Origin - closestSphere->Position;
    
    //t = hit distance -> plug in original ray equation to find point -> a + bt 
    //No need to do every coord by itself -> glm takes care of it
    //glm::vec3 h0 = rayOrigin + rayDirection * t0;
    glm::vec3 hitPoint = origin + ray.Direction * hitDistance;
    //Can use hitpoint as color -> is a vec3. Basically take every coordinate of the hit points and use that as color
    // x = r, y = g and z = b
    //Negative on x and y will get clamped to 0 so we get blue in middle and bottom left
    //More red going right x++
    //More green going up y++

    //We need to get direction sphere is facing in to correctly light it/shade it
    //Need to know for every pixel where it is facing --> Get the normal for every pixel
    //To get normal for every pixel in a sphere is easy -> subtract origin from the hit points
    //Our current origin for our sphere = 0 -> hit point = normal
    //Need to normalize to only get a direction vector not the whole radius (if radius is big, we dont want that)
    //Just need the direction, length doesnt matter for the normal
    glm::vec3 normal = glm::normalize(hitPoint);

    //Now we are just shooting rays in -z direction and we dont get much out of the normals -> everything face us is
    //in +z direction so everything is blue mostly
    //We need a light direction on our sphere so we can compare our normals to that direction instead
    //This way we can find HOW MUCH our surface is facing the light
    //Define a direction vector going in -x, -y and -z -> from right, top and front
    glm::vec3 lightDir = glm::normalize(glm::vec3(-1,-1,-1));

    //direction of light is going towards us -> we want to compare normal with direction that is going towards light direction
    //Incoming vs outgoing direction vector
    //Negate incoming direction -> flips it
    //To compare the 2 vectors we use the dot product
    //It will tell us the relationship between the 2 directional vectors
    //The value between -1 and 1 will tell how much the vectors look like each other 1 is exact same , -1 is total other direction
    //We can use this to tell how much it is facing the light and make those areas lighter

    //Clamp to 0 if result is negative = facing away
    float d = glm::max(glm::dot(normal, -lightDir), 0.0f);
    
    glm::vec3 sphereColor = closestSphere->Albedo;
    //result of dot product gives us the intensity of what the color should be
    sphereColor *= d;
    return {sphereColor, 1.0f};
    //glm::vec4(sphereColor, 1.0f);
}
