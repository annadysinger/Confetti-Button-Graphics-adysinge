#include "engine.h"

enum state {start, play, over};
state screen;

// Colors
color originalFill, hoverFill, pressFill;

Engine::Engine() : keys() {
    this->initWindow();
    this->initShaders();
    this->initShapes();

    originalFill = {1, 0, 0, 1};
    hoverFill.vec = originalFill.vec + vec4{0.5, 0.5, 0.5, 0};
    pressFill.vec = originalFill.vec - vec4{0.5, 0.5, 0.5, 0};
}

Engine::~Engine() {}

unsigned int Engine::initWindow(bool debug) {
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, false);

    window = glfwCreateWindow(width, height, "engine", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // OpenGL configuration
    glViewport(0, 0, width, height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSwapInterval(1);

    return 0;
}

void Engine::initShaders() {
    // load shader manager
    shaderManager = make_unique<ShaderManager>();

    // Load shader into shader manager and retrieve it
    shapeShader = this->shaderManager->loadShader("../res/shaders/shape.vert", "../res/shaders/shape.frag",  nullptr, "shape");

    // Configure text shader and renderer
    textShader = shaderManager->loadShader("../res/shaders/text.vert", "../res/shaders/text.frag", nullptr, "text");
    fontRenderer = make_unique<FontRenderer>(shaderManager->getShader("text"), "../res/fonts/MxPlus_IBM_BIOS.ttf", 24);

    // Set uniforms
    textShader.use().setVector2f("vertex", vec4(100, 100, .5, .5));
    shapeShader.use().setMatrix4("projection", this->PROJECTION);
}

void Engine::initShapes() {
    // red spawn button centered in the top left corner
    spawnButton = make_unique<Rect>(shapeShader, vec2{width/2,height/2}, vec2{100, 50}, color{1, 0, 0, 1});
}

void Engine::processInput() {
    glfwPollEvents();

    // Set keys to true if pressed, false if released
    for (int key = 0; key < 1024; ++key) {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            keys[key] = true;
        else if (glfwGetKey(window, key) == GLFW_RELEASE)
            keys[key] = false;
    }

    // Close window if escape key is pressed
    if (keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);

    // Mouse position saved to check for collisions
    glfwGetCursorPos(window, &MouseX, &MouseY);

    // TODO: If we're in the start screen and the user presses s, change screen to play
    // Hint: The index is GLFW_KEY_S
    if(screen == start && keys[GLFW_KEY_S]){
        screen = play;
    }

    // TODO: If we're in the play screen and an arrow key is pressed, move the pawnButton
    // Hint: one of the indices is GLFW_KEY_UP
    if(screen == play && keys[GLFW_KEY_UP] && spawnButton->getPosY() <= 600){
        spawnButton->moveY(1);
    }
    if(screen == play && keys[GLFW_KEY_DOWN] && spawnButton->getPosY() >= 0){
        spawnButton->moveY(-1);
    }
    if(screen == play && keys[GLFW_KEY_LEFT] && spawnButton->getPosX() >= 0){
        spawnButton->moveX(-1);
    }
    if(screen == play && keys[GLFW_KEY_RIGHT] && spawnButton->getPosX() <= 800){
        spawnButton->moveX(1);
    }
    // TODO: Make sure the spawnButton cannot go off the screen


    // Mouse position is inverted because the origin of the window is in the top left corner
    MouseY = height - MouseY; // Invert y-axis of mouse position
    bool buttonOverlapsMouse = spawnButton->isOverlapping(vec2(MouseX, MouseY));
    bool mousePressed = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    // TODO: When in play screen, if the user hovers or clicks on the button then change the spawnButton's color
    // Hint: look at the color objects declared at the top of this file
    if(screen == play && (buttonOverlapsMouse || mousePressed)){
        spawnButton->setRed(100);
    }
    // TODO: When in play screen, if the button was released then spawn confetti
    // Hint: the button was released if it was pressed last frame and is not pressed now
    if(screen == play && mousePressed == false && mousePressedLastFrame == true){
        spawnConfetti();
    }
    // TODO: Make sure the spawn button is its original color when the user is not hovering or clicking on it.
    if(screen == play && !(buttonOverlapsMouse && mousePressed)){
        spawnButton->setRed(10);
    }


    // Save mousePressed for next frame
    mousePressedLastFrame = mousePressed;

}

void Engine::update() {
    // Calculate delta time
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // TODO: End the game when the user spawns 100 confetti
    if(confetti.size() >= 100){
        screen = over;
    }
    // If the size of the confetti vector reaches 100, change screen to over

}

void Engine::render() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Set background color
    glClear(GL_COLOR_BUFFER_BIT);

    // Set shader to use for all shapes
    shapeShader.use();

    // Render differently depending on screen
    switch (screen) {
        case start: {
            string message = "Press s to start";
            // (12 * message.length()) is the offset to center text.
            // 12 pixels is the width of each character scaled by 1.
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, 1, vec3{1, 1, 1});
            break;
        }
        case play: {
            // TODO: call setUniforms and draw on the spawnButton and all of the confetti pieces
            //  Hint: make sure you draw the spawn button after the confetti to make it appear on top
            // Render font on top of spawn button


            for(const unique_ptr<Shape>& r: confetti){
                r->setUniforms();
                r->draw();
            }
            spawnButton->setUniforms();
            spawnButton->draw();


            fontRenderer->renderText("Spawn", spawnButton->getPos().x - 30, spawnButton->getPos().y - 5, 0.5, vec3{1, 1, 1});
            break;
        }
        case over: {
            string message = "You win!";
            this->fontRenderer->renderText(message, width/2 - (12 * message.length()), height/2, 1, vec3{1, 1, 1});
            break;
            // TODO: Display the message on the screen

        }
    }

    glfwSwapBuffers(window);
}

void Engine::spawnConfetti() {
    vec2 pos = {rand() % (int)width, rand() % (int)height};
    // TODO: Make each piece of confetti a different size, getting bigger with each spawn.
    //  The smallest should be a square of size 1 and the biggest should be a square of size 100
    int rd = 1 + (rand() % 100);
    vec2 size = {rd, rd}; // placeholder
    color color = {float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), float(rand() % 10 / 10.0), 1.0f};
    confetti.push_back(make_unique<Rect>(shapeShader, pos, size, color));
}

bool Engine::shouldClose() {
    return glfwWindowShouldClose(window);
}