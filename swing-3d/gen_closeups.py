"""Generate close-up reference frames of "The Swing" with gpt-image-2 (OpenAI images/edits).

Usage: python3 gen_closeups.py [shot_id ...]   (no args = all shots)
Each shot sends the matching crop + the full painting as references.
"""
import base64, json, sys, subprocess, concurrent.futures as cf
from pathlib import Path

ROOT = Path(__file__).parent
OUT = ROOT / "closeups"
OUT.mkdir(exist_ok=True)
SIZE = "1024x1536"

STYLE = (
    "This must look like a detail of the SAME oil painting: Jean-Honoré Fragonard, 'The Swing' (1767), "
    "Rococo oil on canvas. Keep the exact palette (silvery blue-green foliage, pale pink silk, soft "
    "cool daylight from the upper left), loose feathery brushwork, visible canvas texture and impasto "
    "highlights. It is a close-up as if a camera flew INTO the painted world, which is now a real 3D "
    "space, but everything is still made of paint. Time is frozen: nothing blurs, fabric and leaves hang "
    "mid-motion. Do not add any new characters or objects, no text, no frame, no modern elements, "
    "no photorealism, no CGI look."
)

SHOTS = {
    "1a_shoe_hero": ("1_shoe",
        "Extreme close-up of the small pink silk mule slipper frozen in mid-air after being kicked off, "
        "hanging against the blue-grey painted clouds and the soft pale tree trunks behind it. The slipper "
        "fills the centre of the frame, seen slightly from below and from the side, with its pointed toe, "
        "small heel and silver-white trim. Shallow painterly depth of field: the cupid statue's pedestal is a "
        "soft blurred shape at the left edge, a hint of the woman's raised white-stockinged foot and pink "
        "ruffles out of focus at the lower right."),
    "1b_shoe_orbit": ("1_shoe",
        "Close-up, the camera has orbited around the frozen pink silk slipper and now looks back toward the "
        "swing. The slipper is sharp in the lower-left foreground, small-heeled and pointed, life-size (it "
        "occupies about a fifth of the frame). Behind it, filling the upper right and softly out of focus, the "
        "young woman in the billowing pink dress on the swing, seen at mid distance from the left: her white-"
        "stockinged foot kicked toward the camera, her straw hat, the golden ropes, and dark green trees and "
        "blue clouds behind her. Only air, clouds and foliage around the slipper, no water, no ground."),
    "2a_woman_hero": ("2_woman",
        "Intimate close-up portrait of the young woman on the swing, frozen at the top of the arc: her "
        "flushed cheeks, playful sideways glance and slight smile, the straw hat with pink ribbons and "
        "flowers, the lace ruff at her throat, the blue bow at her bodice, her hand holding the thick golden "
        "rope. Behind her, cool blue painted clouds and dark foliage. Her face must match the painting exactly."),
    "2b_woman_dress": ("2_woman",
        "Low-angle close-up from beneath and slightly behind the swing, looking up: the cascading layers of "
        "pink silk ruffles and white lace petticoats frozen mid-billow, the red velvet swing seat, the rope "
        "crossing the frame, her white-stockinged leg kicked up toward the sky, and painted clouds and "
        "leaves above. Every ruffle frozen in the air like a sculpture of paint."),
    "3a_lover_hero": ("3_lover",
        "Close-up of the young man reclining in the rose bushes, looking up with an astonished, enraptured "
        "expression, powdered grey wig, flushed cheeks, grey silk coat with a pink rose on the lapel, lace "
        "jabot, his left arm raised high holding out his black tricorn hat toward the swing. Clusters of "
        "white, pink and blue-green painted flowers frame him in the foreground, slightly out of focus."),
    "3b_lover_pov": ("3_lover",
        "View from just behind the young man's shoulder, nestled among the flowering bushes: his raised arm "
        "and black hat in the foreground, and above them, filling the upper frame, the woman on the swing in "
        "her pink dress flying overhead against the clouds, her foot and the falling slipper visible. Over-"
        "the-shoulder point of view from his hiding place, deep layered parallax of leaves and blossoms."),
    "4a_cupid_hero": ("4_cupid",
        "Close-up of the white marble statue of Cupid (after Falconet) on its pedestal at the left edge of "
        "the garden: the chubby winged child holding one finger to his lips in a 'shhh' gesture, his eyes "
        "glancing sideways toward the swing. Weathered pale stone painted with cool blue-grey shadows, "
        "small wings, curly hair. Behind him, soft trees and the misty blue sky."),
    "4b_cupid_reveal": ("4_cupid",
        "View from just behind and beside the Cupid statue's head and shoulder, his raised finger to his "
        "lips visible in the foreground, looking out over the whole garden scene below him: the woman in "
        "pink on the swing, the young man in the bushes, the older man in the shadows pulling the ropes, "
        "all small and frozen in the distance under the great dark trees. A secret witness's point of view."),
}


def gen(shot_id):
    crop, desc = SHOTS[shot_id]
    prompt = desc + "\n\n" + STYLE
    out = OUT / f"{shot_id}.png"
    cmd = [
        "curl", "-sS", "-N", "-m", "900", "https://api.openai.com/v1/images/edits",
        "-F", "model=gpt-image-2", "-F", f"size={SIZE}", "-F", "quality=high", "-F", "stream=true", "-F", "partial_images=2",
        "-F", f"prompt={prompt}",
        "-F", f"image[]=@{ROOT / 'crops' / (crop + '.png')}",
        "-F", f"image[]=@{ROOT / 'source' / 'the_swing_small.jpg'}",
    ]
    r = subprocess.run(cmd, capture_output=True, text=True)
    try:
        # Streamed (SSE) so the request survives the ~30s proxy idle timeout; keep the final event.
        events = [json.loads(l[5:]) for l in r.stdout.splitlines() if l.startswith("data:")]
        done = next(e for e in events if e.get("type") == "image_edit.completed")
        out.write_bytes(base64.b64decode(done["b64_json"]))
        return f"OK  {out.name}"
    except Exception:
        return f"ERR {shot_id}: {r.stdout[:400]} {r.stderr[:200]}"


if __name__ == "__main__":
    ids = sys.argv[1:] or list(SHOTS)
    with cf.ThreadPoolExecutor(len(ids)) as ex:
        for res in ex.map(gen, ids):
            print(res, flush=True)
