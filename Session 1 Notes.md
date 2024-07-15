Introduction to music technology and essential concepts
1. Introduction 
   1. Who is a music technologist?
   2. What do we need to know?we will be talking about what sound and music consists of 
   3. I am a musician first and technologist second 
      1. everything you learn will be important whether you are a musician performing with technology or someone creating your own new technology to make music
2. Fundamental concepts
   1. Learning the languageWhat is sound harmonics amplitude frequency 
3. names for pitch, frequency, midinote, chord degree
4. What is an octave frequency, ratios, harmonics, and pitches? ￼
5. Visualization 
6. Frequency spectrum is and frequency responses. ￼
7. Sounds and time envelopes, rolloff, etc. also revert and delay. ￼
8. Playing with synths 
9. ￼ what is digital audio, sampling rate bit depth, audio interfaces 
10. using plug-ins working with EQ and compression ￼￼
11. Mastering using speakers and the entire recording chain. ￼

# Introduction
Who is a music technologist? I would say anyone who is creative, musical ideas are expressed through new electronic or digital systems from any of us. This will involve learning how do use new systems being aware of what the new possibilities are in integrating them into our musical performance practice however, increasingly it is common for musicians to actively create new technologies as part of their creative process. For me, this is one of the most exciting aspects of being a contemporary musician.

Here are some examples of work you might engage in:
* recording music entirely within a digital audio workstation
  * you might use a MIDI controller as an input device, or work solely with your laptop
* building a personal sample library for performance or composition
  * this involves either recording your own sounds, or finding sounds only and processing them
  * your personal musical tastes and goals will cause you to be highly selective, and you will likely manipulate all of the samples in your library to get them to sound just right
* building synthesizers, or other musical instruments, inspired by current instruments
  * many people are inspired by existing instruments, either wanting to improve on them, or combine them, or just build something that you can't get your hands on any other way
* designing new kinds of instruments, inspired by your own ideas
  * sometimes you'll have an idea for a new instrument that you've never seen before. Exploring your own ideas in this way, can be highly satisfying and lead to very unique music.
* designing new forms of musical performance using sensors
  * all hardware instruments rely on sensors to detect gestures. There are lots of sensors out there and also lots of different ways of making mechanical structures to work with the sensors. It can take a long time to get a sensor to feel just right!
* creating tools to generate music
  * AI has gotten a lot of attention for generating music in the past few years, but people have been creating generative music processes for centuries. Many musicians have specific generative processes that they rely on as part of their practice, and get to know extremely well. Generative music does not have to be an all or nothing deal, you can have a generative system create only part of the musical texture, and then control or compose the other parts of yourself. As you can imagine, there are lots of possibilities, and they can be really fun!
* creating tools to analyze music
  * Similarly, there has been a lot of attention on music analysis in the past few years. Music analysis lets the computer understand what is going on within a musical structure in the same way that a human might hear it. This makes it possible for a computer to compare pieces of music, identify genres tempos or other musical elements, learn the underlying patterns in a musical piece in order to create similar pieces, etc. 

These are only a few examples of work within music technology – there are many others, anything, which lets you explore your personal fascination with music and sound through a computer!

# Learning the Language pt 1.

Music technology brings together people from lots of different fields, including performers and composers, acoustic engineers, software programmers, electrical engineers, hardware designers, UX designers, human-computer interaction researchers, cognitive scientists. . . . the list goes on! All of these people work with the same fundamentals of music, but often talk about it in different ways. We will need to learn the language of music technology to help us understand how the same concepts can be thought of in different ways.

##  Visualizing audio
It is helpful to have a visual representation of audio. There are two common ways by do this, looking at audio in the time domain using an oscilloscope, or looking at audio in the frequency domain using a spectroscope.

### Oscilloscopes
Oscilloscopes are a tool to analyze signals used within electrical engineering. The nice thing about them is they are able to give us a representation of what an audio waveform actually looks like in the air. Generally speaking, oscilloscopes show amplitude in the Y axis, and time in the X axis. Because sound moves really fast and we might want to zoom in or zoom out in both axes. You will notice that the sound continues to travel from left to right.
### Spectroscopes
Spectroscopes are used to show the frequency content of sound. We will take a look at what that means in the moment. There are several kinds of spectroscopes that you will see. Some of them will represent frequency on the x-axis and amplitude on the Y axis. But there are also special tools called Spectrograms that try to combine time, amplitude, and frequency. 
## Pitch & Frequency
When you ask people what they think the most fundamental aspect of music is, they will often say musical pitch. When a young musician first picks up an instrument, they will often begin by playing a scale. What are the terms we use to describe a musical scale?

1. *Note Name*: a letter or a word used to refer to a specific pitch, such as “Middle C”, “High G”, or “Sol”. Music theorists will often include the octave number, such as C3 being an octave lower than C4.
2. *Harmonic Interval*: the way in which a note relates to the harmony, a piece of music, such as the tonic, 5th, or leading tone.

We might think that understanding how to name musical pitches gives a strong foundation for understanding music, but in reality is that what the name of a musical pitch means can vary. Different cultures refer to notes with different words, different styles of music think about pitches in different ways, different traditions use different kinds of notation, and particularly how a pitch relates to a frequency can be almost totally arbitrary.
### Frequency
Fundamentally, pitch is a construct that musicians agree on in order to communicate with each other. This is very different from the *frequency* components of music.

Frequency is a measurable characteristic of sound, something that has a firm basis in traditional physics, and will be consistent throughout the world. Put very simply, the frequency of a waveform is the of number of repetitions of a waveform within one second. In fact, the earliest name for musical frequency is simply cycles per second (cps). This name is still used within certain disciplines. 

However, most of us use the term *hertz* as the unit of frequency, abbreviated as hz or khz for kilohertz. The lowest note on a guitar is generally ~83Hz, middle C on a piano is 261.63 hz, and the highest sound a human can hear is generally ~20khz.

So, how is frequency different from pitch? A couple of key ways:
* because frequencies are measurable they are consistent. The frequency that pitches refers to can very, for example A equals 440 Hz or A equals 432 Hz.
* we generally compare two frequencies by looking at the ratio between them. Because both frequencies are fixed, the ratio is also fixed. In musical tuning systems, however, the distance between two pitches may refer to different ratios. This is referred to as using different intonations.
* there are a relatively small number of pitches within most musical systems, whereas frequency and represents any musical sound.
### Sine waves
We often think about calculating frequency in terms of the simplest waveform, a sine wave. 

What do we mean when we say that a sine wave is the simplest waveform? If you take a look at any arbitrary waveform you might notice that it looks very complicated with lots of peaks and valleys that are continually changing. But it turns out that these complicated waveforms can be created by simply adding together lots of sine waves. That is to say that every complex wave forms consists of a bunch of simple sine waves. 

We can understand this better by taking a look at how a few more complex but still fairly simple waveforms can be created using sine waves. Common waveforms we might see like this are triangle waves sawtooth waves, and square waves. Each of these can be created by something together a series of sine waves.

### Harmonics
While any sound can be thought of as a combination of sine waves, simple waveforms are a special case in that all of the sine waves that are present fuse together into a single shape, and also a single pitch. This causes each waveform to have a slightly different sound. which we refer to as *timbre*. 

The way that we create waveforms is by combining a bunch of sine waves that are integer multiples of the *fundamental frequency.* The fundamental frequency is the lowest frequency that is present, and every other frequency, it must be an integer multiple of that fundamental. These integer multiples are called harmonics, and they are present in any sound of that has a single strongly defined musical pitch.

For example, if the fundamental frequency of a waveform is 100 hz, the harmonics will be 200, 300, 400, 500, 600, etc. 
### Octaves
We already mentioned how important it is to be able to compare the distance between two frequencies. Partly this is because it is very common for music to have more than one pitches played at the same time. The most basic relationship between two pitches is that of the octave. 

From a frequency perspective, an octave is a doubling of frequency. So the octave of 110 Hz (A2) is 220 Hz (A3), and the octave of 200 Hz (A3) is 400 Hz (A4). If you look at this for a second, we will see that two octaves above 110 Hz is 440 Hz.  This shows us that our perception of pitch is nonlinear, that is the distance in an octave will change, depending on where it is located in the frequency spectrum.

If we continue following the above octaves we get: 880 hz (A5), 1760 hz (A6), 3520 hz (A7), 7040 hz (A8), 140080 hz (A9). Any higher than this and we won’t be able to hear it. Notice that means that the highest octave we just described has a distance of 7000 Hz, versus just 110 Hz between A2 and A3.
 
Another implcation of this is that all the other intervals in a musical scale are similarly nonlinear. 
# Exploring synthesizers
Take a moment to play around with a synthesizer, either a hardware synthesizer, or a software synthesizer. Look for the controls that affect the frequency of the sound. You might also consider what other aspects of the sound you can change, for example, the loudness of the sound, or the quality of the sound. 
# Learning the Language pt. 2
## Volume & Amplitude
Another common characteristic of sound is how loud it is. The typical way of talking about loudness is volume. Volume is just a perceptual description of our current experience of loudness levels, but in fact our perception changes strongly based on our environment. If we are at home in bed, something that sounds very loud to us might not sound nearly as loud if you are sitting in a noisy restaurant.

### Amplitude
Amplitude is the part of the waveform that affects our perception of volume. Very simply amplitude is a measurement of how far a waveform deviates from its midline. The taller or larger is a deviation is. the louder the volume of a waveform will be. There are two common ways of referring to amplitude.

When working with digital audio, particularly when programming, we often refer to the amplitude of a wave form as ranging between -1 and 1.  Notice that we are using zero as the middle line of our waveform. This makes sense because all sound is an alternating wave form that periodically moves up and down and revolves around a single middle line. When you look at the raw data of an audio file, you will find that all of the amplitudes are within this range of [-1, 1]. 

The other way of referring to amplitude is by using decibels (or dB). (If you think about that for a second, a Bel is the actual unit, but we will always refer to deci-Bels, hence decibel and dB). A decibel is a handy unit because it takes into account that, just like frequency, our perception of amplitude is nonlinear. In fact, we have to more than double the amplitude of a signal in order for it to sound twice as loud. 

The most important number to know when thinking about decibels is that 6dB is a doubling of measured amplitude.

### Decibels Full-Scale
Remember, how frequency is related to the number of cycles per second? That per second gives us a constant reference we can compare to. Similarly, we need to use a constant reference in order to compare our sound’s amplitude in order to get a measurable result. It turns out that there are actually a variety of decibel formats, depending on whether we are measuring sound in  air (dB Sound Pressure Level or dB SPL), sound as an electrical signal (dBV), or inside a computer (dBFS). 

Focusing on the computer side of things, decibel full-scale or **dBFS** represents amplitude in terms of the highest possible amplitude able to be represented within a computer given a certain number of bits used to represent that amplitude. For 8 bit audio there are a total of 256 possible amplitude levels. The highest amplitude (255) would have a value of 0dBFS, and every other amplitude level would be represented as a negative number. For this reason, dBFS always consists of negative numbers.

The one exception to this is when using dB to come here to arbitrary audio signals. For example, it is common to say you increased the amplitude of a sound by 6 dB, meaning, you doubled its amplitude compared to its previous amplitude.

## Timbre, tone quality, and frequency spectrums
Now we can talk about sound in terms of its frequencies, it's amplitude, and the relative amplitude of the different frequencies within the sound. We already saw Hall combining frequencies at different amplitudes can let us create different waveforms. It turns out that all of their difference between sounds can be described in just this way. 

What makes my voice sound different than your voice? Why does a flute sound different than a trumpet? How can we tell the difference when we knock on a piece of wood versus a piece of metal?

All of these perceptions are possible because we understand on the deep level how frequencies combine to make a sound. Frequencies that are harmonics will create musical pictures that are very clear. Frequencies that are not harmonic (or inharmonic) will cause, dissonance, noise, or complexity.

### Timbre
Timbre (pronounced tam-brr in american english) refers to the quality of a sound. We might say an instrument has a nice timbre, or a woody timbre, or a harsh timbre. Timbre can mean much the same thing as tone quality, or musical tone in other contexts. 
### Frequency spectrum
While timber is a qualitative term, we can describe it in terms of the frequency spectrum of a sound. The frequency spectrum is simply a representation of all of the frequencies that are present within a sound, and their various amplitudes. Using a spectroscope, we can clearly see the difference between the frequency spectrum of different vowels.

Everything in the world that can create or respond to sound has a *frequency response*. This frequency response illustrates how an object or system will affect the frequency spectrum of the sound that it creates or the sound that a processes. We might say that when we send audio through a system that has a flat frequency response, the output is the same as the input. But in reality, there is no such thing as a flat frequency response. Everything has some kind of response, and generally has an audible impact on the sound as it passes through it.
### Timbral evolution
Something that will become very apparent when looking at a spectroscope is that the frequency spectrum of a sound does not stay constant. This is true in almost all contexts. Because this is such a fundamental aspect of sound, we are very sensitive to changes in timbre over time and they play an important role in our musical perception.

The most basic way of thinking about timbral evolution is when you play a note on an instrument. When you pluck a note on a string, for example, there is a brief moment, when the note is first plucked when it is at its highest amplitude. At this moment it is also at its brightest, that is the highest components of the frequency spectrum are relatively loud. As the sound progresses over time, the higher frequencies become quieter and less perceptible. This is characteristic of many kinds of instruments, including pianos, drums, string instruments of all sorts, mallet instruments, even trumpets and wind instruments when they are strongly tongued. 

### Envelopes
One way of expressing this evolution overtime within synthesizers is through the use of envelopes and filters. A filter is used to modify the frequency spectrum of a sound. The most common kind of filter is a lowpass filter, which allows low frequencies to pass while filtering out higher frequencies. Generally, speaking, filters that don't change are not that interesting. 

What makes filters much more interesting is when they change over time as a sound progresses. This is much like how the higher frequencies and a string die out overtime. To do this, we control the frequency of a filter using an envelope. The envelope starts with an amplitude of zero, and then when a note is played,the envelope’s amplitude increases to one and then gradually falls back to zero. When the envelope is used to modify the frequency of a filter, it will make an audible change, allowing the higher components of a sound through at the beginning, but filtering them out as the sound progresses. When an envelope is used to modify the amplitude of a sound, it will make the sound louder at the beginning of the note, and gradually fade to silence.

### ADSR Envelopes
Amplitudes will generally have controls for how fast the amplitude goes from 0 to 1 and then back down to zero again. The rise from 0 to 1 is called the attack of the envelope, or the rise time. This is often quite short for plucked or struck instruments. What happens after this point varies depending on what kind of instrument we are playing.

The most common kind of instrument we might emulate is a keyboard instrument. With keyboard instruments, you strike a note and then hold it for a period of time before releasing the note. When you play a note on the piano, you will hear a quick rise, and then a gradual fade to what we called the sustain level of the piano, where the note continues to ring with relatively little change in amplitude. When you release your finger from the key, the sound then goes from this is sustain level to silence.

To represent this in synthesizers, we use something called an attack-decay-sustain-release (ADSR) envelope. 
* The attack time represents that sudden rise in amplitude
* The decay time represents how long it takes for the amplitude to fade to the sustain leve
* The sustain is the amplitude of the sustain level. In an ADSR envelope the sustain is a fixed value, while in a piano this sustain does actually have a gradual fade.
* The release time determines what happens when a key is released. At this point, the sound goes from its current amplitude 20 over at the duration of the release time.
There are several other kinds of envelopes out there, including an attack-sustain-release envelope, and attack-release envelope, etc. But if you understand an ADSR envelope then every other envelope is relatively simple.

# Exploring synthesizers
Take a look at a hardware or software synthesizer, and look for the elements of the synthesizer used for changing the timbre and the envelope of the sound. Envelopes will generally need to be triggered in someway, either by a keyboard or a sequence, or sometimes just by pushing a button.

# Digital Audio
Everything we have talked about so far generally refers to audio in all of its different forms. But for the most part we work with sound in a digital forum at this point, so it makes sense to understand how we represent sound digitally. Luckily, it is quite simple.
## Digital sampling
The biggest change is that we are going to move from a continuous or analog signal to a discrete or digital signal. Remember how a sound wave on an oscilloscope has time on the X axis and amplitude on the Y axis. To turn this into a discrete signal we will have to discretize both time and amplitude.
### Sampling rate
The time dimension of sound and digital audio is chopped up into what we call samples a sample is simply a representation of the amplitude of a wave form at one moment in time the rate at which we capture samples is called the sampling rate.
* Sample: the amplitude of a sound wave at one discrete moment of time, generally represented from -1 to 1
* Sampling rate: the number of samples we measure per second, generally represented in kilohertz
If you zoom in on a waveform, you can often find dots or values representing the amplitude of the individual samples. The magic is that if you create a sound wave that smoothly changes between these values at the same rate as the sampling rate you will perfectly reconstruct the sound that was sampled with no noticeable change in total quality.

To say that again, if you take a sound wave, convert it to digital, and then convert it back to an analog sound wave you will not be able to hear any notice in the sound. Sound like an impossibility? Well, assuming a microphone, and speaker that both have flat frequency responses then this will be true. (Of course, we know that there is no such thing as a flat frequency response so this is not quite true, but let's ignore that for the moment).
### Nyquist frequency
There is one giant caveat here, and that is that the highest frequency of the sound he wants to sample must not be higher than half of the sampling rate. We know that the highest frequency that humans can generally hear does not exceed 20kHz. Therefore, frequencies above that in generally be ignored. This means that a sampling rate of 40kHz or higher should be sufficient for most audio. In fact, most sampling rates are either 44.1 kHz or 48 kHz.
### Bit depth
The other consideration in digital audio is how many bits we use to represent amplitude. The reason why this is a little bit of a big deal is the more bits we used to represent amplitude the larger the file sizes that we generate, and the more processing it takes to generate audio. Back in the old days, eight bit audio was often used, actually affected quite a bit of a nostalgia for eight bit audio. When digital audio became of available on CD, the bit depth went from 8-bit (0-255) to 16-bit (0-65535) which increase the fidelity of music so much that we generally consider 16 bit audio as having a wider dynamic range than we can perceive. 

There are higher bit depths that computers use now, which can decrease the likelihood of complex processing affecting the sound of negatively, and also make it marginally more CPU intensive to work with audio. But for all practical purposes, 16 bit is good enough.
# Use cases of all of the above 
1. Acoustic instrument construction: stradivarious
2. Electronic synthesis techniques: east-coast vs west-coast
3. Lo-fi audio: vinyl vs digital
4. Loudness wars