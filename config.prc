load-display pandagl

win-origin -2 -2
win-size 1200 900

fullscreen #f

framebuffer-hardware #t
framebuffer-software #f

depth-bits 1
color-bits 1 1 1
alpha-bits 0
stencil-bits 0
multisamples 0

notify-level warning
default-directnotify-level warning

model-path    $MAIN_DIR

want-directtools  #f
want-tk           #f

want-pstats            #f
show-frame-rate-meter  #f

audio-library-name p3openal_audio

use-movietexture #t

hardware-animated-vertices #f

model-cache-dir $XDG_CACHE_HOME/panda3d
model-cache-textures #f

basic-shaders-only #f

gl-coordinate-system default

textures-auto-power-2 1
textures-power-2 down
