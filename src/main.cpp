#include <SFML/Graphics.hpp>
#include <chrono>

#include "config.h"
#include "common.h"
#include "material.h"
#include "primitives.h"
#include "renderer.h"
#include "scene.h"
#include "vector.h"

// ---------------------------------------------------------------------------------------------------------------------
// Declarations
// ---------------------------------------------------------------------------------------------------------------------

static void render_preview_mode(const Scene& world, const Renderer& render);
static void render_to_image(const Scene& world, const Renderer& render, const char *filename);
static void setup_render(Renderer& render);
static void setup_scene(Scene& scene);

// ---------------------------------------------------------------------------------------------------------------------
// -> Entrypoint <-
// ---------------------------------------------------------------------------------------------------------------------

int main() {
    Scene world;
    Renderer render(WINDOW_WIDTH, WINDOW_HEIGHT);

    setup_scene(world);
    setup_render(render);

#if PREVIEW_MODE
    render_preview_mode(world, render);
#else
    render_to_image(world, render, OUTPUTFILE);
#endif
}

// ---------------------------------------------------------------------------------------------------------------------
// Render modes
// ---------------------------------------------------------------------------------------------------------------------

static void render_preview_mode(const Scene& world, const Renderer& render) {
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), WINDOW_TITLE);

    sf::Texture buffer;
    sf::Sprite bufferSprite;
    buffer.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    bufferSprite.setTexture(buffer);
    sf::Image image = buffer.copyToImage();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
                return;
            }
        }

        render.render(image, world);

        // display
        buffer.loadFromImage(image);
        window.draw(bufferSprite);
        window.display();
    }
}

static void render_to_image(const Scene& world, const Renderer& render, const char *filename) {
    sf::Image image;
    image.create(WINDOW_WIDTH, WINDOW_HEIGHT);
    render.render(image, world);
    image.saveToFile(filename);
}

// ---------------------------------------------------------------------------------------------------------------------
// Setup functions
// ---------------------------------------------------------------------------------------------------------------------

static void setup_render(Renderer& render) {
    render.samples_num  = SAMPLES_NUM;
    render.render_depth = RENDER_DEPTH;

    render.vfov     = 40;
    render.lookfrom = Point(13, 2, 3);
    render.lookat   = Point(0, 0, 0);
    render.vup      = Vector(0, 1, 0);

    render.defocus_angle = 0; // disable defocus for now
    render.focus_dist    = 10.0;

    // reconfigure after changing parameters
    render.configure();
}

static void setup_scene(Scene& scene) {
    Material *ground_material = new Matte(Color(0.5, 0.5, 0.5));
    scene.register_material(ground_material);
    Sphere *ground = new Sphere(Point(0, -10000, 0), 10000, ground_material);
    scene.add_object(ground);

    // spawm little spheres
    for (int i = -15; i < 11; ++i) {
        for (int j = -15; j < 11; ++j) {
            double choose_mat = random_double();
            Point center(i + 0.9 * random_double(), 0.2, j + 0.9 * random_double());
            Material *sphere_material = nullptr;

            if ((center - Point(4, 0.2, 0)).length() > 0.9) {
                if (choose_mat < 0.8) {                                             // diffuse
                    Color albedo = Color::random() * Color::random();
                    sphere_material = new Matte(albedo);
                } else if (choose_mat < 0.91) {                                     // metal
                    Color albedo = Color::random(0.5, 1);
                    double fuzz = random_double(0, 0.4);
                    sphere_material = new Metal(albedo, fuzz);
                } else {                                                            // glass
                    sphere_material = new Glass(Color(1.0, 1.0, 1.0), 1.5);
                }
            }

            if (sphere_material) {
                scene.register_material(sphere_material);
                Sphere *sphere = new Sphere(center, 0.2, sphere_material);
                scene.add_object(sphere);
            }
        }
    }

    // three main spheres
    Material *material1 = new Glass(Color(1.0, 1.0, 1.0), 1.5);
    scene.register_material(material1);
    Sphere *sphere1 = new Sphere(Point(0, 1, 0), 1.0, material1);
    scene.add_object(sphere1);

    Material *material2 = new Matte(Color(0.4, 0.2, 0.1));
    scene.register_material(material2);
    Sphere *sphere2 = new Sphere(Point(-4, 1, 0), 1.0, material2);
    scene.add_object(sphere2);

    Material *material3 = new Metal(Color(0.7, 0.6, 0.5), 0.0);
    scene.register_material(material3);
    Sphere *sphere3 = new Sphere(Point(4, 1, 0), 1.0, material3);
    scene.add_object(sphere3);
}