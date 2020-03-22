ZChordz
=======

ZChordz is an LV2 plugin that allows to map the chords of a whole Minor or Major scale onto the white keys of a piano keyboard. It allows creating complex chords by setting the velocity multiplier for each note of the chord.

For example, if you set the 3rd key control to 0, the 3rd note will not get output. If you set it halfway, it will output a 3rd note in the chord with half the velocity of the input note. If you set it to max it will output the 3rd note at the velocity of the input note.


All Minor and Major chords are included. They can be set by the scale and major controls. Scale sets the scale, so that:

Major is a boolean (whether to use major or not).

White keys are mapped to the chords in order. With A Minor selected:
C plays A Minor
D plays B diminished
E plays C Major
F plays D Minor
G plays E Minor
A plays F Major
B plays G Major

7th, 9th, 11th, 13th, bass, and one octave up notes can be added in varying amounts, in addition to to the standard base note, 3rd and 5th triad. Notes can be removed as well. The only drawback is that these controls are global. I will make another plugin in the future that allows mapping arbitrary chords to each key via a UI.

The black keys are not used at the moment. They will be used as modifiers in the future for inversions.
