

(
SynthDef(\noisepad, { |out = 0, amp = 0.5, freq = 440, gate = 1, attack = 0.01, decay = 0.1, roughness = 0.01, pressure = 0.3, pan = 0, freqLag = 0.2, fragAmount = 0.5, fragDensity = 10, focus = 0.01, maxRoughness = 1, minRoughness = 0, lpf = 10000 |

	var sig, env, unison, detune, freqs, noise, osc2, rough;
	unison = 7;
	detune = 0.03;
	detune = \detune.kr(0.001);//LFNoise2.kr(1/2).range(0.001, 0.05);
	rough = LFNoise1.kr(1).range(\minDist.kr(0), \maxDist.kr(1));
	roughness = LFNoise1.kr(0.2).range(minRoughness, maxRoughness);

	freq = freq.lag(freqLag);
	pan = pan.lag(1);

	freqs = unison.collect{|id| freq * (1 + detune*(id+1)) };
	//freqs = freqs.addAll(freq*[2, 4, 5, 6]);
	//freqs.poll;

	env = EnvGen.kr(Env(
		levels: [0, 1, 0.5, 0],
		times: [attack, 0.8, decay],
		curve: [0, 0, -7],
		releaseNode: 2),
		gate: gate,
		doneAction: 2);

	sig = Resonz.ar(BrownNoise.ar(), freqs, 0.001) * 2;

	//sig = RLPFD.ar(sig, freqs, 0.4, 0.1) * 3;
	sig = Mix(sig);
	sig = CombL.ar(sig, 0.2, freq.reciprocal, -3);

	osc2 = Resonz.ar(WhiteNoise.ar(), freq, 0.001) * 0.0001;
	osc2 = RLPF.ar(RHPF.ar(WhiteNoise.ar()*0.01, freqs, focus*2), freq, focus);
	osc2 = Mix(osc2);

	sig = (sig * roughness) +  (osc2 * (1 - roughness));

	sig = (sig * (1-rough)) + (DFM1.ar(sig, freq, rough, 1, 0.0, 0.005*rough)*0.5 * rough);
	//sig = sig + (DFM1.ar(sig, 100, 1.0, 20*rough).tanh);

	sig = (sig*(1-fragAmount)) + (sig * Lag.kr(Demand.kr(Dust.kr(fragDensity) + Impulse.kr(0), 0, Dseq([1, 0], inf)), 0.02) * fragAmount);
	sig = LPF.ar(sig, lpf.min(19000));


	Out.ar(out, Pan2.ar(sig, pan)*env*amp);
}).add;
SynthDef(\tick, {
	var sig, env;
	env = Env.perc(0, 0.2).kr(2);
	sig = Impulse.ar(0);
	sig = sig * 0.1;
	sig = RLPF.ar(sig, \freq.kr(1000), \rq.kr(0.1));
	sig = Pan2.ar(sig, \pan.kr(0)) * \amp.kr(1.0);
	Out.ar(\out.kr(0), sig);
}).add;
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
	wet = JPverb.ar(wet, 4.0, 0.0, 1.8);
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
~speedSynth = Synth(\noisepad, [\freq, 400]);
~functionCallDensity = 0;
~speed = 1;
~scriptChords = [];
300.do{|i|
	var chord = [1.0, 3/2, 5/4, 2, 4/3, 16/15]*rrand(2000, 4000);
	~scriptChords = ~scriptChords.add(chord);
};
OSCdef(\test, {
	|msg, time, addr, recvPort|
	if(msg[1] == \functionCall) { var notes = [1.0, 3/2, 5/4, 2, 4/3, 16/15]*4000;
		~functionCallDensity = ~functionCallDensity + 1;
		Synth(\tick,[\freq, rrand(5000, 6000)*~speed.linlin(0.0, 2.0, 0.1, 2.0), \rq, ~functionCallDensity.linexp(0, 1, 0.001, 1.0), \amp, min(~speed.pow(2), 1.0), \pan, [-1, 1].choose]);
		Synth(\sineperc, [\freq, min(notes.choose*~speed.linlin(0.0, 1.0, 0.1, 2.0), 19000), \amp, 0.05 * max((1-~speed), 0).pow(2), \rel, 10/~functionCallDensity, \pan, rrand(-1, 1.0)]);
		msg.postln;
	};
	if(msg[1] == \changeSpeed) {
		~speedSynth.set(\freq, msg[2].linlin(0.0, 2.0, 40, 400));
		~speed = msg[2];
		msg.postln;
	};

},
'/timeline-message', n); // def style

{
	loop {
		~functionCallDensity = min(~functionCallDensity, 100);
		~functionCallDensity = ~functionCallDensity * 0.95;
		~functionCallDensity = max(~functionCallDensity, 0);
		0.01.wait;
	}
}.fork;
)