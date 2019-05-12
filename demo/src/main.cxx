/*
  (C) 2019 David Lettier
  lettier.com
*/

#include <thread>
#include <unistd.h>
#include <random>
#include <string>
#include <algorithm>

#include "pandaFramework.h" // Panda3D 1.10.1-1
#include "load_prc_file.h"
#include "pandaSystem.h"
#include "mouseButton.h"
#include "mouseWatcher.h"
#include "buttonRegistry.h"
#include "orthographicLens.h"
#include "ambientLight.h"
#include "directionalLight.h"
#include "pointLight.h"
#include "spotlight.h"
#include "shader.h"
#include "cardMaker.h"
#include "texturePool.h"
#include "audioManager.h"
#include "audioSound.h"

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

float toColorRange
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

FramebufferTexture generateFramebufferTexture
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , LVecBase4 rgbaBits
  , bool setFloatColor
  , std::string name
  , LColor clearColor
  , bool useScene
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

void hideBuffer
  ( NodePath render2d
  );

int microsecondsSinceEpoch
  (
  );

bool isButtonDown
  ( PT(MouseWatcher) mouseWatcher
  , std::string character
  );

PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  );

void setSoundOff
  ( PT(AudioSound) sound
  );

void setSoundOn
  ( PT(AudioSound) sound
  );

void setSoundState
  ( PT(AudioSound) sound
  , bool on
  );

void updateAudoManager
  ( NodePath sceneRootNP
  , NodePath cameraNP
  );

LVecBase3f calculateCameraPosition
  ( double radius
  , double phi
  , double theta
  , LVecBase3 cameraLookAt
  );

LVecBase3f calculateCameraLookAt
  ( double upDownAdjust
  , double leftRightAdjust
  , double phi
  , double theta
  , LVecBase3 lookAt
  );

double microsecondToSecond
  ( int m
  );

double toRadians
  ( double d
  );

LVecBase2 makeEnabledVec
  ( int t
  );

LVecBase2 toggleEnabledVec
  ( LVecBase2 vec
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

PT(AudioManager) audioManager = AudioManager::create_AudioManager();

// END GLOBALS

// MAIN

int main
  ( int argc
  , char *argv[]
  ) {
  LColor backgroundColor [] =
    { LColor
        ( toColorRange(99,  1)
        , toColorRange(108, 1)
        , toColorRange(114, 1)
        , 1
        )
    , LColor
        ( toColorRange(209, 1)
        , toColorRange(145, 1)
        , toColorRange(109, 1)
        , 1
        )
    };

  double cameraRotatePhiInitial    =   66;
  double cameraRotateThetaInitial  =  236;
  double cameraRotateRadiusInitial = 1486;
  LVecBase3 cameraLookAtInitial    = LVecBase3(0, 0.5, 3);
  int cameraNear                   =  450;
  int cameraFar                    = 2000;
  double cameraRotateRadius        = cameraRotateRadiusInitial;
  double cameraRotatePhi           = cameraRotatePhiInitial;
  double cameraRotateTheta         = cameraRotateThetaInitial;
  LVecBase3 cameraLookAt           = cameraLookAtInitial;

  float fogNearInitial = cameraRotateRadiusInitial - 1;
  float fogFarInitial  = fogNearInitial            + 8;
  float fogNear        = fogNearInitial;
  float fogFar         = fogFarInitial;
  float fogAdjust      = 0.1;

  LVecBase2 foamDepthInitial = LVecBase2(4, 4);
  float     foamDepthAdjust  = 0.1;
  LVecBase2 foamDepth        = foamDepthInitial;

  LVecBase2 mouseThen = LVecBase2(0, 0);
  LVecBase2 mouseNow  = mouseThen;
  bool mouseWheelDown = false;
  bool mouseWheelUp   = false;

  LVecBase2f riorInitial = LVecBase2f(1.05, 1.05);
  float      riorAdjust  = 0.005;
  LVecBase2f rior        = riorInitial;

  bool soundEnabled = true;

  LVecBase2 ssaoEnabled         = makeEnabledVec(1);
  LVecBase2 refractionEnabled   = makeEnabledVec(1);
  LVecBase2 reflectionEnabled   = makeEnabledVec(1);
  LVecBase2 fogEnabled          = makeEnabledVec(1);
  LVecBase2 outlineEnabled      = makeEnabledVec(1);
  LVecBase2 normalMapsEnabled   = makeEnabledVec(1);
  LVecBase2 bloomEnabled        = makeEnabledVec(1);
  LVecBase2 sharpenEnabled      = makeEnabledVec(1);
  LVecBase2 depthOfFieldEnabled = makeEnabledVec(1);
  LVecBase2 filmGrainEnabled    = makeEnabledVec(1);
  LVecBase2 flowMapsEnabled     = makeEnabledVec(1);
  LVecBase2 posterizeEnabled    = makeEnabledVec(0);
  LVecBase2 pixelizeEnabled     = makeEnabledVec(0);

  LVecBase4 rgba8  = ( 8,  8,  8,  8);
  LVecBase4 rgba16 = (16, 16, 16, 16);
  LVecBase4 rgba32 = (32, 32, 32, 32);

  load_prc_file("config.prc");

  std::vector<PT(AudioSound)> sounds =
    { audioManager->get_sound("sounds/wheel.ogg", true)
    , audioManager->get_sound("sounds/water.ogg", true)
    };

  PandaFramework framework;
  framework.open_framework(argc, argv);
  framework.set_window_title("3D Game Shaders For Beginners By David Lettier");
  PT(WindowFramework) window = framework.open_window();

  window->enable_keyboard();

  PT(DisplayRegion) displayRegion3d = window->get_display_region_3d();
  displayRegion3d->set_clear_color_active(true);
  displayRegion3d->set_clear_depth_active(true);
  displayRegion3d->set_clear_stencil_active(true);
  displayRegion3d->set_clear_color(backgroundColor[1]);
  displayRegion3d->set_clear_depth(1.0f);
  displayRegion3d->set_clear_stencil(0);

  NodePath render   = window->get_render();
  NodePath render2d = window->get_render_2d();

  PT(MouseWatcher) mouseWatcher = getMouseWatcher(window);

  PT(Lens) lens = window->get_camera(0)->get_lens();
  lens->set_fov(1);
  lens->set_near_far(cameraNear, cameraFar);

  NodePath cameraNP = window->get_camera_group();

  cameraNP.set_pos
    ( calculateCameraPosition
        ( cameraRotateRadius
        , cameraRotatePhi
        , cameraRotateTheta
        , cameraLookAt
        )
    );
  cameraNP.look_at(cameraLookAt);

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

  NodePath wheelNP   = environmentNP.find("**/wheel-lp");
  NodePath waterNP   = environmentNP.find("**/water-lp");
  NodePath gridFloor = environmentNP.find("**/grid-floor");

  wheelNP.set_two_sided(true);
  gridFloor.set_two_sided(true);

  gridFloor.set_light_off();

  generateLights(render, false);

  PT(Texture) blackTexture       = TexturePool::load_texture("eggs/mill-scene/tex/black.png");
  PT(Texture) foamPatternTexture = TexturePool::load_texture("eggs/mill-scene/tex/foam-pattern.png");
  PT(Texture) stillFlowTexture   = TexturePool::load_texture("eggs/mill-scene/tex/still-flow.png");
  PT(Texture) upFlowTexture      = TexturePool::load_texture("eggs/mill-scene/tex/up-flow.png");

  PT(Shader) baseShader                  = loadShader("base",  "base");
  PT(Shader) materialDiffuseShader       = loadShader("base",  "material-diffuse");
  PT(Shader) materialSpecularShader      = loadShader("base",  "material-specular");
  PT(Shader) positionShader              = loadShader("base",  "position");
  PT(Shader) normalShader                = loadShader("base",  "normal");
  PT(Shader) foamMaskShader              = loadShader("base",  "foam-mask");
  PT(Shader) foamShader                  = loadShader("basic", "foam");
  PT(Shader) fogShader                   = loadShader("basic", "fog");
  PT(Shader) boxBlurShader               = loadShader("basic", "box-blur");
  PT(Shader) medianFilterShader          = loadShader("basic", "median-filter");
  PT(Shader) kuwaharaFilterShader        = loadShader("basic", "kuwahara-filter");
  PT(Shader) sharpenShader               = loadShader("basic", "sharpen");
  PT(Shader) outlineShader               = loadShader("basic", "outline");
  PT(Shader) bloomShader                 = loadShader("basic", "bloom");
  PT(Shader) ssaoShader                  = loadShader("basic", "ssao");
  PT(Shader) screenSpaceRefractionShader = loadShader("basic", "screen-space-refraction");
  PT(Shader) screenSpaceReflectionShader = loadShader("basic", "screen-space-reflection");
  PT(Shader) refractionShader            = loadShader("basic", "refraction");
  PT(Shader) reflectionColorShader       = loadShader("basic", "reflection-color");
  PT(Shader) reflectionShader            = loadShader("basic", "reflection");
  PT(Shader) baseCombineShader           = loadShader("basic", "base-combine");
  PT(Shader) sceneCombineShader          = loadShader("basic", "scene-combine");
  PT(Shader) depthOfFieldShader          = loadShader("basic", "depth-of-field");
  PT(Shader) posterizeShader             = loadShader("basic", "posterize");
  PT(Shader) pixelizeShader              = loadShader("basic", "pixelize");
  PT(Shader) filmGrainShader             = loadShader("basic", "film-grain");

  PT(GraphicsOutput) graphicsOutput = window->get_graphics_output();
  PT(GraphicsEngine) graphicsEngine = graphicsOutput->get_gsg()->get_engine();

  FramebufferTexture positionFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba32
      , true
      , "positionBuffer"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) positionBuffer = positionFramebufferTexture.buffer;
  PT(Camera)         positionCamera = positionFramebufferTexture.camera;
  NodePath           positionNP     = positionFramebufferTexture.shaderNP;
  positionNP.set_shader(positionShader);
  positionCamera->set_initial_state(positionNP.get_state());
  PT(Texture) positionTexture    = positionBuffer->get_texture();
  PT(Lens)    positionCameraLens = positionCamera->get_lens();
  positionCamera->set_camera_mask(BitMask32::bit(30));
  waterNP.hide(BitMask32::bit(30));

  FramebufferTexture positionWithWaterFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba32
      , true
      , "positionWithWaterBuffer"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) positionWithWaterBuffer = positionWithWaterFramebufferTexture.buffer;
  PT(Camera)         positionWithWaterCamera = positionWithWaterFramebufferTexture.camera;
  NodePath           positionWithWaterNP     = positionWithWaterFramebufferTexture.shaderNP;
  positionWithWaterBuffer->set_sort(positionBuffer->get_sort() + 1);
  positionWithWaterNP.set_shader(positionShader);
  positionWithWaterCamera->set_initial_state(positionWithWaterNP.get_state());
  PT(Texture) positionWithWaterTexture    = positionWithWaterBuffer->get_texture();
  PT(Lens)    positionWithWaterCameraLens = positionWithWaterCamera->get_lens();

  FramebufferTexture normalFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba32
      , true
      , "normalBuffer"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) normalBuffer = normalFramebufferTexture.buffer;
  PT(Camera)         normalCamera = normalFramebufferTexture.camera;
  NodePath           normalNP     = normalFramebufferTexture.shaderNP;
  normalNP.set_shader(normalShader);
  normalNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  normalNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
  normalNP.set_shader_input("flowTexture",       stillFlowTexture);
  normalCamera->set_initial_state(normalNP.get_state());
  PT(Texture) normalTexture = normalFramebufferTexture.buffer->get_texture();
  normalCamera->set_camera_mask(BitMask32::bit(30));
  waterNP.hide(BitMask32::bit(30));

  FramebufferTexture normalWithWaterFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba32
      , true
      , "normalWithWaterBuffer"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) normalWithWaterBuffer = normalWithWaterFramebufferTexture.buffer;
  PT(Camera)         normalWithWaterCamera = normalWithWaterFramebufferTexture.camera;
  NodePath           normalWithWaterNP     = normalWithWaterFramebufferTexture.shaderNP;
  normalWithWaterBuffer->set_sort(normalBuffer->get_sort() + 1);
  normalWithWaterNP.set_shader(normalShader);
  normalWithWaterNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  normalWithWaterNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
  normalWithWaterNP.set_shader_input("flowTexture",       stillFlowTexture);
  normalWithWaterCamera->set_initial_state(normalWithWaterNP.get_state());
  NodePath normalWithAnimatedWaterNP = NodePath("normalWithAnimatedWater");
  normalWithAnimatedWaterNP.set_shader(normalShader);
  normalWithAnimatedWaterNP.set_shader_input("flowTexture", upFlowTexture);
  normalWithWaterCamera->set_tag_state_key("waterNormal");
  normalWithWaterCamera->set_tag_state("waterNormalTrue", normalWithAnimatedWaterNP.get_state());
  waterNP.set_tag("waterNormal", "waterNormalTrue");
  PT(Texture) normalWithWaterTexture = normalWithWaterFramebufferTexture.buffer->get_texture();

  FramebufferTexture materialDiffuseFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "materialDiffuse"
      , LColor(0, 0, 0, 1)
      , true
      );
  PT(GraphicsOutput) materialDiffuseBuffer = materialDiffuseFramebufferTexture.buffer;
  PT(Camera)         materialDiffuseCamera = materialDiffuseFramebufferTexture.camera;
  NodePath           materialDiffuseNP     = materialDiffuseFramebufferTexture.shaderNP;
  materialDiffuseNP.set_shader(materialDiffuseShader);
  materialDiffuseCamera->set_initial_state(materialDiffuseNP.get_state());

  FramebufferTexture materialSpecularFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "materialSpecular"
      , LColor(0, 0, 0, 1)
      , true
      );
  PT(GraphicsOutput) materialSpecularBuffer = materialSpecularFramebufferTexture.buffer;
  PT(Camera)         materialSpecularCamera = materialSpecularFramebufferTexture.camera;
  NodePath           materialSpecularNP     = materialSpecularFramebufferTexture.shaderNP;
  materialSpecularNP.set_shader(materialSpecularShader);
  materialSpecularCamera->set_initial_state(materialSpecularNP.get_state());
  PT(Texture) materialSpecularTexture = materialSpecularBuffer->get_texture();

  FramebufferTexture foamMaskFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "foamMask"
      , LColor(0, 0, 0, 1)
      , true
      );
  PT(GraphicsOutput) foamMaskBuffer = foamMaskFramebufferTexture.buffer;
  PT(Camera)         foamMaskCamera = foamMaskFramebufferTexture.camera;
  NodePath           foamMaskNP     = foamMaskFramebufferTexture.shaderNP;
  foamMaskNP.set_shader(foamMaskShader);
  foamMaskNP.set_shader_input("foamPatternTexture", blackTexture);
  foamMaskNP.set_shader_input("flowMapsEnabled",    flowMapsEnabled);
  foamMaskNP.set_shader_input("flowTexture",        stillFlowTexture);
  foamMaskCamera->set_initial_state(foamMaskNP.get_state());
  NodePath foamMaskWithAnimatedWaterNP = NodePath("foamMaskWithAnimatedWater");
  foamMaskWithAnimatedWaterNP.set_shader(foamMaskShader);
  foamMaskWithAnimatedWaterNP.set_shader_input("flowTexture",        upFlowTexture);
  foamMaskWithAnimatedWaterNP.set_shader_input("foamPatternTexture", foamPatternTexture);
  foamMaskCamera->set_tag_state_key("waterFoamMask");
  foamMaskCamera->set_tag_state("waterFoamMaskTrue", foamMaskWithAnimatedWaterNP.get_state());
  waterNP.set_tag("waterFoamMask", "waterFoamMaskTrue");
  PT(Texture) foamMaskTexture = foamMaskBuffer->get_texture();

  FramebufferTexture fogFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "fog"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) fogBuffer = fogFramebufferTexture.buffer;
  PT(Camera)         fogCamera = fogFramebufferTexture.camera;
  NodePath           fogNP     = fogFramebufferTexture.shaderNP;
  fogBuffer->set_sort(positionWithWaterBuffer->get_sort() + 1);
  fogNP.set_shader(fogShader);
  fogNP.set_shader_input("backgroundColor0", backgroundColor[0]);
  fogNP.set_shader_input("backgroundColor1", backgroundColor[1]);
  fogNP.set_shader_input("positionTexture",  positionWithWaterTexture);
  fogNP.set_shader_input("enabled",          fogEnabled);
  fogNP.set_shader_input("nearFar",          LVecBase2(fogNear, fogFar));
  fogCamera->set_initial_state(fogNP.get_state());
  PT(Texture) fogTexture = fogBuffer->get_texture();

  FramebufferTexture ssaoFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "ssao"
      , LColor(1, 1, 1, 0)
      , false
      );
  PT(GraphicsOutput) ssaoBuffer = ssaoFramebufferTexture.buffer;
  PT(Camera)         ssaoCamera = ssaoFramebufferTexture.camera;
  NodePath           ssaoNP     = ssaoFramebufferTexture.shaderNP;
  ssaoNP.set_shader(ssaoShader);
  ssaoNP.set_shader_input("positionTexture", positionTexture);
  ssaoNP.set_shader_input("normalTexture",   normalTexture);
  ssaoNP.set_shader_input("samples",         generateSsaoSamples(64));
  ssaoNP.set_shader_input("noise",           generateSsaoNoise());
  ssaoNP.set_shader_input("lensProjection",  positionCameraLens->get_projection_mat());
  ssaoNP.set_shader_input("enabled",         ssaoEnabled);
  ssaoCamera->set_initial_state(ssaoNP.get_state());

  FramebufferTexture ssaoBlurFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "ssaoBlur"
      , LColor(1, 1, 1, 0)
      , false
      );
  PT(GraphicsOutput) ssaoBlurBuffer = ssaoBlurFramebufferTexture.buffer;
  NodePath           ssaoBlurNP     = ssaoBlurFramebufferTexture.shaderNP;
  ssaoBlurNP.set_shader(medianFilterShader);
  ssaoBlurNP.set_shader_input("colorTexture", ssaoBuffer->get_texture());
  ssaoBlurNP.set_shader_input("parameters",   LVecBase2(3, 10));
  ssaoBlurFramebufferTexture.camera->set_initial_state(ssaoBlurNP.get_state());
  PT(Texture) ssaoBlurTexture = ssaoBlurBuffer->get_texture();

  FramebufferTexture refractionUvFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba16
      , false
      , "refractionUv"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) refractionUvBuffer = refractionUvFramebufferTexture.buffer;
  PT(Camera)         refractionUvCamera = refractionUvFramebufferTexture.camera;
  NodePath           refractionUvNP     = refractionUvFramebufferTexture.shaderNP;
  refractionUvBuffer->set_sort(ssaoBlurBuffer->get_sort() + 1);
  refractionUvNP.set_shader(screenSpaceRefractionShader);
  refractionUvNP.set_shader_input("positionFromTexture", positionWithWaterTexture);
  refractionUvNP.set_shader_input("positionToTexture",   positionTexture);
  refractionUvNP.set_shader_input("normalFromTexture",   normalWithWaterTexture);
  refractionUvNP.set_shader_input("lensProjection",      positionWithWaterCameraLens->get_projection_mat());
  refractionUvNP.set_shader_input("enabled",             refractionEnabled);
  refractionUvNP.set_shader_input("rior",                rior);
  refractionUvCamera->set_initial_state(refractionUvNP.get_state());
  PT(Texture) refractionUvTexture = refractionUvBuffer->get_texture();

  FramebufferTexture reflectionUvFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba16
      , false
      , "reflectionUv"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) reflectionUvBuffer = reflectionUvFramebufferTexture.buffer;
  PT(Camera)         reflectionUvCamera = reflectionUvFramebufferTexture.camera;
  NodePath           reflectionUvNP     = reflectionUvFramebufferTexture.shaderNP;
  reflectionUvBuffer->set_sort(refractionUvBuffer->get_sort() + 1);
  reflectionUvNP.set_shader(screenSpaceReflectionShader);
  reflectionUvNP.set_shader_input("positionTexture", positionWithWaterTexture);
  reflectionUvNP.set_shader_input("normalTexture",   normalWithWaterTexture);
  reflectionUvNP.set_shader_input("lensProjection",  positionWithWaterCameraLens->get_projection_mat());
  reflectionUvNP.set_shader_input("enabled",         reflectionEnabled);
  reflectionUvCamera->set_initial_state(reflectionUvNP.get_state());
  PT(Texture) reflectionUvTexture = reflectionUvBuffer->get_texture();

  FramebufferTexture outlineFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "outline"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) outlineBuffer = outlineFramebufferTexture.buffer;
  PT(Camera)         outlineCamera = outlineFramebufferTexture.camera;
  NodePath           outlineNP     = outlineFramebufferTexture.shaderNP;
  outlineBuffer->set_sort(fogBuffer->get_sort() + 1);
  outlineNP.set_shader(outlineShader);
  outlineNP.set_shader_input("materialDiffuseTexture", materialDiffuseBuffer->get_texture());
  outlineNP.set_shader_input("fogTexture",             fogTexture);
  outlineNP.set_shader_input("enabled",                outlineEnabled);
  outlineCamera->set_initial_state(outlineNP.get_state());
  PT(Texture) outlineTexture = outlineBuffer->get_texture();

  FramebufferTexture baseFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "base"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) baseBuffer = baseFramebufferTexture.buffer;
  PT(Camera)         baseCamera = baseFramebufferTexture.camera;
  NodePath           baseNP     = baseFramebufferTexture.shaderNP;
  baseBuffer->set_sort(ssaoBlurBuffer->get_sort() + 1);
  baseNP.set_shader(baseShader);
  baseNP.set_shader_input("ssaoBlurTexture",   ssaoBlurTexture);
  baseNP.set_shader_input("flowTexture",       stillFlowTexture);
  baseNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  baseNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
  baseNP.set_shader_input("specularOnly",      LVecBase2(0, 0));
  baseCamera->set_initial_state(baseNP.get_state());
  PT(Texture) baseTexture = baseBuffer->get_texture();
  baseCamera->set_camera_mask(BitMask32::bit(0));
  waterNP.hide(BitMask32::bit(0));

  FramebufferTexture specularFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "specular"
      , LColor(0, 0, 0, 0)
      , true
      );
  PT(GraphicsOutput) specularBuffer = specularFramebufferTexture.buffer;
  PT(Camera)         specularCamera = specularFramebufferTexture.camera;
  NodePath           specularNP     = specularFramebufferTexture.shaderNP;
  specularBuffer->set_sort(baseBuffer->get_sort() + 1);
  specularNP.set_shader(baseShader);
  specularNP.set_shader_input("ssaoBlurTexture",   ssaoBlurTexture);
  specularNP.set_shader_input("flowTexture",       stillFlowTexture);
  specularNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
  specularNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
  specularNP.set_shader_input("specularOnly",      LVecBase2(1, 1));
  specularCamera->set_initial_state(specularNP.get_state());
  PT(Texture) specularTexture = specularBuffer->get_texture();
  NodePath specularWithAnimatedWaterNP = NodePath("specularWithAnimatedWater");
  specularWithAnimatedWaterNP.set_shader(baseShader);
  specularWithAnimatedWaterNP.set_shader_input("flowTexture", upFlowTexture);
  specularCamera->set_tag_state_key("waterSpecular");
  specularCamera->set_tag_state("waterSpecularTrue", specularWithAnimatedWaterNP.get_state());
  waterNP.set_tag("waterSpecular", "waterSpecularTrue");

  FramebufferTexture refractionFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "refraction"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) refractionBuffer = refractionFramebufferTexture.buffer;
  PT(Camera)         refractionCamera = refractionFramebufferTexture.camera;
  NodePath           refractionNP     = refractionFramebufferTexture.shaderNP;
  refractionBuffer->set_sort(specularBuffer->get_sort() + 1);
  refractionNP.set_shader(refractionShader);
  refractionNP.set_shader_input("uvTexture",              refractionUvTexture);
  refractionNP.set_shader_input("maskTexture",            materialSpecularTexture);
  refractionNP.set_shader_input("positionFromTexture",    positionWithWaterTexture);
  refractionNP.set_shader_input("positionToTexture",      positionTexture);
  refractionNP.set_shader_input("backgroundColorTexture", baseTexture);
  refractionCamera->set_initial_state(refractionNP.get_state());
  PT(Texture) refractionTexture = refractionBuffer->get_texture();

  FramebufferTexture foamFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "foam"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) foamBuffer = foamFramebufferTexture.buffer;
  PT(Camera)         foamCamera = foamFramebufferTexture.camera;
  NodePath           foamNP     = foamFramebufferTexture.shaderNP;
  foamBuffer->set_sort(refractionBuffer->get_sort() + 1);
  foamNP.set_shader(foamShader);
  foamNP.set_shader_input("maskTexture",         foamMaskTexture);
  foamNP.set_shader_input("foamDepth",           foamDepth);
  foamNP.set_shader_input("positionFromTexture", positionWithWaterTexture);
  foamNP.set_shader_input("positionToTexture",   positionTexture);
  foamCamera->set_initial_state(foamNP.get_state());
  PT(Texture) foamTexture = foamBuffer->get_texture();

  FramebufferTexture reflectionColorFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "reflectionColor"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) reflectionColorBuffer = reflectionColorFramebufferTexture.buffer;
  PT(Camera)         reflectionColorCamera = reflectionColorFramebufferTexture.camera;
  NodePath           reflectionColorNP     = reflectionColorFramebufferTexture.shaderNP;
  reflectionColorBuffer->set_sort(foamBuffer->get_sort() + 1);
  reflectionColorNP.set_shader(reflectionColorShader);
  reflectionColorNP.set_shader_input("colorTexture", baseTexture);
  reflectionColorNP.set_shader_input("uvTexture",    reflectionUvTexture);
  reflectionColorCamera->set_initial_state(reflectionColorNP.get_state());
  PT(Texture) reflectionColorTexture = reflectionColorBuffer->get_texture();

  FramebufferTexture reflectionColorBlurFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "reflectionColorBlur"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) reflectionColorBlurBuffer = reflectionColorBlurFramebufferTexture.buffer;
  PT(Camera)         reflectionColorBlurCamera = reflectionColorBlurFramebufferTexture.camera;
  NodePath           reflectionColorBlurNP     = reflectionColorBlurFramebufferTexture.shaderNP;
  reflectionColorBlurBuffer->set_sort(reflectionColorBuffer->get_sort() + 1);
  reflectionColorBlurNP.set_shader(boxBlurShader);
  reflectionColorBlurNP.set_shader_input("colorTexture", reflectionColorTexture);
  reflectionColorBlurNP.set_shader_input("parameters",   LVecBase2(8, 1));
  reflectionColorBlurCamera->set_initial_state(reflectionColorBlurNP.get_state());
  PT(Texture) reflectionColorBlurTexture = reflectionColorBlurBuffer->get_texture();

  FramebufferTexture reflectionFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "reflection"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) reflectionBuffer = reflectionFramebufferTexture.buffer;
  NodePath           reflectionNP     = reflectionFramebufferTexture.shaderNP;
  reflectionBuffer->set_sort(reflectionColorBlurBuffer->get_sort() + 1);
  reflectionNP.set_shader(reflectionShader);
  reflectionNP.set_shader_input("colorTexture",     reflectionColorTexture);
  reflectionNP.set_shader_input("colorBlurTexture", reflectionColorBlurTexture);
  reflectionNP.set_shader_input("specularTexture",  materialSpecularTexture);
  reflectionFramebufferTexture.camera->set_initial_state(reflectionNP.get_state());
  PT(Texture) reflectionTexture = reflectionBuffer->get_texture();

  FramebufferTexture baseCombineFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "baseCombine"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) baseCombineBuffer = baseCombineFramebufferTexture.buffer;
  PT(Camera)         baseCombineCamera = baseCombineFramebufferTexture.camera;
  NodePath           baseCombineNP     = baseCombineFramebufferTexture.shaderNP;
  baseCombineBuffer->set_sort(reflectionBuffer->get_sort() + 1);
  baseCombineNP.set_shader(baseCombineShader);
  baseCombineNP.set_shader_input("baseTexture",       baseTexture);
  baseCombineNP.set_shader_input("refractionTexture", refractionTexture);
  baseCombineNP.set_shader_input("foamTexture",       foamTexture);
  baseCombineNP.set_shader_input("reflectionTexture", reflectionTexture);
  baseCombineNP.set_shader_input("specularTexture",   specularTexture);
  baseCombineCamera->set_initial_state(baseCombineNP.get_state());
  PT(Texture) baseCombineTexture = baseCombineBuffer->get_texture();

  FramebufferTexture sharpenFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "sharpen"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) sharpenBuffer = sharpenFramebufferTexture.buffer;
  NodePath           sharpenNP     = sharpenFramebufferTexture.shaderNP;
  sharpenBuffer->set_sort(baseCombineBuffer->get_sort() + 1);
  sharpenNP.set_shader(sharpenShader);
  sharpenNP.set_shader_input("colorTexture", baseCombineTexture);
  sharpenNP.set_shader_input("enabled",      sharpenEnabled);
  PT(Camera) sharpenCamera = sharpenFramebufferTexture.camera;
  sharpenCamera->set_initial_state(sharpenNP.get_state());
  PT(Texture) sharpenTexture = sharpenBuffer->get_texture();

  FramebufferTexture posterizePreBlurFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "posterizePreBlur"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) posterizePreBlurBuffer = posterizePreBlurFramebufferTexture.buffer;
  NodePath           posterizePreBlurNP     = posterizePreBlurFramebufferTexture.shaderNP;
  posterizePreBlurBuffer->set_sort(sharpenBuffer->get_sort() + 1);
  posterizePreBlurNP.set_shader(kuwaharaFilterShader);
  posterizePreBlurNP.set_shader_input("colorTexture", sharpenTexture);
  posterizePreBlurNP.set_shader_input("parameters",   LVecBase2(0, 0));
  PT(Camera) posterizePreBlurCamera = posterizePreBlurFramebufferTexture.camera;
  posterizePreBlurCamera->set_initial_state(posterizePreBlurNP.get_state());
  PT(Texture) posterizePreBlurTexture = posterizePreBlurBuffer->get_texture();

  FramebufferTexture posterizeFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "posterize"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) posterizeBuffer = posterizeFramebufferTexture.buffer;
  NodePath           posterizeNP     = posterizeFramebufferTexture.shaderNP;
  posterizeBuffer->set_sort(posterizePreBlurBuffer->get_sort() + 1);
  posterizeNP.set_shader(posterizeShader);
  posterizeNP.set_shader_input("colorTexture",     posterizePreBlurTexture);
  posterizeNP.set_shader_input("positionTexture",  positionTexture);
  posterizeNP.set_shader_input("enabled",          posterizeEnabled);
  PT(Camera) posterizeCamera = posterizeFramebufferTexture.camera;
  posterizeCamera->set_initial_state(posterizeNP.get_state());
  PT(Texture) posterizeTexture = posterizeBuffer->get_texture();

  FramebufferTexture bloomFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "bloom"
      , LColor(0, 0, 0, 0)
      , false
      );
  PT(GraphicsOutput) bloomBuffer = bloomFramebufferTexture.buffer;
  PT(Camera)         bloomCamera = bloomFramebufferTexture.camera;
  NodePath           bloomNP     = bloomFramebufferTexture.shaderNP;
  bloomBuffer->set_sort(posterizeBuffer->get_sort() + 1);
  bloomNP.set_shader(bloomShader);
  bloomNP.set_shader_input("colorTexture", posterizeTexture);
  bloomNP.set_shader_input("enabled",      bloomEnabled);
  bloomCamera->set_initial_state(bloomNP.get_state());
  PT(Texture) bloomTexture = bloomBuffer->get_texture();

  FramebufferTexture sceneCombineFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "sceneCombine"
      , backgroundColor[1]
      , false
      );
  PT(GraphicsOutput) sceneCombineBuffer = sceneCombineFramebufferTexture.buffer;
  PT(Camera)         sceneCombineCamera = sceneCombineFramebufferTexture.camera;
  NodePath           sceneCombineNP     = sceneCombineFramebufferTexture.shaderNP;
  sceneCombineBuffer->set_sort(bloomBuffer->get_sort() + 1);
  sceneCombineNP.set_shader(sceneCombineShader);
  sceneCombineNP.set_shader_input("backgroundColor0", backgroundColor[0]);
  sceneCombineNP.set_shader_input("backgroundColor1", backgroundColor[1]);
  sceneCombineNP.set_shader_input("baseTexture",      posterizeTexture);
  sceneCombineNP.set_shader_input("bloomTexture",     bloomTexture);
  sceneCombineNP.set_shader_input("outlineTexture",   outlineTexture);
  sceneCombineNP.set_shader_input("fogTexture",       fogTexture);
  PT(Texture) sceneCombineTexture = sceneCombineBuffer->get_texture();
  sceneCombineCamera->set_initial_state(sceneCombineNP.get_state());

  FramebufferTexture outOfFocusFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "outOfFocus"
      , backgroundColor[1]
      , false
      );
  PT(GraphicsOutput) outOfFocusBuffer = outOfFocusFramebufferTexture.buffer;
  PT(Camera)         outOfFocusCamera = outOfFocusFramebufferTexture.camera;
  NodePath           outOfFocusNP     = outOfFocusFramebufferTexture.shaderNP;
  outOfFocusBuffer->set_sort(sceneCombineBuffer->get_sort() + 1);
  outOfFocusNP.set_shader(boxBlurShader);
  outOfFocusNP.set_shader_input("colorTexture", sceneCombineTexture);
  outOfFocusNP.set_shader_input("parameters",   LVecBase2(8, 1));
  outOfFocusCamera->set_initial_state(outOfFocusNP.get_state());
  PT(Texture) outOfFocusTexture = outOfFocusBuffer->get_texture();

  FramebufferTexture depthOfFieldFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "depthOfField"
      , backgroundColor[1]
      , false
      );
  PT(GraphicsOutput) depthOfFieldBuffer = depthOfFieldFramebufferTexture.buffer;
  NodePath           depthOfFieldNP     = depthOfFieldFramebufferTexture.shaderNP;
  depthOfFieldBuffer->set_sort(outOfFocusBuffer->get_sort() + 1);
  depthOfFieldNP.set_shader(depthOfFieldShader);
  depthOfFieldNP.set_shader_input("positionTexture",   positionTexture);
  depthOfFieldNP.set_shader_input("focusTexture",      sceneCombineTexture);
  depthOfFieldNP.set_shader_input("outOfFocusTexture", outOfFocusTexture);
  depthOfFieldNP
    .set_shader_input
      ( "focalLength"
      , LVecBase2f(environmentNP.get_distance(cameraNP), 0)
      );
  depthOfFieldNP.set_shader_input("enabled", depthOfFieldEnabled);
  PT(Camera) depthOfFieldCamera = depthOfFieldFramebufferTexture.camera;
  depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());

  FramebufferTexture pixelizeFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "pixelize"
      , backgroundColor[1]
      , false
      );
  PT(GraphicsOutput) pixelizeBuffer = pixelizeFramebufferTexture.buffer;
  NodePath           pixelizeNP     = pixelizeFramebufferTexture.shaderNP;
  pixelizeBuffer->set_sort(depthOfFieldBuffer->get_sort() + 1);
  pixelizeNP.set_shader(pixelizeShader);
  pixelizeNP.set_shader_input("colorTexture", depthOfFieldBuffer->get_texture());
  pixelizeNP.set_shader_input("enabled",      pixelizeEnabled);
  PT(Camera) pixelizeCamera = pixelizeFramebufferTexture.camera;
  pixelizeCamera->set_initial_state(pixelizeNP.get_state());
  PT(Texture) pixelizeTexture= pixelizeBuffer->get_texture();

  FramebufferTexture filmGrainFramebufferTexture =
    generateFramebufferTexture
      ( window
      , graphicsOutput
      , graphicsEngine
      , rgba8
      , false
      , "filmGrain"
      , backgroundColor[1]
      , false
      );
  PT(GraphicsOutput) filmGrainBuffer = filmGrainFramebufferTexture.buffer;
  NodePath           filmGrainNP     = filmGrainFramebufferTexture.shaderNP;
  filmGrainBuffer->set_sort(pixelizeBuffer->get_sort() + 1);
  filmGrainNP.set_shader(filmGrainShader);
  filmGrainNP.set_shader_input("colorTexture", pixelizeTexture);
  filmGrainNP.set_shader_input("enabled",      filmGrainEnabled);
  PT(Camera) filmGrainCamera = filmGrainFramebufferTexture.camera;
  filmGrainCamera->set_initial_state(filmGrainNP.get_state());

  int showBufferIndex = 0;

  std::vector <PT(GraphicsOutput)> bufferArray =
    { positionBuffer
    , positionWithWaterBuffer
    , normalBuffer
    , normalWithWaterBuffer
    , materialDiffuseBuffer
    , materialSpecularBuffer
    , ssaoBuffer
    , ssaoBlurBuffer
    , fogBuffer
    , outlineBuffer
    , baseBuffer
    , refractionUvBuffer
    , refractionBuffer
    , reflectionUvBuffer
    , reflectionColorBuffer
    , reflectionColorBlurBuffer
    , reflectionBuffer
    , foamMaskBuffer
    , foamBuffer
    , specularBuffer
    , baseCombineBuffer
    , posterizePreBlurBuffer
    , posterizeBuffer
    , bloomBuffer
    , sceneCombineBuffer
    , pixelizeBuffer
    , outOfFocusBuffer
    , depthOfFieldBuffer
    , filmGrainBuffer
    };

  showBufferIndex = bufferArray.size() - 1;

  showBuffer
    ( render2d
    , bufferArray[showBufferIndex]
    , false
    );

  int then    = microsecondsSinceEpoch();
  int now     = then;
  int keyTime = now;

  auto beforeFrame =
    [&]() -> void {
    double delta = microsecondToSecond(now - then);

    double timeSinceKey = microsecondToSecond(now - keyTime);
    bool   keyDebounced = timeSinceKey >= 0.1;

    then = now;
    now  = microsecondsSinceEpoch();

    double movement = 100 * delta;

    double cameraUpDownAdjust    = 0;
    double cameraLeftRightAdjust = 0;

    bool shiftDown        = isButtonDown(mouseWatcher, "shift");
    bool tabDown          = isButtonDown(mouseWatcher, "tab");

    bool resetDown        = isButtonDown(mouseWatcher, "r");

    bool fogNearDown      = isButtonDown(mouseWatcher, "[");
    bool fogFarDown       = isButtonDown(mouseWatcher, "]");

    bool equalDown        = isButtonDown(mouseWatcher, "=");
    bool minusDown        = isButtonDown(mouseWatcher, "-");

    bool deleteDown       = isButtonDown(mouseWatcher, "delete");

    bool wDown            = isButtonDown(mouseWatcher, "w");
    bool aDown            = isButtonDown(mouseWatcher, "a");
    bool dDown            = isButtonDown(mouseWatcher, "d");
    bool sDown            = isButtonDown(mouseWatcher, "s");
    bool zDown            = isButtonDown(mouseWatcher, "z");
    bool xDown            = isButtonDown(mouseWatcher, "x");

    bool arrowUpDown      = isButtonDown(mouseWatcher, "arrow_up");
    bool arrowDownDown    = isButtonDown(mouseWatcher, "arrow_down");
    bool arrowLeftDown    = isButtonDown(mouseWatcher, "arrow_left");
    bool arrowRightDown   = isButtonDown(mouseWatcher, "arrow_right");

    bool ssaoDown         = isButtonDown(mouseWatcher, "y");
    bool outlineDown      = isButtonDown(mouseWatcher, "u");
    bool bloomDown        = isButtonDown(mouseWatcher, "i");
    bool normalMapsDown   = isButtonDown(mouseWatcher, "o");
    bool fogDown          = isButtonDown(mouseWatcher, "p");
    bool depthOfFieldDown = isButtonDown(mouseWatcher, "h");
    bool posterizeDown    = isButtonDown(mouseWatcher, "j");
    bool pixelizeDown     = isButtonDown(mouseWatcher, "k");
    bool sharpenDown      = isButtonDown(mouseWatcher, "l");
    bool filmGrainDown    = isButtonDown(mouseWatcher, "n");
    bool reflectionDown   = isButtonDown(mouseWatcher, "m");
    bool refractionDown   = isButtonDown(mouseWatcher, ",");
    bool flowMapsDown     = isButtonDown(mouseWatcher, ".");

    bool mouseLeftDown    = mouseWatcher->is_button_down(MouseButton::one());
    bool mouseRightDown   = mouseWatcher->is_button_down(MouseButton::three());

    if (wDown) {
      cameraRotatePhi -= movement * 0.5;
    }

    if (sDown) {
      cameraRotatePhi += movement * 0.5;
    }

    if (aDown) {
      cameraRotateTheta += movement * 0.5;
    }

    if (dDown) {
      cameraRotateTheta -= movement * 0.5;
    }

    if (zDown || mouseWheelUp) {
      cameraRotateRadius -= movement * 4 + 50 * mouseWheelUp;
      mouseWheelUp = false;
    }

    if (xDown || mouseWheelDown) {
      cameraRotateRadius += movement * 4 + 50 * mouseWheelDown;
      mouseWheelDown = false;
    }

    if (cameraRotatePhi    <  1)               cameraRotatePhi    = 1;
    if (cameraRotatePhi    >  179)             cameraRotatePhi    = 179;
    if (cameraRotatePhi    <  0)               cameraRotatePhi    = 360 - cameraRotateTheta;
    if (cameraRotateTheta  >  360)             cameraRotateTheta  = cameraRotateTheta - 360;
    if (cameraRotateTheta  <  0)               cameraRotateTheta  = 360 - cameraRotateTheta;
    if (cameraRotateRadius <= cameraNear + 10) cameraRotateRadius = cameraNear + 10;
    if (cameraRotateRadius >= cameraFar  - 10) cameraRotateRadius = cameraFar  - 10;

    if (arrowUpDown) {
      cameraUpDownAdjust = -1;
    } else if (arrowDownDown) {
      cameraUpDownAdjust =  1;
    }

    if (arrowLeftDown) {
      cameraLeftRightAdjust = 1;
    } else if (arrowRightDown) {
      cameraLeftRightAdjust = -1;
    }

    if (mouseWatcher->has_mouse()) {
      mouseNow[0] = mouseWatcher->get_mouse_x();
      mouseNow[1] = mouseWatcher->get_mouse_y();

      if (mouseLeftDown) {
        cameraRotateTheta += (mouseThen[0] - mouseNow[0] ) * movement;
        cameraRotatePhi   += (mouseNow[1]  - mouseThen[1]) * movement;
      } else if (mouseRightDown) {
        cameraLeftRightAdjust = (mouseThen[0] - mouseNow[0]) * movement;
        cameraUpDownAdjust    = (mouseThen[1] - mouseNow[1]) * movement;
      }

      if (!mouseLeftDown) {
        mouseThen = mouseNow;
      }
    }

    if (shiftDown && fogNearDown) {
      fogNear -= fogAdjust;
      if (fogNear <= 0 ) fogNear = 0;
    } else if (fogNearDown) {
      fogNear += fogAdjust;
    }

    if (shiftDown && fogFarDown) {
      fogFar -= fogAdjust;
      if (fogFar <= 0 ) fogFar = 0;
    } else if (fogFarDown) {
      fogFar += fogAdjust;
    }

    if (shiftDown && equalDown) {
      rior[0] -= riorAdjust;
    } else if (equalDown) {
      rior[0] += riorAdjust;
    }
    rior[1] = rior[0];

    if (shiftDown && minusDown) {
      foamDepth[0] -= foamDepthAdjust;
      if (foamDepth[0] < 0.001) { foamDepth[0] = 0.001; };
    } else if (minusDown) {
      foamDepth[0] += foamDepthAdjust;
    }
    foamDepth[1] = foamDepth[0];

    if (keyDebounced) {
      if (tabDown) {
        if (shiftDown) {
          showBufferIndex -= 1;
          if (showBufferIndex < 0) showBufferIndex = bufferArray.size() - 1;
        } else {
          showBufferIndex += 1;
          if (showBufferIndex >= bufferArray.size()) showBufferIndex = 0;
        }

        PT(GraphicsOutput) buffer = bufferArray[showBufferIndex];
        std::string bufferName = buffer->get_name();
        bool showAlpha =
              bufferName == "outlineBuffer"
          ||  bufferName == "foamBuffer"
          ;

        showBuffer
          ( render2d
          , buffer
          , showAlpha
          );
        keyTime = now;
      }

      if (resetDown) {
        cameraRotateRadius = cameraRotateRadiusInitial;
        cameraRotatePhi    = cameraRotatePhiInitial;
        cameraRotateTheta  = cameraRotateThetaInitial;
        cameraLookAt       = cameraLookAtInitial;

        fogNear = fogNearInitial;
        fogFar  = fogFarInitial;

        foamDepth = foamDepthInitial;
        rior      = riorInitial;

        keyTime = now;
      }

      if (ssaoDown) {
        ssaoEnabled = toggleEnabledVec(ssaoEnabled);
        keyTime = now;
      }

      if (refractionDown) {
        refractionEnabled = toggleEnabledVec(refractionEnabled);
        keyTime = now;
      }

      if (reflectionDown) {
        reflectionEnabled = toggleEnabledVec(reflectionEnabled);
        keyTime = now;
      }

      if (bloomDown) {
        bloomEnabled = toggleEnabledVec(bloomEnabled);
        keyTime = now;
      }

      if (normalMapsDown){
        normalMapsEnabled = toggleEnabledVec(normalMapsEnabled);
        keyTime = now;
      }

      if (fogDown) {
        fogEnabled = toggleEnabledVec(fogEnabled);
        keyTime = now;
      }

      if (outlineDown) {
        outlineEnabled = toggleEnabledVec(outlineEnabled);
        keyTime = now;
      }

      if (sharpenDown) {
        sharpenEnabled = toggleEnabledVec(sharpenEnabled);
        keyTime = now;
      }

      if (depthOfFieldDown) {
        depthOfFieldEnabled = toggleEnabledVec(depthOfFieldEnabled);
        keyTime = now;
      }

      if (posterizeDown) {
        posterizeEnabled = toggleEnabledVec(posterizeEnabled);
        keyTime = now;
      }

      if (pixelizeDown) {
        pixelizeEnabled = toggleEnabledVec(pixelizeEnabled);
        keyTime = now;
      }

      if (filmGrainDown) {
        filmGrainEnabled = toggleEnabledVec(filmGrainEnabled);
        keyTime = now;
      }

      if (flowMapsDown) {
        flowMapsEnabled = toggleEnabledVec(flowMapsEnabled);
        if (flowMapsEnabled[0] == 1 && soundEnabled) {
          for_each(sounds.begin(), sounds.end(), setSoundOn);
        } else if (flowMapsEnabled[0] != 1) {
          for_each(sounds.begin(), sounds.end(), setSoundOff);
        }
        keyTime = now;
      }

      if (deleteDown) {
        if (soundEnabled) {
          for_each(sounds.begin(), sounds.end(), setSoundOff);
          soundEnabled = false;
        } else {
          if (flowMapsEnabled[0] == 1) {
            for_each(sounds.begin(), sounds.end(), setSoundOn);
          }
          soundEnabled = true;
        }
        keyTime = now;
      }
    }

    if (flowMapsEnabled[0]) {
      LVecBase3 hpr = wheelNP.get_hpr();
        hpr[1] += -90.0 * delta;
      if (hpr[1] >= 360) hpr[0] = 0;
      if (hpr[1] <= 0) hpr[0]   = 360;
      wheelNP.set_hpr(hpr);
    }

    cameraLookAt =
      calculateCameraLookAt
        ( cameraUpDownAdjust
        , cameraLeftRightAdjust
        , cameraRotatePhi
        , cameraRotateTheta
        , cameraLookAt
        );

    cameraNP.set_pos
      ( calculateCameraPosition
          ( cameraRotateRadius
          , cameraRotatePhi
          , cameraRotateTheta
          , cameraLookAt
          )
      );

    cameraNP.look_at(cameraLookAt);

    normalNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    normalNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    normalCamera->set_initial_state(normalNP.get_state());

    normalWithWaterNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    normalWithWaterNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    normalWithWaterCamera->set_initial_state(normalWithWaterNP.get_state());

    foamMaskNP.set_shader_input("flowMapsEnabled",    flowMapsEnabled);
    foamMaskCamera->set_initial_state(foamMaskNP.get_state());

    fogNP.set_shader_input("enabled", fogEnabled);
    fogNP.set_shader_input("nearFar", LVecBase2(fogNear, fogFar));
    fogCamera->set_initial_state(fogNP.get_state());

    ssaoNP.set_shader_input("lensProjection", positionCameraLens->get_projection_mat());
    ssaoNP.set_shader_input("enabled",        ssaoEnabled);
    ssaoCamera->set_initial_state(ssaoNP.get_state());

    refractionUvNP.set_shader_input("lensProjection", positionCameraLens->get_projection_mat());
    refractionUvNP.set_shader_input("enabled",        refractionEnabled);
    refractionUvNP.set_shader_input("rior",           rior);
    refractionUvCamera->set_initial_state(refractionUvNP.get_state());

    reflectionUvNP.set_shader_input("lensProjection", positionCameraLens->get_projection_mat());
    reflectionUvNP.set_shader_input("enabled",        reflectionEnabled);
    reflectionUvCamera->set_initial_state(reflectionUvNP.get_state());

    foamNP.set_shader_input("foamDepth",      foamDepth);
    foamCamera->set_initial_state(foamNP.get_state());

    bloomNP.set_shader_input("enabled", bloomEnabled);
    bloomCamera->set_initial_state(bloomNP.get_state());

    outlineNP.set_shader_input("enabled", outlineEnabled);
    outlineCamera->set_initial_state(outlineNP.get_state());

    baseNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    baseNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    baseCamera->set_initial_state(baseNP.get_state());

    specularNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
    specularNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
    specularCamera->set_initial_state(specularNP.get_state());

    sharpenNP.set_shader_input("enabled", sharpenEnabled);
    sharpenCamera->set_initial_state(sharpenNP.get_state());

    depthOfFieldNP
      .set_shader_input
        ( "focalLength"
        , LVecBase2f(environmentNP.get_distance(cameraNP), 0)
        );
    depthOfFieldNP.set_shader_input("enabled", depthOfFieldEnabled);
    depthOfFieldCamera->set_initial_state(depthOfFieldNP.get_state());

    posterizePreBlurNP
      .set_shader_input
        ( "parameters"
        , LVecBase2(posterizeEnabled[0] == 1 ? 5 : 0, 0)
        );
    posterizePreBlurCamera->set_initial_state(posterizePreBlurNP.get_state());

    posterizeNP.set_shader_input("enabled", posterizeEnabled);
    posterizeCamera->set_initial_state(posterizeNP.get_state());

    pixelizeNP.set_shader_input("enabled", pixelizeEnabled);
    pixelizeCamera->set_initial_state(pixelizeNP.get_state());

    filmGrainNP.set_shader_input("enabled", filmGrainEnabled);
    filmGrainCamera->set_initial_state(filmGrainNP.get_state());

    updateAudoManager
      ( sceneRootNP
      , cameraNP
      );
    };

  auto beforeFrameRunner =
    [](GenericAsyncTask* task, void* arg)
      -> AsyncTask::DoneStatus {
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

  auto setMouseWheelUp =
    [&]() {
    mouseWheelUp = true;
    };

  auto setMouseWheelDown =
    [&]() {
    mouseWheelDown = true;
    };

  framework.define_key
    ( "wheel_up"
    , "Mouse Wheel Up"
    , [](const Event*, void* arg) {
      (*static_cast<decltype(setMouseWheelUp)*>(arg))();
      }
    , &setMouseWheelUp
    );

  framework.define_key
    ( "wheel_down"
    , "Mouse Wheel Down"
    , [](const Event*, void* arg) {
      (*static_cast<decltype(setMouseWheelDown)*>(arg))();
      }
    , &setMouseWheelDown
    );

  LVector3f wheelNPRelPos = wheelNP.get_pos(sceneRootNP);
  sounds[0]->set_3d_attributes
    ( wheelNPRelPos[0]
    , wheelNPRelPos[1]
    , wheelNPRelPos[2]
    , 0
    , 0
    , 0
    );
  LVector3f waterNPRelPos = waterNP.get_pos(sceneRootNP);
  sounds[1]->set_3d_attributes
    ( waterNPRelPos[0]
    , waterNPRelPos[1]
    , waterNPRelPos[2]
    , 0
    , 0
    , 0
    );

  sounds[0]->set_3d_min_distance(100);
  sounds[1]->set_3d_min_distance(150);
  sounds[0]->set_loop(true);
  sounds[1]->set_loop(true);
  sounds[0]->play();
  sounds[1]->play();

  framework.main_loop();
  framework.close_framework();

  return 0;
  }

// END MAIN

float toColorRange
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
      ( toColorRange(99, 1)
      , toColorRange(91, 1)
      , toColorRange(114, 1)
      , 1
      )
  );
  NodePath ambientLightNP = render.attach_new_node(ambientLight);
  render.set_light(ambientLightNP);

  PT(DirectionalLight) sunLight = new DirectionalLight("sunLight");
  sunLight->set_color(
    LVecBase4f
      ( toColorRange(214, 0.7)
      , toColorRange(149, 0.5)
      , toColorRange(113, 0.2)
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

  generateWindowLight
    ( "windowLight"
    , render
    , LVecBase3
        ( 1.5
        , 2.21
        , 7.9
        )
    , showLights
    );
  generateWindowLight
    ( "windowLight1"
    , render
    , LVecBase3
        ( 3.5
        , 2.21
        , 7.9
        )
    , showLights
    );
  generateWindowLight
    ( "windowLight2"
    , render
    , LVecBase3
        ( 3.5
        , 1.1
        , 4.5
        )
    , showLights
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
      ( toColorRange(255, 0.33)
      , toColorRange(215, 0.33)
      , toColorRange(188, 0.33)
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
    LVecBase3f sample =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator)
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
    LVecBase3f noise =
      LVecBase3f
        ( randomFloats(generator) * 2.0 - 1.0
        , randomFloats(generator) * 2.0 - 1.0
        , 0.0
        );

    ssaoNoise.push_back(noise);
  }

  return ssaoNoise;
  }

FramebufferTexture generateFramebufferTexture
  ( PT(WindowFramework) window
  , PT(GraphicsOutput) graphicsOutput
  , PT(GraphicsEngine) graphicsEngine
  , LVecBase4 rgbaBits
  , bool setFloatColor
  , std::string name
  , LColor clearColor
  , bool useScene
  ) {
  FrameBufferProperties fbp = FrameBufferProperties::get_default();
  fbp.set_back_buffers(0);
  fbp.set_rgba_bits(rgbaBits[0], rgbaBits[1], rgbaBits[2], rgbaBits[3]);
  fbp.set_rgb_color(true);
  fbp.set_float_color(setFloatColor);

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
  buffer->set_clear_color(clearColor);
  buffer->set_sort(-30000);

  NodePath   cameraNP = NodePath("");
  PT(Camera) camera   = NULL;

  if (useScene) {
    cameraNP = window->make_camera();
    camera   = DCAST(Camera, cameraNP.node());
    camera->set_lens(window->get_camera(0)->get_lens());
  } else {
    camera = new Camera(name + "Camera");
    PT(OrthographicLens) lens = new OrthographicLens();
    lens->set_film_size(2, 2);
    lens->set_film_offset(0, 0);
    lens->set_near_far(-1, 1);
    camera->set_lens(lens);
    cameraNP = NodePath(camera);
  }

  PT(DisplayRegion) bufferRegion =
    buffer->make_display_region(0, 1, 0, 1);
  bufferRegion->set_camera(cameraNP);

  NodePath shaderNP = NodePath(name + "Shader");

  if (!useScene) {
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
  }

  FramebufferTexture result;
  result.buffer   = buffer;
  result.camera   = camera;
  result.cameraNP = cameraNP;
  result.shaderNP = shaderNP;
  return result;
  }

void showBuffer
  ( NodePath render2d
  , PT(GraphicsOutput) buffer
  , bool alpha
  ) {
  hideBuffer
    ( render2d
    );
  NodePath nodePath = buffer->get_texture_card();
  nodePath.reparent_to(render2d);
  if (alpha)
    nodePath.set_transparency
      ( TransparencyAttrib::Mode::M_alpha
      );
  }

void hideBuffer
  ( NodePath render2d
  ) {
  NodePath nodePath = render2d.find("**/texture card");
  if (nodePath)
    nodePath.detach_node();
  }

int microsecondsSinceEpoch
  (
  ) {
  return std::chrono::duration_cast
    <std::chrono::microseconds>
      ( std::chrono::system_clock::now().time_since_epoch()
      ).count();
  }

bool isButtonDown
  ( PT(MouseWatcher) mouseWatcher
  , std::string character
  ) {
  return
    mouseWatcher
      ->is_button_down
        ( ButtonRegistry::ptr()->find_button(character)
        );
  }

PT(MouseWatcher) getMouseWatcher
  ( WindowFramework* window
  ) {
  return DCAST
    ( MouseWatcher
    , window->get_mouse().node()
    );
  }

void setSoundOff
  ( PT(AudioSound) sound
  ) {
    setSoundState(sound, false);
  }

void setSoundOn
  ( PT(AudioSound) sound
  ) {
    setSoundState(sound, true);
  }

void setSoundState
  ( PT(AudioSound) sound
  , bool on
  ) {
    if (!on && sound->status() == AudioSound::PLAYING) {
      sound->stop();
    } else if (on && sound->status() != AudioSound::PLAYING) {
      sound->play();
    }
  }

void updateAudoManager
  ( NodePath sceneRootNP
  , NodePath cameraNP
  ) {
  LVector3f f = sceneRootNP.get_relative_vector(cameraNP, LVector3f::forward());
  LVector3f u = sceneRootNP.get_relative_vector(cameraNP, LVector3f::up());
  LVector3f v = LVector3f(0, 0, 0);
  LVector3f p = cameraNP.get_pos(sceneRootNP);

  audioManager->audio_3d_set_listener_attributes
    ( p[0], p[1], p[2]
    , v[0], v[1], v[2]
    , f[0], f[1], f[2]
    , u[0], u[1], u[2]
    );

  audioManager->update();
  }

LVecBase3f calculateCameraPosition
  ( double radius
  , double phi
  , double theta
  , LVecBase3 lookAt
  ) {
  double x = radius * sin(toRadians(phi)) * cos(toRadians(theta)) + lookAt[0];
  double y = radius * sin(toRadians(phi)) * sin(toRadians(theta)) + lookAt[1];
  double z = radius * cos(toRadians(phi))                         + lookAt[2];
  return LVecBase3f(x, y, z);
  }

LVecBase3f calculateCameraLookAt
  ( double upDownAdjust
  , double leftRightAdjust
  , double phi
  , double theta
  , LVecBase3 lookAt
  ) {
  lookAt[0] +=  upDownAdjust * sin(toRadians(-theta - 90)) * cos(toRadians(phi));
  lookAt[1] +=  upDownAdjust * cos(toRadians(-theta - 90)) * cos(toRadians(phi));
  lookAt[2] -= -upDownAdjust * sin(toRadians(phi));

  lookAt[0] += leftRightAdjust * sin(toRadians(-theta));
  lookAt[1] += leftRightAdjust * cos(toRadians(-theta));
  return lookAt;
  }

double microsecondToSecond
  ( int m
  ) {
  return m / 1000000.0;
  }

double toRadians
  ( double d
  ) {
  return d * M_PI / 180;
  }

LVecBase2 makeEnabledVec
  ( int t
  ){
  if (t >= 1) { t = 1; } else { t = 0; }
  return LVecBase2(t, t);
  }

LVecBase2 toggleEnabledVec
  ( LVecBase2 vec
  ){
  int t = vec[0];
  if (t >= 1) { t = 0; } else { t = 1; }
  vec[0] = t;
  vec[1] = t;
  return vec;
  }
