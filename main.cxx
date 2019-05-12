/*
  (C) 2019 David Lettier
  lettier.com
*/

#include <thread>
#include <unistd.h>
#include <random>
#include <string>

#include "pandaFramework.h" // Panda3D 1.10.1-1
#include "pandaSystem.h"
#include "mouseWatcher.h"
#include "buttonRegistry.h"
#include "orthographicLens.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "shader.h"
#include "cardMaker.h"
#include "fog.h"

// STRUCTURES

struct FramebufferTexture
  { PT(GraphicsOutput) buffer
  ; PT(Camera) camera
  ; NodePath cameraNP
  ; NodePath shaderNP
  ;
  };

// END STRUCTURES

// FUNCTIONS

float generateLightColorPart
  ( float amount
  , float intensity
  );
void generateLights
  ( NodePath render
  , bool showLights
  );
void generateWindowLight
  ( std::string name
  , NodePath render
  , LVecBase3 position
  , bool show
  );

PT(Shader) loadShader
  ( std::string vert
  , std::string frag
  );

FramebufferTexture generateHighPrecisionFramebufferTextureForScene
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , std::string name
  );

FramebufferTexture generateFramebufferTextureForScene
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , std::string name
  , LColor clearColor
  );

FramebufferTexture generateFramebufferTextureForImage
  ( PT(GraphicsOutput) graphicsOutput
  , std::string name
  , LColor clearColor
  );

PTA_LVecBase3f generateSsaoSamples
  ( int numberOfSamples
  );
PTA_LVecBase3f generateSsaoNoise
  (
  );

void showBuffer
  ( NodePath render2d
  , PT(GraphicsOutput) buffer
  , bool alpha
  );

int microsecondsSinceEpoch
  (
  );

double angleIncAndLimit
  ( double angle
  , double inc
  , double limit
  );
double angleDecAndLimit
  ( double angle
  , double dec
  , double limit
  );

bool isButtonDown
  ( WindowFramework* window
  , std::string character
  );
PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  );

// END FUNCTIONS

// GLOBALS

std::uniform_real_distribution<float>
  randomFloats
    ( 0.0
    , 1.0
    );
std::default_random_engine generator;

PT(AsyncTaskManager) taskManager = AsyncTaskManager::get_global_ptr();

// END GLOBALS

// MAIN

int main
  ( int argc
  , char *argv[]
  ) {
  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("3D Game Shaders For Beginners - David Lettier");
  PT(WindowFramework) window = framework.open_window();

  LColor backgroundColor =
    LColor
      ( generateLightColorPart(207, 1)
      , generateLightColorPart(154, 1)
      , generateLightColorPart(108, 1)
      , 1
      );

  PT(DisplayRegion) displayRegion3d = window->get_display_region_3d();
  displayRegion3d->set_clear_color_active(true);
  displayRegion3d->set_clear_depth_active(true);
  displayRegion3d->set_clear_stencil_active(true);
  displayRegion3d->set_clear_color(backgroundColor);
  displayRegion3d->set_clear_depth(1.0f);
  displayRegion3d->set_clear_stencil(0);

  NodePath render = window->get_render();
  NodePath render2d = window->get_render_2d();

  PT(Lens) lens = window->get_camera(0)->get_lens();
  lens->set_fov(1);
  lens->set_near_far(450, 1510);

  NodePath cameraNP = window->get_camera_group();

  float fogNear = 1490;
  float fogFar  = 1500;
  PT(Fog) fog = new Fog("fog");
  fog->set_color(backgroundColor);
  fog->set_linear_range(fogNear, fogFar);
  cameraNP.attach_new_node(fog);
  render.set_fog(fog);

  double cameraRotateRadius = 1400;
  double cameraRotateTheta  = 360 - 135;
  double radians = cameraRotateTheta * 3.14 / 180;
  double x = cameraRotateRadius * cos(radians);
  double y = cameraRotateRadius * sin(radians);

  cameraNP.set_pos(x, y, 505);
  cameraNP.look_at(LVecBase3(0, 0, 5));

  PT(PandaNode) sceneRootPN = new PandaNode("sceneRoot");
  NodePath sceneRootNP = NodePath(sceneRootPN);
  sceneRootNP.reparent_to(render);

  NodePath environmentNP =
    window
      ->load_model
        ( framework.get_models()
        , "eggs/mill-scene/mill-scene"
        );
  environmentNP.reparent_to(sceneRootNP);

  NodePath wheelNP = environmentNP.find("**/wheel-lp");
  wheelNP.set_two_sided(true);

  generateLights(render, false);

  NodePath gridFloor = environmentNP.find("**/grid-floor");
  gridFloor.set_light_off();

  PT(Shader) baseShader            = loadShader("base",  "base");
  PT(Shader) materialDiffuseShader = loadShader("base",  "material-diffuse");
  PT(Shader) positionShader        = loadShader("base",  "position");
  PT(Shader) normalShader          = loadShader("base",  "normal");
  PT(Shader) outlineShader         = loadShader("basic", "outline");
  PT(Shader) bloomShader           = loadShader("basic", "bloom");
  PT(Shader) ssaoShader            = loadShader("basic", "ssao");
  PT(Shader) ssaoBlurShader        = loadShader("basic", "ssao-blur");
  PT(Shader) combineShader         = loadShader("basic", "combine");
  PT(Shader) combineBlurShader     = loadShader("basic", "combine-blur");
  PT(Shader) depthOfFieldShader    = loadShader("basic", "depth-of-field");
  PT(Shader) posterizeShader       = loadShader("basic", "posterize");
  PT(Shader) pixelizeShader        = loadShader("basic", "pixelize");

  PT(GraphicsOutput) graphicsOutput = window->get_graphics_output();
  PT(GraphicsEngine) graphicsEngine = graphicsOutput->get_gsg()->get_engine();

  bool showPositionBuffer        = false;
  bool showNormalBuffer          = false;
  bool showSsaoBuffer            = false;
  bool showSsaoBlurBuffer        = false;
  bool showMaterialDiffuseBuffer = false;
  bool showOutlineBuffer         = false;
  bool showBaseBuffer            = false;
  bool showBloomBuffer           = false;
  bool showCombineBuffer         = false;
  bool showCombineBlurBuffer     = false;
  bool showDepthOfFieldBuffer    = false;
  bool showPosterizeBuffer       = false;
  bool showPixelizeBuffer        = true;

  int ssaoEnabled         = 1;
  int fogEnabled          = 1;
  int outlineEnabled      = 1;
  int normalMapsEnabled   = 1;
  int bloomEnabled        = 1;
  int depthOfFieldEnabled = 1;
  int posterizeEnabled    = 1;
  int pixelizeEnabled     = 1;

  FramebufferTexture positionFramebufferTexture =
    generateHighPrecisionFramebufferTextureForScene
      ( window
      , graphicsOutput
      , graphicsEngine
      , "positionBuffer"
      );
  positionFramebufferTexture.shaderNP.set_shader(positionShader);
  positionFramebufferTexture
    .camera
      ->set_initial_state(positionFramebufferTexture.shaderNP.get_state());
  PT(Texture) positionTexture = positionFramebufferTexture.buffer->get_texture();
  PT(Lens) positionCameraLens = positionFramebufferTexture.camera->get_lens();

  if (showPositionBuffer)
    showBuffer
      ( render2d
      , positionFramebufferTexture.buffer
      , false
      );

  FramebufferTexture normalFramebufferTexture =
    generateHighPrecisionFramebufferTextureForScene
      ( window
      , graphicsOutput
      , graphicsEngine
      , "normalBuffer"
      );
  normalFramebufferTexture.shaderNP.set_shader(normalShader);
  normalFramebufferTexture
    .camera
      ->set_initial_state(normalFramebufferTexture.shaderNP.get_state());
  PT(Texture) normalTexture = normalFramebufferTexture.buffer->get_texture();

  if (showNormalBuffer)
    showBuffer
      ( render2d
      , normalFramebufferTexture.buffer
      , false
      );

  FramebufferTexture ssaoFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "ssao"
      , (1, 1, 1, 0)
      );
  PT(Camera) ssaoCamera = ssaoFramebufferTexture.camera;
  NodePath ssaoNP = ssaoFramebufferTexture.shaderNP;
  ssaoNP.set_shader(ssaoShader);
  ssaoNP.set_shader_input("positionTexture", positionTexture);
  ssaoNP.set_shader_input("normalTexture", normalTexture);
  ssaoNP.set_shader_input("samples", generateSsaoSamples(64));
  ssaoNP.set_shader_input("noise", generateSsaoNoise());
  ssaoNP.set_shader_input("lensProjection", positionCameraLens->get_projection_mat());
  ssaoNP.set_shader_input("enabled", LVecBase2(ssaoEnabled, ssaoEnabled));
  ssaoCamera->set_initial_state(ssaoNP.get_state());

  if (showSsaoBuffer)
    showBuffer
      ( render2d
      , ssaoFramebufferTexture.buffer
      , false
      );

  FramebufferTexture ssaoBlurFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "ssaoBlur"
      , LColor(1, 1, 1, 0)
      );
  NodePath ssaoBlurNP = ssaoBlurFramebufferTexture.shaderNP;
  ssaoBlurNP.set_shader(ssaoBlurShader);
  ssaoBlurNP.set_shader_input("ssaoTexture", ssaoFramebufferTexture.buffer->get_texture());
  ssaoBlurFramebufferTexture.camera->set_initial_state(ssaoBlurNP.get_state());

  if (showSsaoBlurBuffer)
    showBuffer
      ( render2d
      , ssaoBlurFramebufferTexture.buffer
      , false
      );

  FramebufferTexture materialDiffuseFramebufferTexture =
    generateFramebufferTextureForScene
      ( window
      , graphicsOutput
      , graphicsEngine
      , "materialDiffuse"
      , LColor(0, 0, 0, 1)
      );
  PT(GraphicsOutput) materialDiffuseBuffer = materialDiffuseFramebufferTexture.buffer;
  PT(Camera) materialDiffuseCamera = materialDiffuseFramebufferTexture.camera;
  NodePath materialDiffuseNP = materialDiffuseFramebufferTexture.shaderNP;
  materialDiffuseNP.set_shader(materialDiffuseShader);
  materialDiffuseCamera->set_initial_state(materialDiffuseNP.get_state());

  if (showMaterialDiffuseBuffer)
    showBuffer
      ( render2d
      , materialDiffuseBuffer
      , false
      );

  FramebufferTexture outlineFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "outline"
      , LColor(1, 1, 1, 0)
      );
  PT(GraphicsOutput) outlineBuffer = outlineFramebufferTexture.buffer;
  PT(Camera) outlineCamera = outlineFramebufferTexture.camera;
  NodePath outlineNP = outlineFramebufferTexture.shaderNP;
  outlineNP.set_shader(outlineShader);
  outlineNP.set_shader_input("materialDiffuseTexture", materialDiffuseBuffer->get_texture());
  outlineNP.set_shader_input("positionTexture", positionTexture);
  outlineNP.set_shader_input("enabled", LVecBase2(outlineEnabled, outlineEnabled));
  outlineNP.set_shader_input("fogEnabled", LVecBase2(fogEnabled, fogEnabled));
  outlineCamera->set_initial_state(outlineNP.get_state());
  NodePath outlineCameraNP = outlineFramebufferTexture.cameraNP;
  NodePath outlineRenderNP = outlineCameraNP.get_parent();
  PT(Fog) outlineFog = new Fog("outlineFog");
  outlineFog->set_color(backgroundColor);
  outlineFog->set_linear_range(fogNear, fogFar);
  outlineCameraNP.attach_new_node(outlineFog);
  outlineRenderNP.set_fog(outlineFog);

  if (showOutlineBuffer)
    showBuffer
      ( render2d
      , outlineBuffer
      , true
      );

  FramebufferTexture baseFramebufferTexture =
    generateFramebufferTextureForScene
      ( window
      , graphicsOutput
      , graphicsEngine
      , "base"
      , LColor(0, 0, 0, 0)
      );
  PT(GraphicsOutput) baseBuffer = baseFramebufferTexture.buffer;
  PT(Camera) baseCamera = baseFramebufferTexture.camera;
  NodePath baseNP = baseFramebufferTexture.shaderNP;
  baseNP.set_shader(baseShader);
  baseNP.set_shader_input("ssaoBlurTexture", ssaoBlurFramebufferTexture.buffer->get_texture());
  baseNP.set_shader_input("normalMapsEnabled", LVecBase2(normalMapsEnabled, normalMapsEnabled));
  baseNP.set_shader_input("fogEnabled", LVecBase2(fogEnabled, fogEnabled));
  baseCamera->set_initial_state(baseNP.get_state());

  if (showBaseBuffer)
    showBuffer
      ( render2d
      , baseBuffer
      , false
      );

  FramebufferTexture bloomFramebufferTexture =
    generateFramebufferTextureForScene
      ( window
      , graphicsOutput
      , graphicsEngine
      , "bloom"
      , LColor(0, 0, 0, 1)
      );
  PT(GraphicsOutput) bloomBuffer = bloomFramebufferTexture.buffer;
  PT(Camera) bloomCamera = bloomFramebufferTexture.camera;
  NodePath bloomNP = bloomFramebufferTexture.shaderNP;
  bloomNP.set_shader(bloomShader);
  bloomNP.set_shader_input("bloomTexture", baseBuffer->get_texture());
  bloomNP.set_shader_input("enabled", LVecBase2(bloomEnabled, bloomEnabled));
  bloomCamera->set_initial_state(bloomNP.get_state());

  if (showBloomBuffer)
    showBuffer
      ( render2d
      , bloomBuffer
      , false
      );

  FramebufferTexture combineFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "combine"
      , backgroundColor
      );
  PT(GraphicsOutput) combineBuffer = combineFramebufferTexture.buffer;
  NodePath combineNP = combineFramebufferTexture.shaderNP;
  combineNP.set_shader(combineShader);
  combineNP.set_shader_input("backgroundColor", backgroundColor);
  combineNP.set_shader_input("baseTexture", baseBuffer->get_texture());
  combineNP.set_shader_input("bloomTexture", bloomBuffer->get_texture());
  combineNP.set_shader_input("outlineTexture", outlineBuffer->get_texture());
  combineFramebufferTexture.camera->set_initial_state(combineNP.get_state());

  if (showCombineBuffer)
    showBuffer
      ( render2d
      , combineBuffer
      , false
      );

  FramebufferTexture posterizeFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "posterize"
      , backgroundColor
      );
  PT(GraphicsOutput) posterizeBuffer = posterizeFramebufferTexture.buffer;
  NodePath posterizeNP = posterizeFramebufferTexture.shaderNP;
  posterizeNP.set_shader(posterizeShader);
  posterizeNP.set_shader_input("posterizeTexture", combineBuffer->get_texture());
  posterizeNP.set_shader_input("enabled", LVecBase2(posterizeEnabled, posterizeEnabled));
  PT(Camera) posterizeCamera = posterizeFramebufferTexture.camera;
  posterizeCamera->set_initial_state(posterizeNP.get_state());

  if (showPosterizeBuffer)
    showBuffer
      ( render2d
      , posterizeBuffer
      , false
      );

  FramebufferTexture combineBlurFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "combineBlur"
      , backgroundColor
      );
  PT(GraphicsOutput) combineBlurBuffer = combineBlurFramebufferTexture.buffer;
  NodePath combineBlurNP = combineBlurFramebufferTexture.shaderNP;
  combineBlurNP.set_shader(combineBlurShader);
  combineBlurNP.set_shader_input("blurTexture", posterizeBuffer->get_texture());
  combineBlurFramebufferTexture.camera->set_initial_state(combineBlurNP.get_state());

  if (showCombineBlurBuffer)
    showBuffer
      ( render2d
      , combineBlurBuffer
      , false
      );

  FramebufferTexture depthOfFieldFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "depthOfField"
      , backgroundColor
      );
  PT(GraphicsOutput) depthOfFieldBuffer = depthOfFieldFramebufferTexture.buffer;
  NodePath depthOfFieldNP = depthOfFieldFramebufferTexture.shaderNP;
  depthOfFieldNP.set_shader(depthOfFieldShader);
  depthOfFieldNP.set_shader_input("positionTexture", positionTexture);
  depthOfFieldNP.set_shader_input("focusTexture", posterizeBuffer->get_texture());
  depthOfFieldNP.set_shader_input("outOfFocusTexture", combineBlurBuffer->get_texture());
  depthOfFieldNP
    .set_shader_input
      ( "focalLength"
      , LVecBase2f(environmentNP.get_distance(cameraNP), 0)
      );
  depthOfFieldNP.set_shader_input("enabled", LVecBase2(depthOfFieldEnabled, depthOfFieldEnabled));
  PT(Camera) depthOfFieldCamera = depthOfFieldFramebufferTexture.camera;
  depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());

  if (showDepthOfFieldBuffer)
    showBuffer
      ( render2d
      , depthOfFieldBuffer
      , false
      );

  FramebufferTexture pixelizeFramebufferTexture =
    generateFramebufferTextureForImage
      ( graphicsOutput
      , "pixelize"
      , backgroundColor
      );
  PT(GraphicsOutput) pixelizeBuffer = pixelizeFramebufferTexture.buffer;
  NodePath pixelizeNP = pixelizeFramebufferTexture.shaderNP;
  pixelizeNP.set_shader(pixelizeShader);
  pixelizeNP.set_shader_input("pixelizeTexture", depthOfFieldBuffer->get_texture());
  pixelizeNP.set_shader_input("enabled", LVecBase2(pixelizeEnabled, pixelizeEnabled));
  PT(Camera) pixelizeCamera = pixelizeFramebufferTexture.camera;
  pixelizeCamera->set_initial_state(pixelizeNP.get_state());

  if (showPixelizeBuffer)
    showBuffer
      ( render2d
      , pixelizeBuffer
      , false
      );

  int then = microsecondsSinceEpoch();
  int now = then;

  auto beforeFrame =
    [ &window
    , &ssaoEnabled
    , &bloomEnabled
    , &fogEnabled
    , &outlineEnabled
    , &normalMapsEnabled
    , &depthOfFieldEnabled
    , &posterizeEnabled
    , &pixelizeEnabled
    , &then
    , &now
    , &wheelNP
    , &cameraRotateTheta
    , &cameraRotateRadius
    , &cameraNP
    , &environmentNP
    , &positionCameraLens
    , &ssaoNP
    , &ssaoCamera
    , &bloomNP
    , &bloomCamera
    , &outlineNP
    , &outlineCamera
    , &baseNP
    , &baseCamera
    , &depthOfFieldNP
    , &depthOfFieldCamera
    , &posterizeNP
    , &posterizeCamera
    , &pixelizeNP
    , &pixelizeCamera
    ]() -> void {
      double delta = (now - then) / 1000000.0;
      then = now;
      now = microsecondsSinceEpoch();

      LVecBase3 hpr = wheelNP.get_hpr();
      hpr[1] = angleIncAndLimit(hpr[1], -90.0 * delta, 360);
      wheelNP.set_hpr(hpr);

      double movement = 100 * delta;

      bool shiftDown = isButtonDown(window, "shift");

      bool wDown     = isButtonDown(window, "w");
      bool aDown     = isButtonDown(window, "a");
      bool dDown     = isButtonDown(window, "d");
      bool sDown     = isButtonDown(window, "s");

      bool ssaoDown         = isButtonDown(window, "y");
      bool outlineDown      = isButtonDown(window, "u");
      bool bloomDown        = isButtonDown(window, "i");
      bool normalMapsDown   = isButtonDown(window, "o");
      bool fogDown          = isButtonDown(window, "p");
      bool depthOfFieldDown = isButtonDown(window, "h");
      bool posterizeDown    = isButtonDown(window, "j");
      bool pixelizeDown     = isButtonDown(window, "k");

      if (wDown) {
        cameraRotateRadius -= movement * 2;
        if (cameraRotateRadius <= 10) cameraRotateRadius = 10;
      }
      if (aDown) {
        cameraRotateTheta = angleIncAndLimit(cameraRotateTheta, movement * 0.5, 360);
      }
      if (dDown) {
        cameraRotateTheta = angleDecAndLimit(cameraRotateTheta, movement * 0.5, 0);
      }
      if (sDown) {
        cameraRotateRadius += movement * 2;
        if (cameraRotateRadius >= 1400) cameraRotateRadius = 1400;
      }

      if (shiftDown && ssaoDown) {
        ssaoEnabled = 0;
      } else if (ssaoDown) {
        ssaoEnabled = 1;
      }

      if (shiftDown && bloomDown) {
        bloomEnabled = 0;
      } else if (bloomDown) {
        bloomEnabled = 1;
      }

      if (shiftDown && normalMapsDown) {
        normalMapsEnabled = 0;
      } else if (normalMapsDown){
        normalMapsEnabled = 1;
      }

      if (shiftDown && fogDown) {
        fogEnabled = 0;
      } else if (fogDown) {
        fogEnabled = 1;
      }

      if (shiftDown && outlineDown) {
        outlineEnabled = 0;
      } else if (outlineDown) {
        outlineEnabled = 1;
      }

      if (shiftDown && depthOfFieldDown) {
        depthOfFieldEnabled = 0;
      } else if (depthOfFieldDown) {
        depthOfFieldEnabled = 1;
      }

      if (shiftDown && posterizeDown) {
        posterizeEnabled = 0;
      } else if (posterizeDown) {
        posterizeEnabled = 1;
      }

      if (shiftDown && pixelizeDown) {
        pixelizeEnabled = 0;
      } else if (pixelizeDown) {
        pixelizeEnabled = 1;
      }

      double radians = cameraRotateTheta * 3.14 / 180;
      double x = cameraRotateRadius * cos(radians);
      double y = cameraRotateRadius * sin(radians);

      cameraNP.set_pos(x, y, 505);
      cameraNP.look_at(LVecBase3(0, 0, 5));

      ssaoNP.set_shader_input("lensProjection", positionCameraLens->get_projection_mat());
      ssaoNP.set_shader_input("enabled", LVecBase2(ssaoEnabled, ssaoEnabled));
      ssaoCamera->set_initial_state(ssaoNP.get_state());

      bloomNP.set_shader_input("enabled", LVecBase2(bloomEnabled, bloomEnabled));
      bloomCamera->set_initial_state(bloomNP.get_state());

      outlineNP.set_shader_input("enabled", LVecBase2(outlineEnabled, outlineEnabled));
      outlineNP.set_shader_input("fogEnabled", LVecBase2(fogEnabled, fogEnabled));
      outlineCamera->set_initial_state(outlineNP.get_state());

      baseNP.set_shader_input("normalMapsEnabled", LVecBase2(normalMapsEnabled, normalMapsEnabled));
      baseNP.set_shader_input("fogEnabled", LVecBase2(fogEnabled, fogEnabled));
      baseCamera->set_initial_state(baseNP.get_state());

      depthOfFieldNP
        .set_shader_input
          ( "focalLength"
          , LVecBase2f(environmentNP.get_distance(cameraNP), 0)
          );
      depthOfFieldNP.set_shader_input("enabled", LVecBase2(depthOfFieldEnabled, depthOfFieldEnabled));
      depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());

      posterizeNP.set_shader_input("enabled", LVecBase2(posterizeEnabled, posterizeEnabled));
      posterizeCamera->set_initial_state(posterizeNP.get_state());

      pixelizeNP.set_shader_input("enabled", LVecBase2(pixelizeEnabled, pixelizeEnabled));
      pixelizeCamera->set_initial_state(pixelizeNP.get_state());
    };

  auto beforeFrameRunner = [](GenericAsyncTask* task, void* arg) -> AsyncTask::DoneStatus {
    (*static_cast<decltype(beforeFrame)*>(arg))();
    return AsyncTask::DS_cont;
  };

  taskManager->add
    ( new GenericAsyncTask
        ( "beforeFrame"
        , beforeFrameRunner
        , &beforeFrame
        )
    );

  framework.main_loop();
  framework.close_framework();

  return 0;
}

// END MAIN

float generateLightColorPart
  ( float amount
  , float intensity
  ) {
  return amount / 255.0 * intensity;
}

void generateLights
  ( NodePath render
  , bool showLights
  ) {
  PT(AmbientLight) ambientLight = new AmbientLight("ambientLight");
  ambientLight->set_color(
    LVecBase4
      ( generateLightColorPart(99, 1)
      , generateLightColorPart(91, 1)
      , generateLightColorPart(114, 1)
      , 1
      )
  );
  NodePath ambientLightNP = render.attach_new_node(ambientLight);
  render.set_light(ambientLightNP);

  PT(DirectionalLight) sunLight = new DirectionalLight("sunLight");
  sunLight->set_color(
    LVecBase4f
      ( generateLightColorPart(214, 0.7)
      , generateLightColorPart(149, 0.5)
      , generateLightColorPart(113, 0.2)
      , 1
      )
  );
  sunLight->set_shadow_caster(true, 2048, 2048);
  sunLight->get_lens()->set_film_size(30, 30);
  sunLight->get_lens()->set_near_far(0.1, 30);
  if (showLights) sunLight->show_frustum();
  NodePath sunLightNP = render.attach_new_node(sunLight);
  sunLightNP.set_pos(9, 12, 12);
  sunLightNP.set_hpr(135, -20, 0);
  render.set_light(sunLightNP);

  generateWindowLight(
    "windowLight",
    render,
    LVecBase3
      ( 1.5
      , 2.21
      , 7.9
      ),
    showLights
  );
  generateWindowLight(
    "windowLight1",
    render,
    LVecBase3
      ( 3.5
      , 2.21
      , 7.9
      ),
    showLights
  );
  generateWindowLight(
    "windowLight2",
    render,
    LVecBase3
      ( 3.5
      , 1.1
      , 4.5
      ),
    showLights
  );
}

void generateWindowLight
  ( std::string name
  , NodePath render
  , LVecBase3 position
  , bool show
  ) {
  LVecBase4f windowLightColor =
    LVecBase4f
      ( generateLightColorPart(255, 0.33)
      , generateLightColorPart(215, 0.33)
      , generateLightColorPart(188, 0.33)
      , 1
      );
  PT(Spotlight) windowLight = new Spotlight(name);
  windowLight->set_color(windowLightColor);
  windowLight->set_exponent(0.5);
  windowLight->set_attenuation(LVecBase3(1, 0.008, 0));
  windowLight->set_max_distance(37);

  PT(PerspectiveLens) windowLightLens = new PerspectiveLens();
  windowLightLens->set_near_far(0.5, 12);
  windowLightLens->set_fov(175);
  windowLight->set_lens(windowLightLens);

  if (show) windowLight->show_frustum();

  windowLight->set_shadow_caster(true, 1024, 1024);

  NodePath windowLightNP = render.attach_new_node(windowLight);
  windowLightNP.set_pos(position);
  windowLightNP.set_hpr(180, 0, 0);
  render.set_light(windowLightNP);
}

PT(Shader) loadShader
  ( std::string vert
  , std::string frag
  ) {
  return Shader::load
    ( Shader::SL_GLSL
    , "shaders/vertex/"   + vert + ".vert"
    , "shaders/fragment/" + frag + ".frag"
    );
}

PTA_LVecBase3f generateSsaoSamples
  ( int numberOfSamples
  ) {
  auto lerp = [](float a, float b, float f) -> float {
    return a + f * (b - a);
  };

  PTA_LVecBase3f ssaoSamples = PTA_LVecBase3f();

  for (int i = 0; i < numberOfSamples; ++i) {
    LVecBase3f sample = LVecBase3f(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator)
    ).normalized();

    float rand = randomFloats(generator);
    sample[0] *= rand;
    sample[1] *= rand;
    sample[2] *= rand;

    float scale = (float) i / (float) numberOfSamples;
    scale = lerp(0.1, 1.0, scale * scale);
    sample[0] *= scale;
    sample[1] *= scale;
    sample[2] *= scale;

    ssaoSamples.push_back(sample);
  }

  return ssaoSamples;
}

PTA_LVecBase3f generateSsaoNoise
  (
  ) {
  PTA_LVecBase3f ssaoNoise = PTA_LVecBase3f();

  for (int i = 0; i < 16; ++i) {
    LVecBase3f noise = LVecBase3f(
      randomFloats(generator) * 2.0 - 1.0,
      randomFloats(generator) * 2.0 - 1.0,
      0.0
    );

    ssaoNoise.push_back(noise);
  }

  return ssaoNoise;
}

FramebufferTexture generateHighPrecisionFramebufferTextureForScene
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , std::string name
  ) {
  FrameBufferProperties fbp = FrameBufferProperties::get_default();
  fbp.set_back_buffers(0);
  fbp.set_rgba_bits(32, 32, 32, 32);
  fbp.set_rgb_color(true);
  fbp.set_float_color(true);

  PT(GraphicsOutput) buffer =
    graphicsEngine
      ->make_output
        ( graphicsOutput->get_pipe()
        , name + "Buffer"
        , -3000
        , fbp
        , WindowProperties::size(0, 0),
            GraphicsPipe::BF_refuse_window
          | GraphicsPipe::BF_resizeable
          | GraphicsPipe::BF_can_bind_every
          | GraphicsPipe::BF_rtt_cumulative
          | GraphicsPipe::BF_size_track_host
        , graphicsOutput->get_gsg()
        , graphicsOutput->get_host()
        );
  buffer->add_render_texture
    ( NULL
    , GraphicsOutput::RTM_bind_or_copy
    , GraphicsOutput::RTP_color
    );
  buffer->set_clear_color(LVecBase4f(0, 0, 0, 0));

  NodePath cameraNP = window->make_camera();
  PT(Camera) camera = DCAST(Camera, cameraNP.node());
  camera->set_lens(window->get_camera(0)->get_lens());

  PT(DisplayRegion) bufferRegion = buffer->make_display_region(0, 1, 0, 1);
  bufferRegion->set_camera(cameraNP);

  NodePath shaderNP = NodePath(name + "Shader");

  FramebufferTexture result;
  result.buffer = buffer;
  result.camera = camera;
  result.cameraNP = cameraNP;
  result.shaderNP = shaderNP;
  return result;
}

FramebufferTexture generateFramebufferTextureForScene
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , std::string name
  , LColor clearColor
  ) {
  PT(GraphicsOutput) buffer =
    graphicsOutput
      ->make_texture_buffer(name + "Buffer", 0, 0);
  buffer->set_sort(-3000);
  buffer->set_clear_color(clearColor);

  NodePath cameraNP = window->make_camera();
  PT(Camera) camera = DCAST(Camera, cameraNP.node());
  camera->set_lens(window->get_camera(0)->get_lens());

  PT(DisplayRegion) bufferRegion = buffer->make_display_region(0, 1, 0, 1);
  bufferRegion->set_camera(cameraNP);

  NodePath shaderNP = NodePath(name + "Shader");

  FramebufferTexture result;
  result.buffer = buffer;
  result.camera = camera;
  result.cameraNP = cameraNP;
  result.shaderNP = shaderNP;
  return result;
}

FramebufferTexture generateFramebufferTextureForImage
  ( PT(GraphicsOutput) graphicsOutput
  , std::string name
  , LColor clearColor
  ) {
  PT(GraphicsOutput) buffer =
    graphicsOutput
      ->make_texture_buffer(name + "Buffer", 0, 0);
  buffer->set_sort(-3000);
  buffer->set_clear_color(clearColor);

  PT(Camera) camera = new Camera(name + "Camera");
  PT(OrthographicLens) lens = new OrthographicLens();
  lens->set_film_size(2, 2);
  lens->set_film_offset(0, 0);
  lens->set_near_far(-1, 1);
  camera->set_lens(lens);
  NodePath cameraNP = NodePath(camera);

  PT(DisplayRegion) bufferRegion = buffer->make_display_region(0, 1, 0, 1);
  bufferRegion->set_camera(cameraNP);

  NodePath shaderNP = NodePath(name + "Shader");

  NodePath renderNP = NodePath(name + "Render");

  cameraNP.reparent_to(renderNP);

  CardMaker card = CardMaker(name);
  card.set_frame_fullscreen_quad();
  card.set_has_uvs(true);
  NodePath cardNP = NodePath(card.generate());
  cardNP.reparent_to(renderNP);
  cardNP.set_pos(0, 0, 0);
  cardNP.set_hpr(0, 0, 0);
  cameraNP.look_at(cardNP);

  FramebufferTexture result;
  result.buffer = buffer;
  result.camera = camera;
  result.cameraNP = cameraNP;
  result.shaderNP = shaderNP;
  return result;
}

void showBuffer
  ( NodePath render2d
  , PT(GraphicsOutput) buffer
  , bool alpha
  ) {
  NodePath nodePath = buffer->get_texture_card();
  nodePath.reparent_to(render2d);
  if (alpha) nodePath.set_transparency(TransparencyAttrib::Mode::M_alpha);
}

int microsecondsSinceEpoch
  (
  ) {
  return std::chrono::duration_cast<std::chrono::microseconds>(
    std::chrono::system_clock::now().time_since_epoch()
  ).count();
}

double angleIncAndLimit
  ( double angle
  , double inc
  , double limit
  ) {
  angle += inc;
  if (angle > limit) { angle = 0; };
  return angle;
}

double angleDecAndLimit
  ( double angle
  , double dec
  , double limit
  ) {
  angle -= dec;
  if (angle < limit) { angle = 360; };
  return angle;
}

bool isButtonDown
  ( WindowFramework* window
  , std::string character
  ) {
  return
      getMouseWatcher(window)
    ->is_button_down(ButtonRegistry::ptr()
    ->find_button(character));
}

PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  ) {
  return DCAST
    ( MouseWatcher
    , window->get_mouse().node()
    );
}
