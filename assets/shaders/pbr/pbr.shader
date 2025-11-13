technique PBR

vertex   = pbr.vert
fragment = pbr.frag

[defines]
USE_PBR = 1

[properties]
uAlbedo    : vec3  = 1.0,1.0,1.0
uMetallic  : float = 0.1
uRoughness : float = 0.5
uAO        : float = 1.0
