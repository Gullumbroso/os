# The Swing — טיסת FPV לתוך הציור (Seedance)

הכל קפוא בזמן, רק המצלמה זזה. 4 סגמנטים, כל אחד נגמר בדיוק איפה שהבא מתחיל, כך שאפשר לחבר אותם ברצף אחד.

| # | מאיפה → לאן | תמונת מסלול (להעלות ל-Seedance) | Start frame | End frame / רפרנס |
|---|---|---|---|---|
| 1 | כניסה לציור → הקפה סביב הנעל | `paths/segment1_enter_to_shoe.jpg` | `source/the_swing.jpg` | `closeups/1a_shoe_hero.png`, `closeups/1b_shoe_orbit.png` |
| 2 | הנעל → פני האישה בנדנדה | `paths/segment2_shoe_to_woman.jpg` | `closeups/1b_shoe_orbit.png` | `closeups/2a_woman_hero.png` |
| 3 | האישה → מתחת לשמלה → המאהב בשיחים | `paths/segment3_woman_to_lover.jpg` | `closeups/2a_woman_hero.png` | `closeups/2b_woman_dress.png`, `closeups/3a_lover_hero.png`, `closeups/3b_lover_pov.png` |
| 4 | המאהב → עלייה אל פסל הקופידון (״שששש״) | `paths/segment4_lover_to_cupid.jpg` | `closeups/3b_lover_pov.png` | `closeups/4a_cupid_hero.png`, `closeups/4b_cupid_reveal.png` |

**איך להשתמש:** בכל סגמנט – מעלים את תמונת המסלול (קווים אדומים + Point 1/2) כתמונה הראשית, ואת התקריבים כ-reference images (או first/last frame אם יש). מדביקים את הפרומפט המלא למטה, כשה-DESCRIPTION OF DIRECTION מוחלף בטקסט של הסגמנט.

התקריבים נוצרו עם `gpt-image-2` (OpenAI images/edits) – ראו `gen_closeups.py`. כל תקריב קיבל כרפרנס את החיתוך מהציור + הציור המלא.

---

## DESCRIPTION OF DIRECTION – לכל סגמנט

**Segment 1 – Enter → the shoe**
> At point 1, start low inside the flowering bushes at the bottom of the painting and glide slowly forward and upward into the garden, rising past the young man in the roses toward the small pink slipper frozen in mid-air. Slow down as you reach it, then make one slow, smooth orbit around the slipper from left to right, ending at point 2 looking back past the slipper toward the woman on the swing.

**Segment 2 – The shoe → the woman**
> At point 1, start beside the frozen slipper looking toward the swing. Drift slowly forward and dip under her raised white-stockinged foot, glide along the frozen pink ruffles of her dress, then curve gently up and around the front of her, ending at point 2 in a close, eye-level view of her face, her straw hat and her sideways glance.

**Segment 3 – The woman → the lover**
> At point 1, start on the woman's face. Slowly descend beneath the swing, passing under the red seat and the frozen billowing petticoats, then dive gently down into the flowering bushes, weaving between the blossoms toward the young man lying in the roses. End at point 2 just behind his shoulder, looking up past his raised hat toward the woman on the swing.

**Segment 4 – The lover → Cupid**
> At point 1, start behind the young man's shoulder. Rise slowly and vertically along the left side of the garden, past the carved stone pedestal, up to the marble Cupid statue holding his finger to his lips. Slowly orbit behind his head and end at point 2 looking over his shoulder at the whole frozen scene below: the woman on the swing, the young man in the bushes, and the older man in the shadows.

---

## הפרומפט המלא (תבנית מהמדריך, מותאמת לציור)

```
Use the uploaded image as the only source scene. The red lines, arrows, and point labels are camera path instructions only and must never appear in the final video. Completely remove all red lines, arrows, markings, and annotations from the final result. The additional reference images show close-up details of the same painting from inside the scene; use them to keep faces, costumes and objects accurate when the camera gets close.

DESCRIPTION OF DIRECTION:

[PASTE THE SEGMENT DIRECTION HERE]

Generate one single continuous FPV drone shot that starts at point 1 and ends at point 2. The camera must follow the red path as a strict flight path guide, not a loose suggestion. Stay on this path from start to finish. Do not cut, reset, jump, teleport, switch angles. The entire result must feel like one uninterrupted take with no scene changes and no deviation away from the intended route.

TIME IS FROZEN. Every character, the swing, the ropes, the flying slipper, the fabric, leaves and flowers are completely still, like a single instant held forever. Only the camera moves. No character moves, blinks or breathes.

Reconstruct the image into a full, believable 3D environment so the drone can move naturally through real depth rather than sliding across a flat image. The drone should fly between, around, behind and past the subjects and objects where appropriate, using the marked path as the directional route through the space. The movement should create realistic foreground and background parallax, a strong sense of depth, and immersive spatial motion.

The movement should be slow, floating, controlled, cinematic and intentional, like a skilled FPV pilot hovering gently through a frozen moment. Use smooth forward motion, natural acceleration and deceleration, subtle camera banking and gentle curves. Do not use wild acrobatics, huge 360 spins, chaotic rolling, extreme flips, or disorienting motion.

Keep the shot locked inside the same original scene at all times. Preserve the original subjects, layout, proportions, environment, perspective, and artistic painting style exactly: Fragonard's Rococo oil painting, loose feathery brushwork, visible canvas texture, silvery blue-green foliage and pale pink silk. Do not redesign the artwork. Do not add extra objects. Everything remains made of paint.

Audio: no music, no voiceover, no dialogue, and no added vocals of any kind. Use only soft sound effects: gentle air movement, a faint breeze through leaves, distant birds, and subtle whooshes that match the slow camera motion.

Critical constraints:
No visible red lines or annotations in the final result.
No cuts.
No scene changes.
Nothing in the scene moves; only the camera moves.
One continuous FPV shot only.
Start at point 1, follow the marked path, and end at point 2.
```

### טיפים
- אם התנועה כאוטית – לפשט את המסלול (פחות עיקולים) ולקצר את ה-direction.
- אם דמויות זזות למרות הכל – להוסיף בתחילת הפרומפט: `A frozen moment in time, a still life tableau.`
- לחיבור חלק בעריכה: להשתמש בפריים האחרון של כל סגמנט כ-start frame של הבא (עדיף על התקריב שנוצר, אם התוצאה סטתה).
- ליצירה מחדש של תקריב: `python3 gen_closeups.py 2a_woman_hero` (או בלי ארגומנטים לכל 8).
