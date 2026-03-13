import sys
import re

files = [
    r"c:\Git\ProjectBeast\BeastEngine\Physics\CharacterController.h",
    r"c:\Git\ProjectBeast\BeastEngine\Physics\CharacterController.cpp"
]

replacements = {
    r"\bcollider_\b": "m_collider",
    r"\brigidBody_\b": "m_rigidBody",
    r"\bposition_\b": "m_position",
    r"\bprevPosition_\b": "m_prevPosition",
    r"\bverticalVelocity_\b": "m_verticalVelocity",
    r"\bgravity_\b": "m_gravity",
    r"\bradius_\b": "m_radius",
    r"\bheight_\b": "m_height",
    r"\bisInited_\b": "m_isInited",
    r"\bisJump_\b": "m_isJump",
    r"\bisOnGround_\b": "m_isOnGround",
    r"\bisRequestTeleport_\b": "m_isRequestTeleport"
}

try:
    for file_path in files:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()
        
        for k, v in replacements.items():
            content = re.sub(k, v, content)
            
        with open(file_path, "w", encoding="utf-8") as f:
            f.write(content)
    print("SUCCESS")
except Exception as e:
    print("ERROR:", e)
