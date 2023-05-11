from numpy import pi as M_PI
from numpy import cos, sin, radians
from panda3d.core import PTA_LVecBase3f, LVecBase3f, LVecBase2f, LVecBase4f, LColor, LVecBase4, load_prc_file, SamplerState, LVecBase3, TexturePool, PandaNode, NodePath, LVector3f, LPoint3f, TransparencyAttrib, LMatrix4
from panda3d.physics import ParticleSystem, ForceNode, PhysicalNode, PointParticleFactory, SpriteParticleRenderer, BaseParticleRenderer, LinearVectorForce, ParticleSystemManager, PhysicsManager, PointEmitter, BaseParticleEmitter, LinearJitterForce, LinearCylinderVortexForce
from panda3d.core import AmbientLight, DirectionalLight
from panda3d.core import Shader
from panda3d.core import GraphicsOutput, FrameBufferProperties, WindowProperties, GraphicsPipe, OrthographicLens, Camera, CardMaker, BitMask32, Texture
from numpy.random import rand

class FramebufferTextureArguments:
    def __init__(self):
        self.rgbaBits = LVecBase4(0)
        self.clearColor = LColor(0)
        self.aux_rgba = 0
        self.setFloatColor = False
        self.setSrgbColor = False
        self.setRgbColor = False
        self.useScene = False
        self.name = 'no name'
        self.bitplane = None
        
class FramebufferTexture:
    def __init__(self):
        self.buffer = None
        self.bufferRegion = None
        self.camera = None
        self.cameraNP = None
        self.shaderNP = None
        self.tex = None
    def generate(self, fbt_args):
        rgbaBits       = fbt_args.rgbaBits;
        bitplane       = fbt_args.bitplane;
        aux_rgba       = fbt_args.aux_rgba;
        setFloatColor  = fbt_args.setFloatColor;
        setSrgbColor   = fbt_args.setSrgbColor;
        setRgbColor    = fbt_args.setRgbColor;
        useScene       = fbt_args.useScene;
        name           = fbt_args.name;
        clearColor     = fbt_args.clearColor;
        fbp = FrameBufferProperties();
        fbp.set_back_buffers(0);
        fbp.set_rgba_bits( int(rgbaBits[0]), int(rgbaBits[1]), int(rgbaBits[2]), int(rgbaBits[3]));
        fbp.set_aux_rgba(aux_rgba);
        fbp.set_float_color(setFloatColor);
        fbp.set_srgb_color (setSrgbColor );
        fbp.set_rgb_color  (setRgbColor  );
        
def calculateCameraPosition(radius, phi, theta, lookAt):
  x = radius * sin(radians(phi)) * cos(radians(theta)) + lookAt[0];
  y = radius * sin(radians(phi)) * sin(radians(theta)) + lookAt[1];
  z = radius * cos(radians(phi))                         + lookAt[2];
  return LVecBase3f(x, y, z);
        
def generateFramebufferTexture(framebufferTextureArguments):
    rgbaBits       = framebufferTextureArguments.rgbaBits;
    bitplane       = framebufferTextureArguments.bitplane;
    aux_rgba       = framebufferTextureArguments.aux_rgba;
    setFloatColor  = framebufferTextureArguments.setFloatColor;
    setSrgbColor   = framebufferTextureArguments.setSrgbColor;
    setRgbColor    = framebufferTextureArguments.setRgbColor;
    useScene       = framebufferTextureArguments.useScene;
    name           = framebufferTextureArguments.name;
    clearColor     = framebufferTextureArguments.clearColor;

    fbp = FrameBufferProperties();
    fbp.set_back_buffers(0);
    fbp.set_rgba_bits( int(rgbaBits[0]), int(rgbaBits[1]), int(rgbaBits[2]), int(rgbaBits[3]));
    fbp.set_aux_rgba(aux_rgba);
    fbp.set_float_color(setFloatColor);
    fbp.set_srgb_color (setSrgbColor );
    fbp.set_rgb_color  (setRgbColor  );
    
    hold_tex = Texture()

    buffer = base.graphicsEngine.make_output(base.win.get_pipe(), name + "Buffer" , BACKGROUND_RENDER_SORT_ORDER - 1, fbp, WindowProperties.size(0, 0), GraphicsPipe.BF_refuse_window|GraphicsPipe.BF_resizeable|GraphicsPipe.BF_can_bind_every|GraphicsPipe.BF_rtt_cumulative| GraphicsPipe.BF_size_track_host, base.win.gsg, base.win)
    buffer.add_render_texture(hold_tex , GraphicsOutput.RTM_bind_or_copy, bitplane);
    buffer.set_clear_color(clearColor);

    cameraNP = NodePath("");
    camera   = None;

    if (useScene):
        cameraNP = base.makeCamera(base.win, lens = base.camLens);
        camera   = cameraNP.node()
#        camera.set_lens();
    else:
        camera = Camera(name + "Camera");
        lens = OrthographicLens();
        lens.set_film_size(2, 2);
        lens.set_film_offset(0, 0);
        lens.set_near_far(-1, 1);
        camera.set_lens(lens);
        cameraNP = NodePath(camera);
    
    bufferRegion =buffer.make_display_region(0, 1, 0, 1);
    bufferRegion.set_camera(cameraNP);
    
    shaderNP = NodePath(name + "Shader");
    
    if (not useScene):
        renderNP = NodePath(name + "Render");
        renderNP.set_depth_test(False);
        renderNP.set_depth_write(False);
        cameraNP.reparent_to(renderNP);
        card = CardMaker(name);
        card.set_frame_fullscreen_quad();
        card.set_has_uvs(True);
        cardNP = NodePath(card.generate());
        cardNP.reparent_to(renderNP);
        cardNP.set_pos(0, 0, 0);
        cardNP.set_hpr(0, 0, 0);
        cameraNP.look_at(cardNP);
    
    result = FramebufferTexture();
    result.buffer       = buffer;
    result.bufferRegion = bufferRegion;
    result.camera       = camera;
    result.cameraNP     = cameraNP;
    result.shaderNP     = shaderNP;
    result.tex = hold_tex
    return result
    
def hideBuffer(render2d):
    nodePath = render2d.find("**/texture card");
    if (nodePath):
        nodePath.detach_node();
    
def showBuffer(render2d, statusNP,bufferTexture, alpha):
    hideBuffer( render2d);
    bufferName, buffer, texture = bufferTexture;
    nodePath = buffer.get_texture_card();
    nodePath.set_texture(buffer.get_texture(texture));
    nodePath.reparent_to(render2d);
    nodePath.set_y(0);
    if (alpha):
        nodePath.set_transparency(TransparencyAttrib.M_alpha);
    if statusNP is not None:
        statusNP.reparent_to(nodePath);
        
def loadShader(vs, fs):
    shader = Shader.load(Shader.SL_GLSL,
                     vertex= 'shaders/vertex/' + vs + '.vert',
                     fragment='shaders/fragment/' + fs + ".frag")
    return shader
    
def generateSsaoSamples( numberOfSamples, generator = 1):
    def lerp(a, b, f):
        return a + f * (b - a)

    ssaoSamples = PTA_LVecBase3f();

    for i in range(numberOfSamples):
        sample = LVecBase3f( rand() * 2.0 - 1.0, rand() * 2.0 - 1.0, rand()).normalized();
    
        randx = rand();
        sample[0] *= randx;
        sample[1] *= randx;
        sample[2] *= randx;
        
        scale =  i / numberOfSamples
        scale = lerp(0.1, 1.0, scale * scale);
        sample[0] *= scale;
        sample[1] *= scale;
        sample[2] *= scale;
        
        ssaoSamples.push_back(sample)

    return ssaoSamples

def generateSsaoNoise( numberOfNoise):
    ssaoNoise = PTA_LVecBase3f();

    for i in range(numberOfNoise):
        noise =LVecBase3f( rand() * 2.0 - 1.0, rand() * 2.0 - 1.0, 0.0);
    
        ssaoNoise.push_back(noise);
    
    return ssaoNoise
    
def setUpParticles( render, smokeTexture, particleSystemManager, physicsManager):
    smokePS = ParticleSystem();
    smokeFN = ForceNode("smoke");
    smokePN = PhysicalNode("smoke");
    
    smokePS.set_pool_size(75);
    smokePS.set_birth_rate(0.01);
    smokePS.set_litter_size(1);
    smokePS.set_litter_spread(2);
    smokePS.set_system_lifespan(0.0);
    smokePS.set_local_velocity_flag(True);
    smokePS.set_system_grows_older_flag(False);
    
    smokePPF = PointParticleFactory();
    smokePPF.set_lifespan_base(0.1);
    smokePPF.set_lifespan_spread(3);
    smokePPF.set_mass_base(1);
    smokePPF.set_mass_spread(0);
    smokePPF.set_terminal_velocity_base(400);
    smokePPF.set_terminal_velocity_spread(0);
    smokePS.set_factory(smokePPF);
    
    smokeSPR = SpriteParticleRenderer();
    smokeSPR.set_alpha_mode(BaseParticleRenderer.PR_ALPHA_OUT);
    smokeSPR.set_user_alpha(1.0);
    smokeSPR.set_texture(smokeTexture);
    smokeSPR.set_color(LColor(1.0, 1.0, 1.0, 1.0));
    smokeSPR.set_x_scale_flag(True);
    smokeSPR.set_y_scale_flag(True);
    smokeSPR.set_anim_angle_flag(True);
    smokeSPR.set_initial_x_scale(0.0000001);
    smokeSPR.set_final_x_scale(  0.007);
    smokeSPR.set_initial_y_scale(0.0000001);
    smokeSPR.set_final_y_scale(  0.007);
    smokeSPR.set_nonanimated_theta(209.0546);
    smokeSPR.set_alpha_blend_method(BaseParticleRenderer.PP_BLEND_CUBIC);
    smokeSPR.set_alpha_disable(False);
    smokeSPR.get_color_interpolation_manager().add_linear( 0.0, 1.0, LColor(1.0,   1.0,   1.0,   1.0), LColor(0.039, 0.078, 0.156, 1.0), True);
    smokePS.set_renderer(smokeSPR);
    
    smokePE = PointEmitter();
    smokePE.set_emission_type(BaseParticleEmitter.ET_EXPLICIT);
    smokePE.set_amplitude(0.0);
    smokePE.set_amplitude_spread(1.0);
    smokePE.set_offset_force(LVector3f(0.0, 0.0, 2.0));
    smokePE.set_explicit_launch_vector(LVector3f(0.0, 0.1, 0.0));
    smokePE.set_radiate_origin(LPoint3f(0.0, 0.0, 0.0));
    smokePE.set_location(LPoint3f(0.0, 0.0, 0.0));
    smokePS.set_emitter(smokePE);
    
    smokeLVF = LinearVectorForce(LVector3f(3.0, -2.0, 0.0), 1.0, False);
    smokeLVF.set_vector_masks(True, True, True);
    smokeLVF.set_active(True);
    smokeFN.add_force(smokeLVF);
    smokePS.add_linear_force(smokeLVF);
    
    smokeLJF = LinearJitterForce(2.0, False);
    smokeLJF.set_vector_masks(True, True, True);
    smokeLJF.set_active(True);
    smokeFN.add_force(smokeLJF);
    smokePS.add_linear_force(smokeLJF);
    
    smokeLCVF = LinearCylinderVortexForce(10.0, 1.0, 4.0, 1.0, False);
    smokeLCVF.set_vector_masks(True, True, True);
    smokeLCVF.set_active(True);
    smokeFN.add_force(smokeLCVF);
    smokePS.add_linear_force(smokeLCVF);
    
    smokePN.insert_physical(0, smokePS);
    smokePS.set_render_parent(smokePN);
    smokeNP = render.attach_new_node(smokePN);
    smokeNP.attach_new_node(smokeFN);
    
    
    particleSystemManager.attach_particlesystem(smokePS);
    physicsManager.attach_physical(smokePS);
    
    smokeNP.set_pos(0.47, 4.5, 8.9);
    smokeNP.set_transparency(TransparencyAttrib.M_dual);
    smokeNP.set_bin("fixed", 0);
    return smokeNP
    
def generateLights( render, showLights):
    ambientLight = AmbientLight("ambientLight");
    ambientLight.set_color( LVecBase4( 0.388, 0.356, 0.447 , 1));
    ambientLightNP = render.attach_new_node(ambientLight);
    render.set_light(ambientLightNP);

    sunlight = DirectionalLight("sunlight");
    sunlight.set_color(sunlightColor1);
    sunlight.set_shadow_caster(True, SHADOW_SIZE, SHADOW_SIZE);
    sunlight.get_lens().set_film_size(35, 35);
    sunlight.get_lens().set_near_far(5.0, 35.0);
    if showLights:
        sunlight.show_frustum()
    sunlightNP = render.attach_new_node(sunlight);
    sunlightNP.set_name("sunlight");
    render.set_light(sunlightNP);
    
    moonlight = DirectionalLight("moonlight");
    moonlight.set_color(moonlightColor1);
    moonlight.set_shadow_caster(True, SHADOW_SIZE, SHADOW_SIZE);
    moonlight.get_lens().set_film_size(35, 35);
    moonlight.get_lens().set_near_far(5.0, 35);
    if (showLights):
        moonlight.show_frustum();
    moonlightNP = render.attach_new_node(moonlight);
    moonlightNP.set_name("moonlight");
    render.set_light_off(moonlightNP);
    
    sunlightPivotNP = NodePath("sunlightPivot");
    sunlightPivotNP.reparent_to(render);
    sunlightPivotNP.set_pos(0, 0.5, 15.0);
    sunlightNP.reparent_to(sunlightPivotNP);
    sunlightNP.set_pos(0, -17.5, 0);
    sunlightPivotNP.set_hpr(135, 340, 0);
    
    moonlightPivotNP = NodePath("moonlightPivot");
    moonlightPivotNP.reparent_to(render);
    moonlightPivotNP.set_pos(0, 0.5, 15.0);
    moonlightNP.reparent_to(moonlightPivotNP);
    moonlightNP.set_pos(0, -17.5, 0);
    moonlightPivotNP.set_hpr(135, 160, 0);
    
def phy_task():
    global particleSystemManager
    global physicsManager
    particleSystemManager.do_particles(delta);
    physicsManager.do_physics(delta);

TO_RAD               = M_PI / 180.0;
PI_SHADER_INPUT = LVecBase2f(M_PI, TO_RAD);

GAMMA                   = 2.2;
GAMMA_REC               = 1.0 / GAMMA;
GAMMA_SHADER_INPUT = LVecBase2f(GAMMA, GAMMA_REC);

BACKGROUND_RENDER_SORT_ORDER = 10;
UNSORTED_RENDER_SORT_ORDER   = 50;

SSAO_SAMPLES = 8;
SSAO_NOISE   = 4;

SHADOW_SIZE = 2048;

sunlightColor0 =LVecBase4f( 0.612, 0.365, 0.306, 1);
sunlightColor1 =LVecBase4f( 0.765, 0.573, 0.400, 1);
moonlightColor0 =LVecBase4f( 0.247, 0.384, 0.404, 1);
moonlightColor1 =LVecBase4f( 0.392, 0.537, 0.571, 1);
windowLightColor =LVecBase4f( 0.765, 0.573, 0.400, 1);

backgroundColor =[LColor( 0.392, 0.537, 0.561, 1) , LColor( 0.953, 0.733 , 0.525 , 1)];

previousViewWorldMat = None
currentViewWorldMat = None

def setTextureToNearestAndClamp(texture):
    texture.set_magfilter(SamplerState.FT_nearest);
    texture.set_minfilter(SamplerState.FT_nearest);
    texture.set_wrap_u(SamplerState.WM_clamp);
    texture.set_wrap_v(SamplerState.WM_clamp);
    texture.set_wrap_w(SamplerState.WM_clamp);

cameraRotatePhiInitial    =   67.5095;
cameraRotateThetaInitial  =  231.721;
cameraRotateRadiusInitial = 1100.83;
cameraLookAtInitial       = LVecBase3(1.00839, 1.20764, 5.85055);
cameraFov                 =    1.0;
cameraNear                =  150;
cameraFar                 = 2000;
cameraNearFar             = LVecBase2f(cameraNear, cameraFar);
cameraRotateRadius        = cameraRotateRadiusInitial;
cameraRotatePhi           = cameraRotatePhiInitial;
cameraRotateTheta         = cameraRotateThetaInitial;
cameraLookAt              = cameraLookAtInitial;

fogNearInitial = 2.0;
fogFarInitial  = 90.0;
fogNear        = fogNearInitial;
fogFar         = fogFarInitial;
fogAdjust      = 0.1;

foamDepthInitial = LVecBase2f(1.5, 1.5);
foamDepthAdjust  = 0.1;
foamDepth        = foamDepthInitial;

mouseThen = LVecBase2f(0.0, 0.0);
mouseNow  = mouseThen;
mouseWheelDown  = False;
mouseWheelUp    = False;

riorInitial = LVecBase2f(1.05, 1.05);
riorAdjust  = 0.005;
rior        = riorInitial;

mouseFocusPointInitial = LVecBase2f(0.509167, 0.598);
mouseFocusPoint        = mouseFocusPointInitial;

sunlightP       = 260;
animateSunlight = True;

soundEnabled  = True;
soundStarted  = False;
startSoundAt  = 0.5;

closedShutters  = True;

statusAlpha       = 1.0;
statusColor       = LColor(0.9, 0.9, 1.0, statusAlpha);
statusShadowColor = LColor(0.1, 0.1, 0.3, statusAlpha);
statusFadeRate    = 2.0;
statusText        = "Ready";

ssaoEnabled                = LVecBase2f(1);
blinnPhongEnabled          = LVecBase2f(1);
fresnelEnabled             = LVecBase2f(1);
rimLightEnabled            = LVecBase2f(1);
refractionEnabled          = LVecBase2f(1);
reflectionEnabled          = LVecBase2f(1);
fogEnabled                 = LVecBase2f(1);
outlineEnabled             = LVecBase2f(1);
celShadingEnabled          = LVecBase2f(1);
normalMapsEnabled          = LVecBase2f(1);
bloomEnabled               = LVecBase2f(1);
sharpenEnabled             = LVecBase2f(1);
depthOfFieldEnabled        = LVecBase2f(1);
filmGrainEnabled           = LVecBase2f(1);
flowMapsEnabled            = LVecBase2f(1);
lookupTableEnabled         = LVecBase2f(1);
painterlyEnabled           = LVecBase2f(0);
motionBlurEnabled          = LVecBase2f(0);
posterizeEnabled           = LVecBase2f(0);
pixelizeEnabled            = LVecBase2f(0);
chromaticAberrationEnabled = LVecBase2f(1);

rgba8  = LVecBase4( 8,  8,  8,  8);
rgba16 = LVecBase4(16, 16, 16, 16);
rgba32 = LVecBase4(32, 32, 32, 32);

load_prc_file("panda3d-prc-file.prc");

blankTexture             = TexturePool.loadTexture("images/blank.png");
foamPatternTexture       = TexturePool.loadTexture("images/foam-pattern.png");
stillFlowTexture         = TexturePool.loadTexture("images/still-flow.png");
upFlowTexture            = TexturePool.loadTexture("images/up-flow.png");
colorLookupTableTextureN = TexturePool.loadTexture("images/lookup-table-neutral.png");
colorLookupTableTexture0 = TexturePool.loadTexture("images/lookup-table-0.png");
colorLookupTableTexture1 = TexturePool.loadTexture("images/lookup-table-1.png");
smokeTexture             = TexturePool.loadTexture("images/smoke.png");
colorNoiseTexture        = TexturePool.loadTexture("images/color-noise.png");

setTextureToNearestAndClamp(colorLookupTableTextureN);
setTextureToNearestAndClamp(colorLookupTableTexture0);
setTextureToNearestAndClamp(colorLookupTableTexture1);

from direct.showbase.ShowBase import ShowBase
ShowBase()

base.win.setClearColorActive(True)
base.win.set_clear_depth_active(True);
base.win.set_clear_stencil_active(True);
base.win.set_clear_color(backgroundColor[1]);
base.win.set_clear_depth(1.0);
base.win.set_clear_stencil(0);

#base.camLens.set_fov(cameraFov)
#base.camLens.set_near_far(cameraNear, cameraFar)
#
#cameraNP = base.camera
#
#cameraNP.set_pos
#( calculateCameraPosition
#    ( cameraRotateRadius
#    , cameraRotatePhi
#    , cameraRotateTheta
#    , cameraLookAt
#    )
#);
#cameraNP.look_at(cameraLookAt);

#print(cameraNP.getPos(render), 'cam world ois')

sceneRootPN = PandaNode("sceneRoot");
sceneRootNP      = NodePath(sceneRootPN);
sceneRootNP.reparent_to(render);

environmentNP =base.loader.loadModel("eggs/mill-scene/mill-scene.bam");
environmentNP.reparent_to(sceneRootNP);
shuttersNP = base.loader.loadModel("eggs/mill-scene/shutters.bam");
shuttersNP.reparent_to(sceneRootNP);
weatherVaneNP =base.loader.loadModel("eggs/mill-scene/weather-vane.bam");
weatherVaneNP.reparent_to(sceneRootNP);
bannerNP =base.loader.loadModel("eggs/mill-scene/banner.bam");
bannerNP.reparent_to(sceneRootNP);

wheelNP   = environmentNP.find("**/wheel-lp");
waterNP   = environmentNP.find("**/water-lp");


waterNP.set_transparency(TransparencyAttrib.M_dual);
waterNP.set_bin("fixed", 0);

cameraNP = base.camera

particleSystemManager = ParticleSystemManager(20)
physicsManager = PhysicsManager()

smokeNP = setUpParticles(render, smokeTexture, particleSystemManager, physicsManager)
generateLights(render, False)
discardShader               = loadShader("discard", "discard");
baseShader                  = loadShader("base",    "base");
geometryBufferShader0       = loadShader("base",    "geometry-buffer-0");
geometryBufferShader1       = loadShader("base",    "geometry-buffer-1");
geometryBufferShader2       = loadShader("base",    "geometry-buffer-2");
foamShader                  = loadShader("basic",   "foam");
fogShader                   = loadShader("basic",   "fog");
boxBlurShader               = loadShader("basic",   "box-blur");
motionBlurShader            = loadShader("basic",   "motion-blur");
kuwaharaFilterShader        = loadShader("basic",   "kuwahara-filter");
dilationShader              = loadShader("basic",   "dilation");
sharpenShader               = loadShader("basic",   "sharpen");
outlineShader               = loadShader("basic",   "outline");
bloomShader                 = loadShader("basic",   "bloom");
ssaoShader                  = loadShader("basic",   "ssao");
screenSpaceRefractionShader = loadShader("basic",   "screen-space-refraction");
screenSpaceReflectionShader = loadShader("basic",   "screen-space-reflection");
refractionShader            = loadShader("basic",   "refraction");
reflectionColorShader       = loadShader("basic",   "reflection-color");
reflectionShader            = loadShader("basic",   "reflection");
baseCombineShader           = loadShader("basic",   "base-combine");
sceneCombineShader          = loadShader("basic",   "scene-combine");
depthOfFieldShader          = loadShader("basic",   "depth-of-field");
posterizeShader             = loadShader("basic",   "posterize");
pixelizeShader              = loadShader("basic",   "pixelize");
filmGrainShader             = loadShader("basic",   "film-grain");
lookupTableShader           = loadShader("basic",   "lookup-table");
gammaCorrectionShader       = loadShader("basic",   "gamma-correction");
chromaticAberrationShader   = loadShader("basic",   "chromatic-aberration");

mainCameraNP = NodePath("mainCamera");
mainCameraNP.set_shader(discardShader);
mainCamera = base.camera
#    mainCamera.set_initial_state(mainCameraNP.get_state());

isWaterNP = NodePath("isWater");
isWaterNP.set_shader_input("isWater",            LVecBase2f(1.0, 1.0));
isWaterNP.set_shader_input("flowTexture",        upFlowTexture);
isWaterNP.set_shader_input("foamPatternTexture", foamPatternTexture);

isSmokeNP = NodePath("isSmoke");
isSmokeNP.set_shader_input("isSmoke",    LVecBase2f(1.0, 1.0));
isSmokeNP.set_shader_input("isParticle", LVecBase2f(1.0, 1.0));

currentViewWorldMat      = cameraNP.get_transform(render).get_mat();
previousViewWorldMat     = LMatrix4();

framebufferTextureArguments = FramebufferTextureArguments();

framebufferTextureArguments.bitplane       = GraphicsOutput.RTP_color;
framebufferTextureArguments.rgbaBits       = rgba32;
framebufferTextureArguments.clearColor     = LColor(0, 0, 0, 0);
framebufferTextureArguments.aux_rgba       = 1;
framebufferTextureArguments.setFloatColor  = True;
framebufferTextureArguments.setSrgbColor   = False;
framebufferTextureArguments.setRgbColor    = True;
framebufferTextureArguments.useScene       = True;
framebufferTextureArguments.name           = "geometry0";

geometryFramebufferTexture0 = generateFramebufferTexture( framebufferTextureArguments)
geometryBuffer0 = geometryFramebufferTexture0.buffer;
geometryCamera0 = geometryFramebufferTexture0.camera;
geometryNP0     = geometryFramebufferTexture0.shaderNP;
geometryBuffer0.add_render_texture(Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_0);
geometryBuffer0.set_clear_active(3, True);
geometryBuffer0.set_clear_value( 3, framebufferTextureArguments.clearColor);
geometryNP0.set_shader(geometryBufferShader0);
geometryNP0.set_shader_input("normalMapsEnabled", normalMapsEnabled);
geometryCamera0.set_initial_state(geometryNP0.get_state());
geometryCamera0.set_camera_mask(BitMask32.bit(1));
positionTexture0    = geometryBuffer0.get_texture(0);
normalTexture0      = geometryBuffer0.get_texture(1);
geometryCameraLens0 = geometryCamera0.get_lens();
waterNP.hide(BitMask32.bit(1));
smokeNP.hide(BitMask32.bit(1));

framebufferTextureArguments.aux_rgba = 4;
framebufferTextureArguments.name     = "geometry1";

geometryFramebufferTexture1 =generateFramebufferTexture( framebufferTextureArguments);
geometryBuffer1 = geometryFramebufferTexture1.buffer;
geometryCamera1 = geometryFramebufferTexture1.camera;
geometryNP1     = geometryFramebufferTexture1.shaderNP;
geometryBuffer1.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_0);
geometryBuffer1.set_clear_active(3, True);
geometryBuffer1.set_clear_value( 3, framebufferTextureArguments.clearColor);
geometryBuffer1.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_1);
geometryBuffer1.set_clear_active(4, True);
geometryBuffer1.set_clear_value( 4, framebufferTextureArguments.clearColor);
geometryBuffer1.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_2);
geometryBuffer1.set_clear_active(5, True);
geometryBuffer1.set_clear_value( 5, framebufferTextureArguments.clearColor);
geometryBuffer1.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_3);
geometryBuffer1.set_clear_active(6, True);
geometryBuffer1.set_clear_value( 6, framebufferTextureArguments.clearColor);
geometryNP1.set_shader(geometryBufferShader1);
geometryNP1.set_shader_input("normalMapsEnabled",  normalMapsEnabled);
geometryNP1.set_shader_input("flowTexture",        stillFlowTexture);
geometryNP1.set_shader_input("foamPatternTexture", blankTexture);
geometryNP1.set_shader_input("flowMapsEnabled",    flowMapsEnabled);
geometryCamera1.set_initial_state(geometryNP1.get_state());
geometryCamera1.set_tag_state_key("geometryBuffer1");
geometryCamera1.set_tag_state("isWater", isWaterNP.get_state());
geometryCamera1.set_camera_mask(BitMask32.bit(2));
positionTexture1        = geometryBuffer1.get_texture(0);
normalTexture1          = geometryBuffer1.get_texture(1);
reflectionMaskTexture   = geometryBuffer1.get_texture(2);
refractionMaskTexture   = geometryBuffer1.get_texture(3);
foamMaskTexture         = geometryBuffer1.get_texture(4);
geometryCameraLens1     = geometryCamera1.get_lens();
waterNP.set_tag("geometryBuffer1", "isWater");
smokeNP.hide(BitMask32.bit(2));

framebufferTextureArguments.aux_rgba = 1;
framebufferTextureArguments.name     = "geometry2";

geometryFramebufferTexture2 =generateFramebufferTexture( framebufferTextureArguments);
geometryBuffer2 = geometryFramebufferTexture2.buffer;
geometryCamera2 = geometryFramebufferTexture2.camera;
geometryNP2     = geometryFramebufferTexture2.shaderNP;
geometryBuffer2.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_0);
geometryBuffer2.set_clear_active(3, True);
geometryBuffer2.set_clear_value( 3, framebufferTextureArguments.clearColor);
geometryBuffer2.set_sort(geometryBuffer1.get_sort() + 1);
geometryNP2.set_shader(geometryBufferShader2);
geometryNP2.set_shader_input("isSmoke",         LVecBase2f(0, 0));
geometryNP2.set_shader_input("positionTexture", positionTexture1);
geometryCamera2.set_initial_state(geometryNP2.get_state());
geometryCamera2.set_tag_state_key("geometryBuffer2");
geometryCamera2.set_tag_state("isSmoke", isSmokeNP.get_state());
smokeNP.set_tag("geometryBuffer2", "isSmoke");
positionTexture2         = geometryBuffer2.get_texture(0);
smokeMaskTexture         = geometryBuffer2.get_texture(1);
geometryCameraLens2      = geometryCamera2.get_lens();

framebufferTextureArguments.rgbaBits      = rgba8;
framebufferTextureArguments.aux_rgba      = 0;
framebufferTextureArguments.clearColor    = LColor(1, 0, 0, 0);
framebufferTextureArguments.setFloatColor = False;
framebufferTextureArguments.useScene      = False;
framebufferTextureArguments.name          = "fog";

fogFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
fogBuffer = fogFramebufferTexture.buffer;
fogCamera = fogFramebufferTexture.camera;
fogNP     = fogFramebufferTexture.shaderNP;
fogBuffer.set_sort(geometryBuffer2.get_sort() + 1);
fogNP.set_shader(fogShader);
fogNP.set_shader_input("pi",               PI_SHADER_INPUT);
fogNP.set_shader_input("gamma",            GAMMA_SHADER_INPUT);
fogNP.set_shader_input("backgroundColor0", backgroundColor[0]);
fogNP.set_shader_input("backgroundColor1", backgroundColor[1]);
fogNP.set_shader_input("positionTexture0", positionTexture1);
fogNP.set_shader_input("positionTexture1", positionTexture2);
fogNP.set_shader_input("smokeMaskTexture", smokeMaskTexture);
fogNP.set_shader_input("sunPosition",      LVecBase2f(sunlightP, 0));
fogNP.set_shader_input("origin",           cameraNP.get_relative_point(render, environmentNP.get_pos()));
fogNP.set_shader_input("nearFar",          LVecBase2f(fogNear, fogFar));
fogNP.set_shader_input("enabled",          fogEnabled);
fogCamera.set_initial_state(fogNP.get_state());
fogTexture = fogBuffer.get_texture();

framebufferTextureArguments.clearColor = LColor(1, 1, 1, 0);
framebufferTextureArguments.name       = "ssao";

ssaoFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
ssaoBuffer = ssaoFramebufferTexture.buffer;
ssaoCamera = ssaoFramebufferTexture.camera;
ssaoNP     = ssaoFramebufferTexture.shaderNP;
ssaoBuffer.set_sort(geometryBuffer0.get_sort() + 1);
ssaoNP.set_shader(ssaoShader);
ssaoNP.set_shader_input("positionTexture", positionTexture0);
ssaoNP.set_shader_input("normalTexture",   normalTexture0);
ssaoNP.set_shader_input("samples",         generateSsaoSamples(SSAO_SAMPLES));
ssaoNP.set_shader_input("noise",           generateSsaoNoise(SSAO_NOISE));
ssaoNP.set_shader_input("lensProjection",  geometryCameraLens0.get_projection_mat());
ssaoNP.set_shader_input("enabled",         ssaoEnabled);
ssaoCamera.set_initial_state(ssaoNP.get_state());

framebufferTextureArguments.name = "ssaoBlur";

ssaoBlurFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
ssaoBlurBuffer = ssaoBlurFramebufferTexture.buffer;
ssaoBlurNP     = ssaoBlurFramebufferTexture.shaderNP;
ssaoBlurBuffer.set_sort(ssaoBuffer.get_sort() + 1);
ssaoBlurNP.set_shader(kuwaharaFilterShader);
ssaoBlurNP.set_shader_input("colorTexture", ssaoBuffer.get_texture());
ssaoBlurNP.set_shader_input("parameters",   LVecBase2f(1, 0));
ssaoBlurFramebufferTexture.camera.set_initial_state(ssaoBlurNP.get_state());
ssaoBlurTexture = ssaoBlurBuffer.get_texture();

framebufferTextureArguments.rgbaBits   = rgba16;
framebufferTextureArguments.clearColor = LColor(0, 0, 0, 0);
framebufferTextureArguments.name       = "refractionUv";

refractionUvFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
refractionUvBuffer = refractionUvFramebufferTexture.buffer;
refractionUvCamera = refractionUvFramebufferTexture.camera;
refractionUvNP     = refractionUvFramebufferTexture.shaderNP;
refractionUvBuffer.set_sort(geometryBuffer1.get_sort() + 1);
refractionUvNP.set_shader(screenSpaceRefractionShader);
refractionUvNP.set_shader_input("positionFromTexture", positionTexture1);
refractionUvNP.set_shader_input("positionToTexture",   positionTexture0);
refractionUvNP.set_shader_input("normalFromTexture",   normalTexture1);
refractionUvNP.set_shader_input("lensProjection",      geometryCameraLens0.get_projection_mat());
refractionUvNP.set_shader_input("enabled",             refractionEnabled);
refractionUvNP.set_shader_input("rior",                rior);
refractionUvCamera.set_initial_state(refractionUvNP.get_state());
refractionUvTexture = refractionUvBuffer.get_texture();

framebufferTextureArguments.name = "reflectionUv";

reflectionUvFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
reflectionUvBuffer = reflectionUvFramebufferTexture.buffer;
reflectionUvCamera = reflectionUvFramebufferTexture.camera;
reflectionUvNP     = reflectionUvFramebufferTexture.shaderNP;
reflectionUvBuffer.set_sort(geometryBuffer1.get_sort() + 1);
reflectionUvNP.set_shader(screenSpaceReflectionShader);
reflectionUvNP.set_shader_input("positionTexture", positionTexture1);
reflectionUvNP.set_shader_input("normalTexture",   normalTexture1);
reflectionUvNP.set_shader_input("maskTexture",     reflectionMaskTexture);
reflectionUvNP.set_shader_input("lensProjection",  geometryCameraLens0.get_projection_mat());
reflectionUvNP.set_shader_input("enabled",         reflectionEnabled);
reflectionUvCamera.set_initial_state(reflectionUvNP.get_state());
reflectionUvTexture = reflectionUvBuffer.get_texture();

framebufferTextureArguments.rgbaBits = rgba8;
framebufferTextureArguments.aux_rgba = 1;
framebufferTextureArguments.useScene = True;
framebufferTextureArguments.name     = "base";

baseFramebufferTexture = generateFramebufferTexture( framebufferTextureArguments);
baseBuffer = baseFramebufferTexture.buffer;
baseCamera = baseFramebufferTexture.camera;
baseNP     = baseFramebufferTexture.shaderNP;
baseBuffer.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_0);
baseBuffer.set_clear_active(3, True);
baseBuffer.set_clear_value( 3, framebufferTextureArguments.clearColor);
baseBuffer.set_sort(max( ssaoBlurBuffer.get_sort() + 1, UNSORTED_RENDER_SORT_ORDER + 1));
baseNP.set_shader(baseShader);
baseNP.set_shader_input("pi",                PI_SHADER_INPUT);
baseNP.set_shader_input("gamma",             GAMMA_SHADER_INPUT);
baseNP.set_shader_input("ssaoBlurTexture",   ssaoBlurTexture);
baseNP.set_shader_input("flowTexture",       stillFlowTexture);
baseNP.set_shader_input("normalMapsEnabled", normalMapsEnabled);
baseNP.set_shader_input("blinnPhongEnabled", blinnPhongEnabled);
baseNP.set_shader_input("fresnelEnabled",    fresnelEnabled);
baseNP.set_shader_input("rimLightEnabled",   rimLightEnabled);
baseNP.set_shader_input("celShadingEnabled", celShadingEnabled);
baseNP.set_shader_input("flowMapsEnabled",   flowMapsEnabled);
baseNP.set_shader_input("specularOnly",      LVecBase2f(0, 0));
baseNP.set_shader_input("isParticle",        LVecBase2f(0, 0));
baseNP.set_shader_input("isWater",           LVecBase2f(0, 0));
baseNP.set_shader_input("sunPosition",       LVecBase2f(sunlightP, 0));
baseCamera.set_initial_state(baseNP.get_state());
baseCamera.set_tag_state_key("baseBuffer");
baseCamera.set_tag_state("isParticle", isSmokeNP.get_state());
baseCamera.set_tag_state("isWater",    isWaterNP.get_state());
baseCamera.set_camera_mask(BitMask32.bit(6));
smokeNP.set_tag("baseBuffer", "isParticle");
waterNP.set_tag("baseBuffer", "isWater");
baseTexture     = baseBuffer.get_texture(0);
specularTexture = baseBuffer.get_texture(1);

framebufferTextureArguments.aux_rgba = 0;
framebufferTextureArguments.useScene = False;
framebufferTextureArguments.name     = "refraction";

refractionFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
refractionBuffer = refractionFramebufferTexture.buffer;
refractionCamera = refractionFramebufferTexture.camera;
refractionNP     = refractionFramebufferTexture.shaderNP;
refractionBuffer.set_sort(baseBuffer.get_sort() + 1);
refractionNP.set_shader(refractionShader);
refractionNP.set_shader_input("pi",                     PI_SHADER_INPUT);
refractionNP.set_shader_input("gamma",                  GAMMA_SHADER_INPUT);
refractionNP.set_shader_input("uvTexture",              refractionUvTexture);
refractionNP.set_shader_input("maskTexture",            refractionMaskTexture);
refractionNP.set_shader_input("positionFromTexture",    positionTexture1);
refractionNP.set_shader_input("positionToTexture",      positionTexture0);
refractionNP.set_shader_input("backgroundColorTexture", baseTexture);
refractionNP.set_shader_input("sunPosition",            LVecBase2f(sunlightP, 0));
refractionCamera.set_initial_state(refractionNP.get_state());
refractionTexture = refractionBuffer.get_texture();

framebufferTextureArguments.name = "foam";

foamFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
foamBuffer = foamFramebufferTexture.buffer;
foamCamera = foamFramebufferTexture.camera;
foamNP     = foamFramebufferTexture.shaderNP;
foamBuffer.set_sort(geometryBuffer1.get_sort() + 1);
foamNP.set_shader(foamShader);
foamNP.set_shader_input("pi",                  PI_SHADER_INPUT);
foamNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
foamNP.set_shader_input("maskTexture",         foamMaskTexture);
foamNP.set_shader_input("foamDepth",           foamDepth);
foamNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
foamNP.set_shader_input("viewWorldMat",        currentViewWorldMat);
foamNP.set_shader_input("positionFromTexture", positionTexture1);
foamNP.set_shader_input("positionToTexture",   positionTexture0);
foamCamera.set_initial_state(foamNP.get_state());
foamTexture = foamBuffer.get_texture();

framebufferTextureArguments.name = "reflectionColor";

reflectionColorFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
reflectionColorBuffer = reflectionColorFramebufferTexture.buffer;
reflectionColorCamera = reflectionColorFramebufferTexture.camera;
reflectionColorNP     = reflectionColorFramebufferTexture.shaderNP;
reflectionColorBuffer.set_sort(refractionBuffer.get_sort() + 1);
reflectionColorNP.set_shader(reflectionColorShader);
reflectionColorNP.set_shader_input("colorTexture", refractionTexture);
reflectionColorNP.set_shader_input("uvTexture",    reflectionUvTexture);
reflectionColorCamera.set_initial_state(reflectionColorNP.get_state());
reflectionColorTexture = reflectionColorBuffer.get_texture();

framebufferTextureArguments.name = "reflectionColorBlur";

reflectionColorBlurFramebufferTexture = generateFramebufferTexture( framebufferTextureArguments);
reflectionColorBlurBuffer = reflectionColorBlurFramebufferTexture.buffer;
reflectionColorBlurCamera = reflectionColorBlurFramebufferTexture.camera;
reflectionColorBlurNP     = reflectionColorBlurFramebufferTexture.shaderNP;
reflectionColorBlurBuffer.set_sort(reflectionColorBuffer.get_sort() + 1);
reflectionColorBlurNP.set_shader(boxBlurShader);
reflectionColorBlurNP.set_shader_input("colorTexture", reflectionColorTexture);
reflectionColorBlurNP.set_shader_input("parameters",   LVecBase2f(8, 1));
reflectionColorBlurCamera.set_initial_state(reflectionColorBlurNP.get_state());
reflectionColorBlurTexture = reflectionColorBlurBuffer.get_texture();

framebufferTextureArguments.name = "reflection";

reflectionFramebufferTexture = generateFramebufferTexture( framebufferTextureArguments);
reflectionBuffer = reflectionFramebufferTexture.buffer;
reflectionNP     = reflectionFramebufferTexture.shaderNP;
reflectionBuffer.set_sort(reflectionColorBlurBuffer.get_sort() + 1);
reflectionNP.set_shader(reflectionShader);
reflectionNP.set_shader_input("colorTexture",     reflectionColorTexture);
reflectionNP.set_shader_input("colorBlurTexture", reflectionColorBlurTexture);
reflectionNP.set_shader_input("maskTexture",      reflectionMaskTexture);
reflectionFramebufferTexture.camera.set_initial_state(reflectionNP.get_state());
reflectionTexture = reflectionBuffer.get_texture();

framebufferTextureArguments.name = "baseCombine";

baseCombineFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
baseCombineBuffer = baseCombineFramebufferTexture.buffer;
baseCombineCamera = baseCombineFramebufferTexture.camera;
baseCombineNP     = baseCombineFramebufferTexture.shaderNP;
baseCombineBuffer.set_sort(reflectionBuffer.get_sort() + 1);
baseCombineNP.set_shader(baseCombineShader);
baseCombineNP.set_shader_input("baseTexture",       baseTexture);
baseCombineNP.set_shader_input("refractionTexture", refractionTexture);
baseCombineNP.set_shader_input("foamTexture",       foamTexture);
baseCombineNP.set_shader_input("reflectionTexture", reflectionTexture);
baseCombineNP.set_shader_input("specularTexture",   specularTexture);
baseCombineCamera.set_initial_state(baseCombineNP.get_state());
baseCombineTexture = baseCombineBuffer.get_texture();

framebufferTextureArguments.name = "sharpen";

sharpenFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments)
sharpenBuffer = sharpenFramebufferTexture.buffer;
sharpenNP     = sharpenFramebufferTexture.shaderNP;
sharpenBuffer.set_sort(baseCombineBuffer.get_sort() + 1);
sharpenNP.set_shader(sharpenShader);
sharpenNP.set_shader_input("colorTexture", baseCombineTexture);
sharpenNP.set_shader_input("enabled",      sharpenEnabled);
sharpenCamera = sharpenFramebufferTexture.camera;
sharpenCamera.set_initial_state(sharpenNP.get_state());
sharpenTexture = sharpenBuffer.get_texture();

framebufferTextureArguments.name = "posterize";

posterizeFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
posterizeBuffer = posterizeFramebufferTexture.buffer;
posterizeNP     = posterizeFramebufferTexture.shaderNP;
posterizeBuffer.set_sort(sharpenBuffer.get_sort() + 1);
posterizeNP.set_shader(posterizeShader);
posterizeNP.set_shader_input("gamma",           GAMMA_SHADER_INPUT);
posterizeNP.set_shader_input("colorTexture",    sharpenTexture);
posterizeNP.set_shader_input("positionTexture", positionTexture2);
posterizeNP.set_shader_input("enabled",         posterizeEnabled);
posterizeCamera = posterizeFramebufferTexture.camera;
posterizeCamera.set_initial_state(posterizeNP.get_state());
posterizeTexture = posterizeBuffer.get_texture();

framebufferTextureArguments.name = "bloom";

bloomFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
bloomBuffer = bloomFramebufferTexture.buffer;
bloomCamera = bloomFramebufferTexture.camera;
bloomNP     = bloomFramebufferTexture.shaderNP;
bloomBuffer.set_sort(posterizeBuffer.get_sort() + 1);
bloomNP.set_shader(bloomShader);
bloomNP.set_shader_input("colorTexture", posterizeTexture);
bloomNP.set_shader_input("enabled",      bloomEnabled);
bloomCamera.set_initial_state(bloomNP.get_state());
bloomTexture = bloomBuffer.get_texture();

framebufferTextureArguments.name = "sceneCombine";

sceneCombineFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
sceneCombineBuffer = sceneCombineFramebufferTexture.buffer;
sceneCombineCamera = sceneCombineFramebufferTexture.camera;
sceneCombineNP     = sceneCombineFramebufferTexture.shaderNP;
sceneCombineBuffer.set_sort(bloomBuffer.get_sort() + 1);
sceneCombineNP.set_shader(sceneCombineShader);
sceneCombineNP.set_shader_input("pi",                  PI_SHADER_INPUT);
sceneCombineNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
sceneCombineNP.set_shader_input("lookupTableTextureN", colorLookupTableTextureN);
sceneCombineNP.set_shader_input("backgroundColor0",    backgroundColor[0]);
sceneCombineNP.set_shader_input("backgroundColor1",    backgroundColor[1]);
sceneCombineNP.set_shader_input("baseTexture",         posterizeTexture);
sceneCombineNP.set_shader_input("bloomTexture",        bloomTexture);
sceneCombineNP.set_shader_input("fogTexture",          fogTexture);
sceneCombineNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
sceneCombineTexture = sceneCombineBuffer.get_texture();
sceneCombineCamera.set_initial_state(sceneCombineNP.get_state());

framebufferTextureArguments.clearColor = backgroundColor[1];
framebufferTextureArguments.name       = "outOfFocus";

outOfFocusFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
outOfFocusBuffer = outOfFocusFramebufferTexture.buffer;
outOfFocusCamera = outOfFocusFramebufferTexture.camera;
outOfFocusNP     = outOfFocusFramebufferTexture.shaderNP;
outOfFocusBuffer.set_sort(sceneCombineBuffer.get_sort() + 1);
outOfFocusNP.set_shader(boxBlurShader);
outOfFocusNP.set_shader_input("colorTexture", sceneCombineTexture);
outOfFocusNP.set_shader_input("parameters",   LVecBase2f(2, 2));
outOfFocusCamera.set_initial_state(outOfFocusNP.get_state());
outOfFocusTexture = outOfFocusBuffer.get_texture();

framebufferTextureArguments.name = "dilatedOutOfFocus";

dilatedOutOfFocusFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments)
dilatedOutOfFocusBuffer = dilatedOutOfFocusFramebufferTexture.buffer;
dilatedOutOfFocusCamera = dilatedOutOfFocusFramebufferTexture.camera;
dilatedOutOfFocusNP     = dilatedOutOfFocusFramebufferTexture.shaderNP;
dilatedOutOfFocusBuffer.set_sort(outOfFocusBuffer.get_sort() + 1);
dilatedOutOfFocusNP.set_shader(dilationShader);
dilatedOutOfFocusNP.set_shader_input("colorTexture", outOfFocusTexture);
dilatedOutOfFocusNP.set_shader_input("parameters",   LVecBase2f(4, 2));
dilatedOutOfFocusCamera.set_initial_state(dilatedOutOfFocusNP.get_state());
dilatedOutOfFocusTexture = dilatedOutOfFocusBuffer.get_texture();

framebufferTextureArguments.aux_rgba = 1;
framebufferTextureArguments.name     = "depthOfField";

depthOfFieldFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
depthOfFieldBuffer = depthOfFieldFramebufferTexture.buffer;
depthOfFieldNP     = depthOfFieldFramebufferTexture.shaderNP;
depthOfFieldBuffer.add_render_texture( Texture(), GraphicsOutput.RTM_bind_or_copy, GraphicsOutput.RTP_aux_rgba_0);
depthOfFieldBuffer.set_clear_active(3, True);
depthOfFieldBuffer.set_clear_value( 3, framebufferTextureArguments.clearColor);
depthOfFieldBuffer.set_sort(dilatedOutOfFocusBuffer.get_sort() + 1);
depthOfFieldNP.set_shader(depthOfFieldShader);
depthOfFieldNP.set_shader_input("positionTexture",   positionTexture0);
depthOfFieldNP.set_shader_input("focusTexture",      sceneCombineTexture);
depthOfFieldNP.set_shader_input("outOfFocusTexture", dilatedOutOfFocusTexture);
depthOfFieldNP.set_shader_input("mouseFocusPoint",   mouseFocusPoint);
depthOfFieldNP.set_shader_input("nearFar",           cameraNearFar);
depthOfFieldNP.set_shader_input("enabled",           depthOfFieldEnabled);
depthOfFieldCamera = depthOfFieldFramebufferTexture.camera;
depthOfFieldCamera.set_initial_state(depthOfFieldNP.get_state());
depthOfFieldTexture0 = depthOfFieldBuffer.get_texture(0);
depthOfFieldTexture1 = depthOfFieldBuffer.get_texture(1);

framebufferTextureArguments.aux_rgba = 0;
framebufferTextureArguments.name     = "outline";

outlineFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
outlineBuffer = outlineFramebufferTexture.buffer;
outlineCamera = outlineFramebufferTexture.camera;
outlineNP     = outlineFramebufferTexture.shaderNP;
outlineBuffer.set_sort(depthOfFieldBuffer.get_sort() + 1);
outlineNP.set_shader(outlineShader);
outlineNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
outlineNP.set_shader_input("positionTexture",     positionTexture0);
outlineNP.set_shader_input("colorTexture",        depthOfFieldTexture0);
outlineNP.set_shader_input("noiseTexture",        colorNoiseTexture);
outlineNP.set_shader_input("depthOfFieldTexture", depthOfFieldTexture1);
outlineNP.set_shader_input("fogTexture",          fogTexture);
outlineNP.set_shader_input("nearFar",             cameraNearFar);
outlineNP.set_shader_input("enabled",             outlineEnabled);
outlineCamera.set_initial_state(outlineNP.get_state());
outlineTexture = outlineBuffer.get_texture();

framebufferTextureArguments.name = "painterly";

painterlyFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
painterlyBuffer = painterlyFramebufferTexture.buffer;
painterlyNP     = painterlyFramebufferTexture.shaderNP;
painterlyBuffer.set_sort(outlineBuffer.get_sort() + 1);
painterlyNP.set_shader(kuwaharaFilterShader);
painterlyNP.set_shader_input("colorTexture", outlineTexture);
painterlyNP.set_shader_input("parameters",   LVecBase2f(0, 0));
painterlyCamera = painterlyFramebufferTexture.camera;
painterlyCamera.set_initial_state(painterlyNP.get_state());
painterlyTexture = painterlyBuffer.get_texture();

framebufferTextureArguments.name = "pixelize";

pixelizeFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
pixelizeBuffer = pixelizeFramebufferTexture.buffer;
pixelizeNP     = pixelizeFramebufferTexture.shaderNP;
pixelizeBuffer.set_sort(painterlyBuffer.get_sort() + 1);
pixelizeNP.set_shader(pixelizeShader);
pixelizeNP.set_shader_input("colorTexture",    painterlyTexture);
pixelizeNP.set_shader_input("positionTexture", positionTexture2);
pixelizeNP.set_shader_input("parameters",      LVecBase2f(5, 0));
pixelizeNP.set_shader_input("enabled",         pixelizeEnabled);
pixelizeCamera = pixelizeFramebufferTexture.camera;
pixelizeCamera.set_initial_state(pixelizeNP.get_state());
pixelizeTexture = pixelizeBuffer.get_texture();

framebufferTextureArguments.name = "motionBlur";

motionBlurFramebufferTexture = generateFramebufferTexture( framebufferTextureArguments);
motionBlurBuffer = motionBlurFramebufferTexture.buffer;
motionBlurNP     = motionBlurFramebufferTexture.shaderNP;
motionBlurBuffer.set_sort(pixelizeBuffer.get_sort() + 1);
motionBlurNP.set_shader(motionBlurShader);
motionBlurNP.set_shader_input("previousViewWorldMat",    previousViewWorldMat);
motionBlurNP.set_shader_input("worldViewMat",            render.get_transform(cameraNP).get_mat());
motionBlurNP.set_shader_input("lensProjection",          geometryCameraLens2.get_projection_mat());
motionBlurNP.set_shader_input("positionTexture",         positionTexture2);
motionBlurNP.set_shader_input("colorTexture",            pixelizeTexture);
motionBlurNP.set_shader_input("motionBlurEnabled",       motionBlurEnabled);
motionBlurNP.set_shader_input("parameters",              LVecBase2f(2, 1.0));
motionBlurCamera = motionBlurFramebufferTexture.camera;
motionBlurCamera.set_initial_state(motionBlurNP.get_state());
motionBlurTexture = motionBlurBuffer.get_texture();

framebufferTextureArguments.name = "filmGrain";

filmGrainFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
filmGrainBuffer = filmGrainFramebufferTexture.buffer;
filmGrainNP     = filmGrainFramebufferTexture.shaderNP;
filmGrainBuffer.set_sort(motionBlurBuffer.get_sort() + 1);
filmGrainNP.set_shader(filmGrainShader);
filmGrainNP.set_shader_input("pi",           PI_SHADER_INPUT);
filmGrainNP.set_shader_input("colorTexture", motionBlurTexture);
filmGrainNP.set_shader_input("enabled",      filmGrainEnabled);
filmGrainCamera = filmGrainFramebufferTexture.camera;
filmGrainCamera.set_initial_state(filmGrainNP.get_state());
filmGrainTexture = filmGrainBuffer.get_texture();

framebufferTextureArguments.name = "lookupTable";

lookupTableFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
lookupTableBuffer = lookupTableFramebufferTexture.buffer;
lookupTableNP     = lookupTableFramebufferTexture.shaderNP;
lookupTableBuffer.set_sort(filmGrainBuffer.get_sort() + 1);
lookupTableNP.set_shader(lookupTableShader);
lookupTableNP.set_shader_input("pi",                  PI_SHADER_INPUT);
lookupTableNP.set_shader_input("gamma",               GAMMA_SHADER_INPUT);
lookupTableNP.set_shader_input("colorTexture",        filmGrainTexture);
lookupTableNP.set_shader_input("lookupTableTextureN", colorLookupTableTextureN);
lookupTableNP.set_shader_input("lookupTableTexture0", colorLookupTableTexture0);
lookupTableNP.set_shader_input("lookupTableTexture1", colorLookupTableTexture1);
lookupTableNP.set_shader_input("sunPosition",         LVecBase2f(sunlightP, 0));
lookupTableNP.set_shader_input("enabled",             lookupTableEnabled);
lookupTableCamera = lookupTableFramebufferTexture.camera;
lookupTableCamera.set_initial_state(lookupTableNP.get_state());
lookupTableTexture = lookupTableBuffer.get_texture();

framebufferTextureArguments.name = "gammaCorrection";

gammaCorrectionFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
gammaCorrectionBuffer = gammaCorrectionFramebufferTexture.buffer;
gammaCorrectionNP     = gammaCorrectionFramebufferTexture.shaderNP;
gammaCorrectionBuffer.set_sort(lookupTableBuffer.get_sort() + 1);
gammaCorrectionNP.set_shader(gammaCorrectionShader);
gammaCorrectionNP.set_shader_input("gamma",        GAMMA_SHADER_INPUT);
gammaCorrectionNP.set_shader_input("colorTexture", lookupTableTexture);
gammaCorrectionCamera = gammaCorrectionFramebufferTexture.camera;
gammaCorrectionCamera.set_initial_state(gammaCorrectionNP.get_state());
gammaCorrectionTexture = gammaCorrectionBuffer.get_texture();

framebufferTextureArguments.name = "chromaticAberration";

chromaticAberrationFramebufferTexture =generateFramebufferTexture( framebufferTextureArguments);
chromaticAberrationBuffer = chromaticAberrationFramebufferTexture.buffer;
chromaticAberrationNP     = chromaticAberrationFramebufferTexture.shaderNP;
chromaticAberrationBuffer.set_sort(gammaCorrectionBuffer.get_sort() + 1);
chromaticAberrationNP.set_shader(chromaticAberrationShader);
chromaticAberrationNP.set_shader_input("mouseFocusPoint", mouseFocusPoint);
chromaticAberrationNP.set_shader_input("colorTexture",    gammaCorrectionTexture);
chromaticAberrationNP.set_shader_input("enabled",         chromaticAberrationEnabled);
chromaticAberrationCamera = chromaticAberrationFramebufferTexture.camera;
chromaticAberrationCamera.set_initial_state(chromaticAberrationNP.get_state());

base.win.set_sort(chromaticAberrationBuffer.get_sort() + 1);

showBufferIndex = 0;

bufferArray = [("Positions 0",          geometryBuffer0,           0)
, ("Normals 0",            geometryBuffer0,           1)
, ("Positions 1",          geometryBuffer1,           0)
, ("Normals 1",            geometryBuffer1,           1)
, ("Reflection Mask",      geometryBuffer1,           2)
, ("Refraction Mask",      geometryBuffer1,           3)
, ("Foam Mask",            geometryBuffer1,           4)
, ("Positions 2",          geometryBuffer2,           0)
, ("Smoke Mask",           geometryBuffer2,           1)
, ("SSAO",                 ssaoBuffer,                0)
, ("SSAO Blur",            ssaoBlurBuffer,            0)
, ("Refraction UV",        refractionUvBuffer,        0)
, ("Refraction",           refractionBuffer,          0)
, ("Reflection UV",        reflectionUvBuffer,        0)
, ("Reflection Color",     reflectionColorBuffer,     0)
, ("Reflection Blur",      reflectionColorBlurBuffer, 0)
, ("Reflection",           reflectionBuffer,          0)
, ("Foam",                 foamBuffer,                0)
, ("Base",                 baseBuffer,                0)
, ("Specular",             baseBuffer,                1)
, ("Base Combine",         baseCombineBuffer,         0)
, ("Painterly",            painterlyBuffer,           0)
, ("Posterize",            posterizeBuffer,           0)
, ("Bloom",                bloomBuffer,               0)
, ("Outline",              outlineBuffer,             0)
, ("Fog",                  fogBuffer,                 0)
, ("Scene Combine",        sceneCombineBuffer,        0)
, ("Out of Focus",         outOfFocusBuffer,          0)
, ("Dilation",             dilatedOutOfFocusBuffer,   0)
, ("Depth of Field Blur",  depthOfFieldBuffer,        1)
, ("Depth of Field",       depthOfFieldBuffer,        0)
, ("Pixelize",             pixelizeBuffer,            0)
, ("Motion Blur",          motionBlurBuffer,          0)
, ("Film Grain",           filmGrainBuffer,           0)
, ("Lookup Table",         lookupTableBuffer,         0)
, ("Gamma Correction",     gammaCorrectionBuffer,     0)
, ("Chromatic Aberration", chromaticAberrationBuffer, 0)
]

showBufferIndex = len(bufferArray) - 1;

showBuffer( render2d, None, bufferArray[showBufferIndex], False);
    
def taskus(task):
    global previousViewWorldMat
    global currentViewWorldMat
    currentViewWorldMat = cameraNP.get_transform(render).get_mat()
    foamNP.set_shader_input("viewWorldMat", currentViewWorldMat)
    motionBlurNP.set_shader_input("previousViewWorldMat",   previousViewWorldMat)
    previousViewWorldMat = currentViewWorldMat
    return task.cont
    
if __name__ == '__main__':
    taskMgr.add(taskus)
#    render2d.hide()
    base.run()
