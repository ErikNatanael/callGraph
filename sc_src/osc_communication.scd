

(
SynthDef(\sineperc, { var freq = \freq.kr(100), sig, env;
	env = EnvGen.kr(Env.perc(\atk.kr(0.01), \rel.kr(1), 1.0, [\c0.kr(4), \c1.kr(-4)]), doneAction: 2);
	freq = XLine.ar(Rand(4000, 6000), freq, 0.006);
	sig = SinOsc.ar(freq) * 0.5 * \amp.kr(1);
	sig = sig * env;
	sig = Pan2.ar(sig, \pan.kr(0));
	Out.ar(0, sig);
}).add;
SynthDef(\verb, {
    var snd, wet;
    snd = In.ar(\out.kr(0), 2);
    wet = snd.sum + PitchShift.ar(snd, 0.2, 2);
    wet = DelayN.ar(wet, 0.02, 0.02);
	wet = JPverb.ar(wet, 2, 0.0, 1.2);
    wet = HPF.ar(wet, 100) * -3.dbamp;
    wet = LPF.ar(wet, 7000);
    snd = snd + (wet * \wetness.kr(0.1));
    // Add a little bit of tape flutter
    snd = DelayC.ar(snd, 0.2, LFNoise2.kr(1).range(0, 0.005));

    //snd = (snd * Demand.kr(Dust.kr(5), 1, Dseq([1, 0], inf))) + (snd*0.5);

    ReplaceOut.ar(\out.kr(0), snd);
}).add;
)
(
Synth(\verb);

~functionCallDensity = 0;
OSCdef(\test, {
	|msg, time, addr, recvPort|
	if(msg[1] == \functionCall) {
		~functionCallDensity = ~functionCallDensity + 1;
		Synth(\sineperc, [\freq, rrand(400, 4000), \amp, 0.05, \rel, 2/~functionCallDensity]);
		msg.postln;
	};

},
'/timeline-message', n); // def style

{
	loop {
		~functionCallDensity = min(~functionCallDensity, 10);
		~functionCallDensity = ~functionCallDensity * 0.95;
		~functionCallDensity = max(~functionCallDensity, 0);
		0.01.wait;
	}
}.fork;
)